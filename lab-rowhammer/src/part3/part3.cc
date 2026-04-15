#include <algorithm>
#include <array>
#include <tuple>
#include <ranges>
#include <numeric>
#include <cmath>

#include "../shared.hh"
#include "../params.hh"
#include "../util.hh"

#define BANKS 16
#define CONSISTENCY_RATE 0.95
#define POOL_SIZE 1000
#define ROUNDS 100

void print_bins(const std::array<std::vector<uint64_t>, BANKS> &bins)
{
    for (size_t i = 0; i < BANKS; i++)
    {
        const auto &bin = bins[i];
        printf("Bin[%zu] size = %zu\n", i, bin.size());
        for (size_t j = 0; j < bin.size(); j++)
        {
            printf("  [%3zu] phys = 0x%lx\n", j, bin[j]);
        }
        printf("\n");
    }
}

int gt(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

uint64_t median(uint64_t *vals, size_t size)
{
    qsort(vals, size, sizeof(uint64_t), gt);
    return ((size % 2) == 0) ? vals[size / 2] : (vals[(size_t)size / 2] + vals[((size_t)size / 2 + 1)]) / 2;
}

int find_first_empty(const std::array<std::vector<uint64_t>, BANKS> &bins)
{
    for (size_t i = 0; i < bins.size(); ++i)
    {
        if (bins[i].empty())
        {
            return i;
        }
    }
    return -1; // none empty
}

int cmp_u64(const void *a, const void *b)
{
    uint64_t x = *(const uint64_t *)a;
    uint64_t y = *(const uint64_t *)b;

    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

uint64_t median_typesafe(uint64_t *vals, size_t size)
{
    qsort(vals, size, sizeof(uint64_t), cmp_u64);

    if (size % 2 == 1)
    {
        return vals[size / 2];
    }
    else
    {
        return (vals[size / 2 - 1] + vals[size / 2]) / 2;
    }
}

/*
 * bin_rows
 *
 * Bins a selection of addresses in the range of [starting_addr, final_addr)
 * based on measure_bank_latency.
 *
 * Input: starting and ending addresses
 * Output: An array of 16 vectors, each of which holds addresses that share the same bank
 *
 * HINT: You can refer the way in part2.cc: run measure_bank_latency for multiple times and then get the median number to avoid noise
 *
 */
std::array<std::vector<uint64_t>, BANKS> bin_rows(uint64_t starting_addr, uint64_t final_addr)
{
    std::array<std::vector<uint64_t>, BANKS> bins;
    const int LAT_TRIALS = 70;
    const size_t REPS_PER_BIN = 6;
    const int MAX_RETRIES = 12;

    // Iterate over addresses in range
    for (uint64_t addr = starting_addr; addr < final_addr; addr += 3 * ROW_SIZE)
    {
        bool handled = false;

        // Allow for several retries in the case that no bin is the clearly best choice
        for (int attempt = 0; attempt < MAX_RETRIES && !handled; ++attempt)
        {
            std::vector<int> matching_bins;

            // Iterate over each bin
            for (size_t i = 0; i < bins.size(); ++i)
            {
                if (bins[i].empty())
                    continue;

                size_t reps = std::min(REPS_PER_BIN, bins[i].size());
                int matches = 0;

                // Find median access time between addr and addresses in bins[i]
                for (size_t r = 0; r < reps; ++r)
                {
                    size_t idx = (reps == 1) ? 0 : (r * (bins[i].size() - 1) / (reps - 1));

                    std::vector<uint64_t> times(LAT_TRIALS + attempt * 20);
                    for (size_t trial = 0; trial < times.size(); ++trial)
                    {
                        times[trial] = measure_bank_latency(
                            reinterpret_cast<volatile char *>(addr),
                            reinterpret_cast<volatile char *>(bins[i][idx]));
                    }

                    uint64_t median_lat = median_typesafe(times.data(), times.size());
                    if (median_lat > THRESHOLD)
                    {
                        matches++;
                    }
                }

                // Require high level of agreement on if addr belongs in bin[i]
                if (matches >= static_cast<int>(reps) - 1)
                {
                    matching_bins.push_back(static_cast<int>(i));
                }
            }

            if (matching_bins.size() == 1)
            {
                // If addr only matches 1 bin, place it in said bin
                printf("[PLACE] addr=0x%lx -> bin[%d] on attempt %d\n",
                       addr, matching_bins[0], attempt + 1);
                bins[matching_bins[0]].push_back(addr);
                handled = true;
            }
            else if (matching_bins.empty())
            {
                // If matches no bin
                int empty_bin = find_first_empty(bins);

                if (empty_bin != -1)
                {
                    // If empty bins exist, place in bin
                    printf("[NEW BIN] addr=0x%lx -> bin[%d] on attempt %d\n",
                           addr, empty_bin, attempt + 1);
                    bins[empty_bin].push_back(addr);
                    handled = true;
                }
                else if (attempt == MAX_RETRIES - 1)
                {
                    // If no empty bins after MAX_RETRIES, skip
                    printf("[SKIP] addr=0x%lx no confident match after %d attempts\n",
                           addr, MAX_RETRIES);
                    handled = true;
                }
            }
            else
            {
                if (attempt == MAX_RETRIES - 1)
                {
                    // If matches multiple bins after MAX_RETRIES, skip
                    printf("[SKIP] addr=0x%lx matches %zu bins after %d attempts\n",
                           addr, matching_bins.size(), MAX_RETRIES);
                    handled = true;
                }
            }
        }
    }

    return bins;
}

/*
 *
 * DO NOT MODIFY BELOW ME
 *
 */
std::tuple<uint64_t, double> get_most_frequent(const std::vector<uint64_t> &data)
{
    std::map<uint64_t, uint64_t> freq_map;

    for (const auto &item : data)
    {
        freq_map[item]++;
    }

    auto [most_freq, max_count] = std::accumulate(
        freq_map.begin(),
        freq_map.end(),
        std::pair<uint64_t, uint64_t>{0, 0},
        [](const auto &best, const auto &current)
        {
            return current.second > best.second ? std::pair{current.first, current.second} : best;
        });

    return {most_freq, static_cast<double>(max_count) / data.size()};
}

template <size_t LEN>
std::optional<uint64_t> find_candidate_function(const std::array<std::vector<uint64_t>, LEN> &bins)
{
    std::optional<uint64_t> result = std::nullopt;

    std::array<std::function<uint64_t(uint64_t)>, 3> functions = {
        [](uint64_t x)
        {
            return ((get_bit(x, 14) ^ get_bit(x, 17)) << 3) |
                   ((get_bit(x, 15) ^ get_bit(x, 18)) << 2) |
                   ((get_bit(x, 16) ^ get_bit(x, 19)) << 1) |
                   ((get_bit(x, 7) ^ get_bit(x, 8) ^ get_bit(x, 9) ^ get_bit(x, 12) ^ get_bit(x, 13) ^ get_bit(x, 15) ^ get_bit(x, 16)));
        },
        [](uint64_t x)
        {
            return ((get_bit(x, 15) ^ get_bit(x, 18)) << 3) |
                   ((get_bit(x, 16) ^ get_bit(x, 19)) << 2) |
                   ((get_bit(x, 17) ^ get_bit(x, 20)) << 1) |
                   ((get_bit(x, 7) ^ get_bit(x, 8) ^ get_bit(x, 9) ^ get_bit(x, 12) ^ get_bit(x, 13) ^ get_bit(x, 18) ^ get_bit(x, 19)));
        },

        [](uint64_t x)
        {
            return ((get_bit(x, 13) ^ get_bit(x, 17)) << 3) |
                   ((get_bit(x, 14) ^ get_bit(x, 18)) << 2) |
                   ((get_bit(x, 15) ^ get_bit(x, 19)) << 1) |
                   ((get_bit(x, 7) ^ get_bit(x, 8) ^ get_bit(x, 9) ^ get_bit(x, 12) ^ get_bit(x, 13) ^ get_bit(x, 20) ^ get_bit(x, 21)));
        },

    };

    for (size_t i = 0; i < functions.size(); i++)
    {
        auto &f = functions[i];
        bool good = true;
        for (auto &bin : bins)
        {
            uint64_t match_count = 0;
            std::vector<uint64_t> bank_comp(bin.size());
            std::transform(bin.begin(), bin.end(), bank_comp.begin(), f);
            auto [id, freq] = get_most_frequent(bank_comp);
            if (freq < CONSISTENCY_RATE)
            {
                good = false;
                break;
            }
        }
        if (good)
        {
            // conflicting results
            if (result.has_value())
                return std::nullopt;
            result = i;
        }
    }

    return result;
}

int main(int ac, char **av)
{

    setvbuf(stdout, NULL, _IONBF, 0);

    // Allocate a large pool of memory (of size BUFFER_SIZE_MB) pointed to
    // by allocated_mem
    allocated_mem = allocate_pages(BUFFER_SIZE_MB * 1024UL * 1024UL);

    // Setup PPN_VPN_map
    setup_PPN_VPN_map(allocated_mem, PPN_VPN_map);

    auto result = find_candidate_function(
        bin_rows((uint64_t)allocated_mem, (uint64_t)allocated_mem + ROW_SIZE * 4096));

    if (result.has_value())
    {
        printf("Identified function %lu as correct\n", result.value());
    }
    else
    {
        puts("Did not identify a correct function :(");
    }

    return 0;
}
