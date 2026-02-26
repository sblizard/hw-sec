
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>
#include"covert.h"

int main(int argc, char **argv)
{
	void *buf = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    	if (buf == (void *)-1) { 
		perror("mmap() error"); 
		exit(EXIT_FAILURE); 
	}
	printf("buf: %p\n", buf);
	for (size_t i = 0; i < BUFF_SIZE; i += 4096) {
        	((volatile char *)buf)[i] = 1;
	}
	
	printf("Please press enter.\n");

	char text_buf[2];
	fgets(text_buf, sizeof(text_buf), stdin);

	printf("Receiver now listening.\n");
	
	int freq[256] = {0};
	int samples = 0;
	bool listening = true;
	while (listening) {
		for (int bit = 0; bit < 8; bit ++) {
			prime_section(buf, bit);
		}

		for (volatile int i = 0; i < 100000; i++);
		
		uint8_t value = 0;
        	for (int bit = 0; bit < 8; bit++) {
			uint64_t t = probe_section(buf, bit);
        		if (t > MISS_THRESHOLD) {
            			value |= (1 << bit);
			}
    		}
        	
		fflush(stdout);
		samples++;
		freq[value]++;
		int mode = 0;
        	
		for (int i = 1; i < 256; i++)
            	
		if (freq[i] > freq[mode]) { 
			mode = i;
		}
        	printf("%d\n", mode);
        	fflush(stdout);
	}

	printf("Receiver finished.\n");
	return 0;
}


