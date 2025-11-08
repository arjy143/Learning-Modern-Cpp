/* Queue - FIFO
    Looking into creating a single producer, single consumer lock free FIFO.
    Inspired by CPPCon talk by Charles Frasch: https://www.youtube.com/watch?v=K3P_Lmq6pw0&t=2368s

    1 writer, 1 reader. No mutexes, oldest entry processed first.
    An example of usage is if you had a thread that was reading from a socket, it could push to a queue that is read by a thread that handles messages, which could push to a queue that is read by a thread that writes a response to a socket.

    Whenever optimising around any array or buffer, always use a power of 2.
    if you want to constrain an index into an array like this, use a bitwise and, not division/modulus.
*/

//initial naive approach

#include <memory>
//allocator is just the default c++ memory allocator
template<typename T, typename Alloc = std::allocator<T>>
//inheriting from private alloc is actually an empty base optimisation trick
// this is where you dont need to store the allocator itself as a member, saving space.
class Fifo1: private Alloc
{
    //these are not actually indices themselves, they will have to be modulo'ed at some point
    std::size_t capacity;
    T* ring;
    std::size_t pushCursor{};
    std::size_t popCursor{};

    //std::allocator_traits is just a wrapper that allows for generic allocator calls.
    //provides static methods used below
    public:
        explicit Fifo1(std::size_t capacity, Alloc const& alloc = Alloc{})
            : Alloc{alloc}, capacity(capacity), ring{std::allocator_traits::allocate(*this, capacity)}{}
        
        ~Fifo1()
        {
            while (!empty())
            {
                ring[popCursor % capacity].~T();
                ++popCursor;
            }
            std::allocator_traits::deallocate(*this, ring, capacity);
        }

        auto capacity() const { return capacity; }
        auto size() const { return pushCursor - popCursor; }
        auto empty() const { return size() == 0; }
        auto full() const { return (size() == capacity()); }
        auto push(T const& value);
        auto pop(T* value); 

        auto push(T const& value)
        {
            if (full())
            {
                return false;
            }
            std::allocator_traits<Alloc>::construct(*this, ring + (pushCursor % capacity), value)
            ++pushCursor;

            return true;
        }

        auto pop(T& value)
        {
            if (empty())
            {
                return false;
            }
            value = ring[popCursor % capacity];
            ring[popCursor % capacity].~T();
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
    Fifo1<int> q(1024);

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

        q.pop(&val);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time = end - start;
    double ops_per_sec = N / elapsed_time.count();

    std::cout << "N: " << N << "\n";
    std::cout << "time: " << elapsed_time.count() << "\n";
    std::cout << "ops_per_sec: " << ops_per_sec << "\n";

    return 0;
}