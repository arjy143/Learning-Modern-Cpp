#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <new>
#include <mutex>


/*
    This is a simple allocation tracker that overrides new and delete, and keeps track of them and reports what
    memory is not freed, like a simple valgrind.
*/


//issues with using the proper new and delete mean that we have to implement some kind of recursion guard,
//because the allocation tracker itself allocates memory.
thread_local bool g_allocating = false;


struct AllocInfo
{
    std::size_t size;
    const char* file;
    int line;
};

class AllocationTracker
{
    private:
        std::unordered_map<void*, AllocInfo> _allocations;
        std::mutex _mutex;
        AllocationTracker() = default;
    
    public:
        void add(void* ptr, std::size_t size, const char* file, int line)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::cout << "Allocating ptr: " << ptr << "\n";
            _allocations[ptr] = AllocInfo{size, file, line};
            std::cout << "Amount of allocations after add: " << _allocations.size() << "\n";
        }

        void remove(void* ptr)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::cout << "Erasing ptr: " << ptr << "\n";
            _allocations.erase(ptr);
            std::cout << "Amount of allocations after delete: " << _allocations.size() << "\n";

        }

        void reportLeaks()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_allocations.empty())
            {
                std::cout << "No memory leaks detected.\n";
            }
            else
            {
                std::cout << "Memory leaks were detected:\n";
                for (auto& [ptr, info] : _allocations)
                {

                    std::cout << "File " << info.file << " at line " << info.line << "\n->Pointer: " << ptr << ", Bytes: " << info.size << "\n";
                }
            }
        }

        //singleton class
        static AllocationTracker& instance()
        {
            static AllocationTracker* tracker = new AllocationTracker();
            return *tracker;
        }
};


//global overrides of new and delete

void* operator new(std::size_t size, const char* file, int line)
{
    void* ptr = std::malloc(size);

    if (!ptr)
    {
        throw std::bad_alloc();
    }
    if (!g_allocating) 
    {
        std::cout << "using custom new: " << ptr << "\n";

        g_allocating = true;
        AllocationTracker::instance().add(ptr, size, file, line);
        g_allocating = false;
    }

    return ptr;
}

void operator delete(void* ptr) noexcept
{
    if (!g_allocating) 
    {
        std::cout << "using custom delete: " << ptr << "\n";

        g_allocating = true;
        AllocationTracker::instance().remove(ptr);
        g_allocating = false;
    }
    
    std::free(ptr);
}

void* operator new[](std::size_t size, const char* file, int line) 
{
    return ::operator new(size, file, line);
}
void operator delete[](void* ptr) noexcept { ::operator delete(ptr); }

void operator delete(void* ptr, std::size_t) noexcept { ::operator delete(ptr); }
void operator delete[](void* ptr, std::size_t) noexcept { ::operator delete(ptr); }


#define new new(__FILE__, __LINE__)

int main()
{
    int* a = new int;
    int* b = new int[10];

    delete a;

    AllocationTracker::instance().reportLeaks();

    return 0;
}


// compile using:
// g++ -std=c++17 -pthread -Wall -Wextra -O2 AllocationTracker.cpp -o AllocationTracker.exe


// using custom new: 0x62b1f772f2b0
// Allocating ptr: 0x62b1f772f2b0
// Amount of allocations after add: 1
// using custom new: 0x62b1f772f7f0
// Allocating ptr: 0x62b1f772f7f0
// Amount of allocations after add: 2
// using custom delete: 0x62b1f772f2b0
// Erasing ptr: 0x62b1f772f2b0
// Amount of allocations after delete: 1
// Memory leaks were detected:
// File AllocationTracker.cpp at line 129
// ->Pointer: 0x62b1f772f7f0, Bytes: 40