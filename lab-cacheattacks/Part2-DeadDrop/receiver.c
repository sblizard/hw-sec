
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

int main(int argc, char **argv)
{
	// Put your covert channel setup code here

	//create a buffer
	uint64_t *eviction_buffer = (uint64_t *)malloc(1.5*16384*8*sizeof(uint64_t));
	uint64_t *test_buffer = (uint64_t *)malloc(1.5*16384*8*sizeof(uint64_t));
	uint64_t *t_buffer = (uint64_t *)malloc(8*sizeof(uint64_t));
	//uint64_t *u_buffer = (uint64_t *)malloc(8*sizeof(uint64_t));
	//uint64_t *v_buffer = (uint64_t *)malloc(8*sizeof(uint64_t));
	//uint64_t *w_buffer = (uint64_t *)malloc(8*sizeof(uint64_t));
	//uint64_t *x_buffer = (uint64_t *)malloc(8*sizeof(uint64_t));


  	if (NULL == eviction_buffer) {
      perror("Unable to malloc");
      return EXIT_FAILURE;
  	}

	volatile char tmp;
	
	printf("Please press enter.\n");

	char text_buf[128];
	fgets(text_buf, sizeof(text_buf), stdin);

	printf("Receiver now listening.\n");

	bool listening = true;

	while (listening) {
		// Put your covert channel code here
		// Fill the L2 cache with the buffer
		/*for (int j=0; j<6144; j++){
           eviction_buffer[j] = 1;
        }*/
		int count = 0;
		for(int i = 0; i < 100; i++) {
			tmp = t_buffer[0];
			//tmp = u_buffer[0];
			//tmp = v_buffer[0];
			//tmp = w_buffer[0];
			//tmp = x_buffer[0];
			//t_buffer[0] = 1;
			//wait for signal
			if(string_to_int(text_buf) == 0b1) {
				printf("Test\n");
				for (int j=0; j<196608; j+=4){
           			test_buffer[j] = 1;
        		}
			}
			if(string_to_int(text_buf) == 2) {
				for (int j=1; j<196608; j+=4){
           			test_buffer[j] = 1;
        		}
			}			
			if(string_to_int(text_buf) == 3) {
				for (int j=2; j<196608; j+=4){
           			test_buffer[j] = 1;
        		}
			}
			//time access to eviction buffer
			CYCLES time = measure_one_block_access_time((uint64_t)t_buffer);
			//CYCLES uime = measure_one_block_access_time((uint64_t)u_buffer);
			//CYCLES vime = measure_one_block_access_time((uint64_t)v_buffer);
			//CYCLES wime = measure_one_block_access_time((uint64_t)w_buffer);
			//CYCLES xime = measure_one_block_access_time((uint64_t)x_buffer);

			//printf("%d\n", time);
			//printf("%d\n", uime);
			//printf("%d\n", vime);
			//printf("%d\n", wime);
			//printf("%d\n", xime);
			//printf("%d\n", time+uime+vime+wime+xime);
			//int sum = time + uime + vime + wime + xime;
			printf("%d\n", time);
			if((time > 130 && time < 250) || (time > 310)) {
				count++;
			}
		}
		printf("%d\n", count);
		listening = false;

	}

	printf("Receiver finished.\n");
	free(eviction_buffer);
	free(test_buffer);
	free(t_buffer);
	//free(u_buffer);
	//free(v_buffer);
	//free(w_buffer);
	//free(x_buffer);
	return 0;
}

void handle_signal(int signal_num) {
    printf("Received signal in receiver: %d\n", signal_num);
}


