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
        auto full() const { return size() == capacity(); }
        auto push(T const& value);
        auto pop(T* value); 
};