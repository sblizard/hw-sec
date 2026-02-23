
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>
#include <signal.h>

// TODO: define your own buffer size
//#define BUFF_SIZE (1<<21)
//#define BUFF_SIZE [TODO]

void loop() {
  loop();
}

void handle_signal(int signal_num) {
  printf("Received signal: %d\n", signal_num);
  loop();
}

int main(int argc, char **argv)
{
  // Allocate a buffer using huge page
  // See the handout for details about hugepage management
  //void *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
  
  //if (buf == (void*) - 1) {
    // perror("mmap() error\n");
     //exit(EXIT_FAILURE);
  //}
  // The first access to a page triggers overhead associated with
  // page allocation, TLB insertion, etc.
  // Thus, we use a dummy write here to trigger page allocation
  // so later access will not suffer from such overhead.
  //*((char *)buf) = 1; // dummy write to trigger page allocation


  // TODO:
  // Put your covert channel setup code here

  uint64_t *eviction_buffer = (uint64_t *)malloc(1.5*512*8*sizeof(uint64_t));
  
  if (NULL == eviction_buffer) {
      perror("Unable to malloc");
      return EXIT_FAILURE;
  }

  printf("Please type a message.\n");

  bool sending = true;
  while(true) {
    while (sending) {
      char text_buf[128];
      fgets(text_buf, sizeof(text_buf), stdin);
      sending = false;
      signal(SIGINT, handle_signal);
      // TODO:
      // Put your covert channel code here
      if((uint64_t)string_to_int(text_buf) == 1) {
        for (int j=0; j<6144; j++){
           eviction_buffer[j] = 1;
        }
      }

      printf("%d", measure_one_block_access_time((uint64_t)eviction_buffer));
    }
    sending = true;
  }
  
  free(eviction_buffer);
  printf("Sender finished.\n");
  return 0;
}


