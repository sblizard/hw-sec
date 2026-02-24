
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

#define CACHE_LINE 64
#define L2_SETS 1024
#define L2_WAYS 16
#define NUM_SECTIONS 8
#define SETS_PER_SECT (L2_SETS / NUM_SECTIONS)    
#define LINES_PER_SECT (SETS_PER_SECT * L2_WAYS)    
#define BYTES_PER_SECT (LINES_PER_SECT * CACHE_LINE)

#define EVICT_OFFSET L2_SIZE  

#define BUFF_SIZE (1 << 21) 

#define MISS_THRESHOLD 27

public void prime_section(volatile char *buf, int section) 
{
	volatile char *base = buf + (section*BYTES_PER_SECT); //Find base address for section
	//Access every line in cache section
	for (int i = 0; i < BYTES_PER_SECT; i += CACHE_LINE) {
        	(void)base[i];	
	}
}

public void evict_section(volatile char *buf, int section) 
{
	//Get base address for evicting cache lines
	volatile char *base = buf + EVICT_OFFSET + (section*BYTES_PER_SECT);
	//Access an address for each cache line
	for (int i = 0; i < BYTES_PER_SECT; i += CACHE_LINE) {
        	(void)base[i];	
	}
}

uint64_t probe_section(volatile char *buf, int section) 
{
	//Base address for section
	volatile char *base = buf + (section * BYTES_PER_SECT);
	//Coutner for amount of time needed to make n line accesses in section
	uint64_t total = 0;
    	for (int i = 0; i < BYTES_PER_SECT; i += CACHE_LINE) {
        	uint64_t t1 = rdtsc_begin();
        	(void)base[i];
        	uint64_t t2 = rdtsc_end();
        	total += (t2 - t1);
    	}
	return total / LINES_PER_SECT;
}

int main(int argc, char **argv)
{
  // Allocate a buffer using huge page
  // See the handout for details about hugepage management
  void *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
  
  if (buf == (void*) - 1) {
     perror("mmap() error\n");
     exit(EXIT_FAILURE);
  }
  // The first access to a page triggers overhead associated with
  // page allocation, TLB insertion, etc.
  // Thus, we use a dummy write here to trigger page allocation
  // so later access will not suffer from such overhead.
  *((char *)buf) = 1; // dummy write to trigger page allocation


  // TODO:
  // Put your covert channel setup code here
  for (size_t i = 0; i < BUFF_SIZE; i += 4096) {
        ((volatile char *)buf)[i] = 1;
  }
  printf("Please type a message.\n");

  bool sending = true;
  while (sending) {
      char text_buf[128];
      fgets(text_buf, sizeof(text_buf), stdin);

      // TODO:
      // Put your covert channel code here
      int val = atoi(text_buf);
      if (val < 0 || val > 255) {
      	printf("Please enter 0-255.\n");
        continue;
      }
      
      for (int i = 0; i < NUM_REPEATS; i++) {
        for (int bit = 0; bit < 8; bit++) {
          if ((value >> bit) & 1) {
            evict_section(buf, bit);
          }
	}
     
        uint64_t start = rdtsc_begin();
        while (rdtsc_begin() - start < SEND_WINDOW_CYCLES) {
          asm volatile("pause");
        }

       printf("Please type a message.\n");
      }
  }

  printf("Sender finished.\n");
  return 0;
}


