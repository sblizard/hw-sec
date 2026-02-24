
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

int main(int argc, char **argv)
{
	// Put your covert channel setup code here

	//create a buffer
	uint64_t *eviction_buffer = (uint64_t *)malloc(1.5*512*8*sizeof(uint64_t));
  
  	if (NULL == eviction_buffer) {
      perror("Unable to malloc");
      return EXIT_FAILURE;
  	}
	
	printf("Please press enter.\n");

	char text_buf[2];
	fgets(text_buf, sizeof(text_buf), stdin);

	printf("Receiver now listening.\n");

	bool listening = true;

	while (listening) {
		printf("test1\n");
		fflush(stdout);
		// Put your covert channel code here
		// Fill the L2 cache with the buffer
		for (int j=0; j<6144; j++){
           eviction_buffer[j] = 1;
        }
		//wait for signal

		//time access to eviction buffer
		CYCLES time = 0;
		for (int j=0; j<6144; j++) {
			time += measure_one_block_access_time(eviction_buffer);
		}
		printf("%d\n", time);
		fflush(stdout);
		listening = false;

	}

	printf("Receiver finished.\n");

	return 0;
}

void handle_signal(int signal_num) {
    printf("Received signal in receiver: %d\n", signal_num);
}


