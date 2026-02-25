
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	// Put your covert channel setup code here
	int mem = shm_open("/state", O_RDWR, 00700);
	ftruncate(mem, sizeof(int));
	int *m = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, mem, 0);
	printf("%d\n", *m);
	
	//create a buffer
	uint64_t *eviction_buffer = (uint64_t *)malloc(1.5*16384*8*sizeof(uint64_t));
	uint64_t *t_buffer = (uint64_t *)malloc(8*sizeof(uint64_t));

  	if (NULL == eviction_buffer) {
      perror("Unable to malloc");
      return EXIT_FAILURE;
  	}

	volatile char tmp;
	
	printf("Please press enter.\n");

	char text_buf[2];
	fgets(text_buf, sizeof(text_buf), stdin);
	
	printf("Receiver now listening.\n");

	bool listening = true;

	while (listening) {
		
		// Put your covert channel code here
		int result = 0;

		// Prime the L2 cache with the buffer
		for(int k = 0; k < 8; k++) {
			int count = 0;
			for(int i = 0; i < 100; i++) {
				tmp = t_buffer[0];
				*m = 0;
			//wait for signal
				while(*m == 0) {}
			//Determine bits sequentially by evicting the L2 cache on 1's
				/*if((string_to_int(text_buf) >> k) & 0b00000001 == 0b1) {
					for (int j=0; j<196608; j++){
           				eviction_buffer[j] = 1;
        			}
				}*/
			//time access to eviction buffer
				CYCLES time = measure_one_block_access_time((uint64_t)t_buffer);

				if((time > 130 && time < 250) || (time > 310)) {
					count++;
				}
			}
			//Determine whether the bit was 1 or 0 and add to the result accordingly
			if(count >= 50) {
				int exponent = k+1;
				int add = 1;
				while(exponent > 1) {
					add *= 2;
					exponent--;
				}
				result += add;
			}
			//listening = false;
		}
		
		printf("%d\n", result);

	}

	printf("Receiver finished.\n");
	free(eviction_buffer);
	free(t_buffer);

	return 0;
}

void handle_signal(int signal_num) {
    printf("Received signal in receiver: %d\n", signal_num);
}

1. Prime L2 cache r
2. transmit first bit s
3. decode first bit r
4. prime L2 cache r
5. transmit second bit s
