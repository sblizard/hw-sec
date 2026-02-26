#ifndef COVERT_H
#define COVERT_H

#include "util.h"
#include <sys/mman.h>

#define CACHE_LINE       64
#define L2_SETS          1024
#define L2_WAYS          16
#define L2_SIZE          1048576
#define NUM_SECTIONS     8
#define SETS_PER_SECT    (L2_SETS / NUM_SECTIONS)
#define LINES_PER_SECT   (SETS_PER_SECT * L2_WAYS)
#define BYTES_PER_SECT   (LINES_PER_SECT * CACHE_LINE)
#define EVICT_OFFSET     L2_SIZE
//#define BUFF_SIZE        (1 << 21)
#define MISS_THRESHOLD   125
#define SEND_WINDOW_CYCLES 10000 
#define PROBE_LINES 	 16 
#define BUFF_SIZE (L2_SIZE * (L2_WAYS + 2)) 

void prime_section(volatile char *buf, int section) {
    volatile char *base = buf + (section * SETS_PER_SECT * CACHE_LINE);
    for (int i = 0; i < PROBE_LINES * CACHE_LINE; i += CACHE_LINE) {
	base[i] ^= 1;
    }
}

void evict_section(volatile char *buf, int section) { 
    for (int repeat = 0; repeat < 3; repeat++) {
        for (int way = 0; way <= L2_WAYS; way++) {
            volatile char *base = buf + (way * L2_SETS * CACHE_LINE) + (section * SETS_PER_SECT * CACHE_LINE);
            for (int set = 0; set < SETS_PER_SECT; set++) {
            	base[set * CACHE_LINE] ^= 1;
	    }
        }
    }
}

uint64_t probe_section(volatile char *buf, int section) {
    volatile char *base = buf + (section * SETS_PER_SECT * CACHE_LINE);
    uint64_t total = 0;
    for (int i = PROBE_LINES - 1; i >= 0; i--) {
        total += measure_one_block_access_time((ADDR_PTR)(base + i * CACHE_LINE));
	base[i * CACHE_LINE] ^= 1;
    }
    return total / PROBE_LINES;
}

#endif
