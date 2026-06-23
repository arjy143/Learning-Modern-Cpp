#pragma once
#include <cstddef>
#include <cstdlib>
#include <new>
#include <utility>
#include <cstring>
#include <type_traits>

template <typename T>
class DynamicArray
{
private:
    T* buffer_;
    size_t capacity_; 
    size_t item_count_;

    void grow()
    {
        reserve(capacity_ ? capacity_ * 2 : 1);
    }


public:
    DynamicArray() : capacity_(1), item_count_(0)
    {
        buffer_ = static_cast<T*>(malloc(sizeof(T) * capacity_));        
    }

    //explicit needed because otherwise the compiler would treat size_t as implicitly convertable to a DynamicArray
    explicit DynamicArray(size_t capacity) : capacity_(capacity ? capacity : 1), item_count_(0)
    {
        buffer_ = static_cast<T*>(malloc(sizeof(T) * capacity_));        
    }

    DynamicArray(const DynamicArray& other)
    {
        T* new_buffer = static_cast<T*>(malloc(sizeof(T) * other.capacity_));

        for (size_t i = 0; i < other.item_count_; ++i)
        {
            //copy each item to the new buffer
            ::new (static_cast<void*>(new_buffer + i)) T(other.buffer_[i]);
        }

        this->buffer_ = new_buffer;
        this->capacity_ = other.capacity_;
        this->item_count_= other.item_count_;
    }

    DynamicArray& operator=(DynamicArray other) noexcept
    {

        std::swap(this->buffer_, other.buffer_);
        std::swap(this->capacity_, other.capacity_);
        std::swap(this->item_count_, other.item_count_);

        return *this;
    }

    DynamicArray(DynamicArray&& other) noexcept : buffer_(other.buffer_), capacity_(other.capacity_), item_count_(other.item_count_) 
    {
        other.buffer_ = nullptr;
        other.capacity_= 0;
        other.item_count_ = 0;
    }

    ~DynamicArray()
    {
        //destruct each item then free the allocated memory
        for (size_t i = 0; i < item_count_; ++i)
        { 
            buffer_[i].~T();
        }

        free(buffer_);
    }

    void push_back(const T& item)
    {
        emplace_back(item);
    }

    //forward referencing pack - keep track of lvalues and rvalues
    template <typename... Args>
    void emplace_back(Args&&... args)
    {
        if (item_count_ >= capacity_)
        {
            grow();
        }

        //perfect forwarding of all arguments into T's constructor, which removes the need for any temporary copy when constructing the item.
        ::new (static_cast<void*>(buffer_ + item_count_)) T(std::forward<Args>(args)...);
        ++item_count_;
    }

    void reserve(size_t new_capacity)
    {
        if (new_capacity <= capacity_)
        {
            return;
        }

        T* new_buffer = static_cast<T*>(malloc(sizeof(T) * new_capacity));

        //small optimisation to do a single copy instead of multiple moves as long as the object only consists of its bytes.
        //constexpr to allow branching at compile time
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            memcpy(new_buffer, buffer_, item_count_ * sizeof(T));
        }
        else
        {
            for (size_t i = 0; i < item_count_; ++i)
            {
                //move each item to the new buffer
                //move_if_noexcept just in case T's move constructor is not noexcept. If so, it will fall back to copying
                ::new (static_cast<void*>(new_buffer + i)) T(std::move_if_noexcept(buffer_[i]));

                //destruct the old item
                buffer_[i].~T();
            }
        }

        free(buffer_);
        buffer_ = new_buffer;
        capacity_ = new_capacity;
    }

    T& operator[](size_t index)
    {
        return buffer_[index];
    }

    
    //readonly
    const T& operator[](size_t index) const
    {
        return buffer_[index];
    }

    //mutable iterators
    T* begin()
    {
        return buffer_;
    }

    T* end()
    {
        return buffer_ + item_count_;
    }

    //immutable iterators
    const T* begin() const
    {
        return buffer_;
    }

    const T* end() const
    {
        return buffer_ + item_count_;
    }
    //helpers
    [[nodiscard]] size_t size() const
    {
        return item_count_;
    }

    [[nodiscard]] size_t capacity() const
    {
        return capacity_;
    }

    [[nodiscard]] bool empty() const
    {
        return item_count_ == 0;
    }
};