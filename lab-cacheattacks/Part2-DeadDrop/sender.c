
#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>
#include "covert.h"

int main(int argc, char **argv)
{
  // Allocate a buffer using huge page
  // See the handout for details about hugepage management
  void *buf = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

  if (buf == (void *)-1)
  {
    perror("mmap() error\n");
    exit(EXIT_FAILURE);
  }

  printf("buf: %p\n", buf);
  // The first access to a page triggers overhead associated with
  // page allocation, TLB insertion, etc.
  // Thus, we use a dummy write here to trigger page allocation
  // so later access will not suffer from such overhead.
  *((char *)buf) = 1; // dummy write to trigger page allocation

  for (size_t i = 0; i < BUFF_SIZE; i += 4096)
  {
    ((volatile char *)buf)[i] = 1;
  }
  printf("Please type a message.\n");

  bool sending = true;
  while (sending)
  {
    char text_buf[128];
    fgets(text_buf, sizeof(text_buf), stdin);

    int val = atoi(text_buf);
    if (val < 0 || val > 255)
    {
      printf("Please enter 0-255.\n");
      continue;
    }

    while (true)
    {
      for (int bit = 0; bit < 8; bit++)
      {
        if ((val >> bit) & 1)
        {
          evict_section(buf, bit);
        }
      }
    }
  }

  return 0;
}
