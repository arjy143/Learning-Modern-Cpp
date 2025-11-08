/* Queues
    Looking into creating a single producer, single consumer lock free FIFO.
    Inspired by CPPCon talk by Charles Frasch: https://www.youtube.com/watch?v=K3P_Lmq6pw0&t=2368s

    1 writer, 1 reader. No mutexes, oldest entry processed first.
    An example of usage is if you had a thread that was reading from a socket, it could push to a queue that is read by a thread that handles messages, which could push to a queue that is read by a thread that writes a response to a socket.

    Whenever optimising around any array or buffer, always use a power of 2.
    if you want to constrain an index into an array like this, use a bitwise and, not division/modulus.
*/

//initial naive approach - FIFO1

#include <memory>
//allocator is just the default c++ memory allocator
template<typename T, typename Alloc = std::allocator<T>>
//inheriting from private alloc is actually an empty base optimisation trick
// this is where you dont need to store the allocator itself as a member, saving space.
class Fifo1: private Alloc
{
    //these are not actually indices themselves, they will have to be modulo'ed at some point
    std::size_t _capacity;
    T* ring;
    std::size_t pushCursor{};
    std::size_t popCursor{};

    //std::allocator_traits is just a wrapper that allows for generic allocator calls.
    //provides static methods used below
    public:
        explicit Fifo1(std::size_t capacity, Alloc const& alloc = Alloc{})
            : Alloc{alloc}, _capacity(capacity), ring{std::allocator_traits<Alloc>::allocate(*this, capacity)}{}
        
        ~Fifo1()
        {
            while (!empty())
            {
                ring[popCursor % _capacity].~T();
                ++popCursor;
            }
            std::allocator_traits<Alloc>::deallocate(*this, ring, _capacity);
        }

        auto capacity() const { return _capacity; }
        auto size() const { return pushCursor - popCursor; }
        auto empty() const { return size() == 0; }
        auto full() const { return (size() == capacity()); }
        // auto push(T const& value);
        // auto pop(T* value); 

        auto push(T const& value)
        {
            if (full())
            {
                return false;
            }
            std::allocator_traits<Alloc>::construct(*this, ring + (pushCursor % _capacity), value);
            ++pushCursor;

            return true;
        }

        auto pop(T& value)
        {
            if (empty())
            {
                return false;
            }
            value = ring[popCursor % _capacity];
            ring[popCursor % _capacity].~T();
            ++popCursor;

            return true;
        }
};

/* FIFO2 - using atomics
    We could use mutexes, but then it wouldnt be lock free, so we use atomics instead.
    ensure that the push and pop cursors are atomic so that a sequentially consistent ordering is maintained.

*/
#include <atomic>

template<typename T, typename Alloc = std::allocator<T>>
class Fifo2: private Alloc
{
    std::size_t _capacity;
    T* ring;
    std::atomic<std::size_t> pushCursor{};
    std::atomic<std::size_t> popCursor{};

    static_assert(std::atomic<std::size_t>::is_always_lock_free);

    public:
        explicit Fifo2(std::size_t capacity, Alloc const& alloc = Alloc{})
            : Alloc{alloc}, _capacity(capacity), ring{std::allocator_traits<Alloc>::allocate(*this, capacity)}{}
        
        ~Fifo2()
        {
            while (!empty())
            {
                ring[popCursor % _capacity].~T();
                ++popCursor;
            }
            std::allocator_traits<Alloc>::deallocate(*this, ring, _capacity);
        }

        auto capacity() const { return _capacity; }
        auto size() const { return pushCursor - popCursor; }
        auto empty() const { return size() == 0; }
        auto full() const { return (size() == capacity()); }

        auto push(T const& value)
        {
            if (full())
            {
                return false;
            }
            std::allocator_traits<Alloc>::construct(*this, ring + (pushCursor % _capacity), value);
            ++pushCursor;

            return true;
        }

        auto pop(T& value)
        {
            if (empty())
            {
                return false;
            }
            value = ring[popCursor % _capacity];
            ring[popCursor % _capacity].~T();
            ++popCursor;

            return true;
        }
};



//testing code

#include <chrono>
#include <iostream>

int main()
{
    //testing on 1 million 
    constexpr const std::size_t N = 1000000;
    
    //capacity of 1024
    Fifo2<int> q(1024);

    // for (int i = 0; i < 100; i++)
    // {
    //     q.push(i);
    //     q.pop(&i);
    // }

    auto start = std::chrono::high_resolution_clock::now();

    int val = 0;
    for (std::size_t i = 0; i < N; ++i)
    {
        //spin if the queue is full
        while (q.full()){}

        q.push(i);

        //spin if empty
        while (q.empty()){}

        q.pop(val);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time = end - start;
    double ops_per_sec = N / elapsed_time.count();

    std::cout << "N: " << N << "\n";
    std::cout << "time: " << elapsed_time.count() << "\n";
    std::cout << "ops_per_sec: " << ops_per_sec << "\n";

    return 0;
}


/* FIFO1:
N: 1000000
time: 0.00338514
ops_per_sec: 2.95409e+08

              2.48 msec task-clock                       #    0.849 CPUs utilized             
                 2      context-switches                 #  805.064 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
               133      page-faults                      #   53.537 K/sec                     
         6,916,923      cycles                           #    2.784 GHz                       
        10,893,938      instructions                     #    1.57  insn per cycle            
         1,684,967      branches                         #  678.253 M/sec                     
            23,220      branch-misses                    #    1.38% of all branches           

       0.002925328 seconds time elapsed

       0.000000000 seconds user
       0.002995000 seconds sys
*/

/* FIFO2
N: 1000000
time: 0.0927534
ops_per_sec: 1.07813e+07

             32.16 msec task-clock                       #    0.969 CPUs utilized             
                 6      context-switches                 #  186.560 /sec                      
                 2      cpu-migrations                   #   62.187 /sec                      
               133      page-faults                      #    4.135 K/sec                     
        90,393,435      cycles                           #    2.811 GHz                       
        35,259,930      instructions                     #    0.39  insn per cycle            
         5,743,692      branches                         #  178.590 M/sec                     
            25,372      branch-misses                    #    0.44% of all branches           

       0.033184534 seconds time elapsed

       0.028763000 seconds user
       0.003967000 seconds sys
*/