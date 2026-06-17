#include "lru_cache.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    size_t   capacity  = argc > 1 ? strtoull(argv[1], nullptr, 10) : 100000;
    uint64_t num_ops   = argc > 2 ? strtoull(argv[2], nullptr, 10) : 20000000;
    uint64_t key_space = argc > 3 ? strtoull(argv[3], nullptr, 10) : 300000;
    uint64_t seed      = argc > 4 ? strtoull(argv[4], nullptr, 10) : 12648430;

    LRUCache<uint64_t, uint64_t> cache(capacity);

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

/*
 Performance counter stats for './bench1':

          6,607.25 msec task-clock                       #    0.987 CPUs utilized             
             1,161      context-switches                 #  175.716 /sec                      
                54      cpu-migrations                   #    8.173 /sec                      
             2,716      page-faults                      #  411.064 /sec                      
    11,362,904,345      cycles                           #    1.720 GHz                       
     2,992,079,601      instructions                     #    0.26  insn per cycle            
       590,928,605      branches                         #   89.436 M/sec                     
        28,068,628      branch-misses                    #    4.75% of all branches           

       6.697440195 seconds time elapsed

       6.561602000 seconds user
       0.040132000 seconds sys
*/