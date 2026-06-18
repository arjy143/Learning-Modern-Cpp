#include "lru_cache2.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {

    //check if nodeslot is actually 24 bytes or if its being padded to 32 bytes
    static_assert(sizeof(NodeSlot<uint64_t, uint64_t>) == 24);

    size_t   capacity  = argc > 1 ? strtoull(argv[1], nullptr, 10) : 100000;
    uint64_t num_ops   = argc > 2 ? strtoull(argv[2], nullptr, 10) : 20000000;
    uint64_t key_space = argc > 3 ? strtoull(argv[3], nullptr, 10) : 300000;
    uint64_t seed      = argc > 4 ? strtoull(argv[4], nullptr, 10) : 12648430;

    LRUCache2<uint64_t, uint64_t> cache(capacity);

    //splitmix64: cheap deterministic RNG so both binaries do identical work
    auto rng = [s = seed]() mutable {
        uint64_t z = (s += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    };

    for (size_t i = 0; i < capacity; ++i)        // warmup: fill to steady state
        cache.put(rng() % key_space, rng());

    uint64_t sink = 0;                            // prevents dead-code elimination
    for (uint64_t op = 0; op < num_ops; ++op) {
        uint64_t r = rng(), key = r % key_space;
        if ((r >> 32) <= (70ULL * 0xFFFFFFFFULL / 100)) {   // 70% gets
            auto v = cache.get(key);
            sink = sink * 1099511628211ULL + (v ? *v : 0);
        } else {
            cache.put(key, r);
            sink = sink * 1099511628211ULL + key;
        }
    }
    printf("checksum=%llu\n", (unsigned long long)sink);
}

//only node pool optimisation
/*
 Performance counter stats for './bench2':

          7,067.36 msec task-clock                       #    0.984 CPUs utilized             
             1,454      context-switches                 #  205.735 /sec                      
                35      cpu-migrations                   #    4.952 /sec                      
             2,129      page-faults                      #  301.244 /sec                      
    12,037,869,401      cycles                           #    1.703 GHz                       
     2,380,755,500      instructions                     #    0.20  insn per cycle            
       408,060,232      branches                         #   57.739 M/sec                     
        26,929,815      branch-misses                    #    6.60% of all branches           

       7.185119796 seconds time elapsed

       6.988292000 seconds user
       0.071593000 seconds sys
*/

//node pool + contiguous flat hash map
/*
Performance counter stats for './bench2':

          2,578.00 msec task-clock                       #    0.978 CPUs utilized             
               709      context-switches                 #  275.020 /sec                      
                 8      cpu-migrations                   #    3.103 /sec                      
               977      page-faults                      #  378.976 /sec                      
     5,538,300,732      cycles                           #    2.148 GHz                       
     1,692,536,552      instructions                     #    0.31  insn per cycle            
       207,103,731      branches                         #   80.335 M/sec                     
        36,492,860      branch-misses                    #   17.62% of all branches           

       2.636935865 seconds time elapsed

       2.566932000 seconds user
       0.008011000 seconds sys
*/