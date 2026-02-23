
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

int main(int argc, char **argv)
{
	// Put your covert channel setup code here

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

		// Put your covert channel code here

	}

	printf("Receiver finished.\n");

	return 0;
}

void handle_signal(int signal_num) {
    printf("Received signal in receiver: %d\n", signal_num);
}


