#pragma once
#include <cstddef>
#include <cstdlib>
#include <new>
#include <utility>

template <typename T>
class DynamicArray
{
private:
    T* buffer_;
    size_t capacity_; 
    size_t item_count_;

public:
    DynamicArray(size_t capacity) : capacity_(capacity ? capacity : 1), item_count_(0)
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

    DynamicArray(DynamicArray&& other) noexcept : buffer_(other.buffer), capacity_(other.capacity_), item_count_(other.item_count_) 
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
        if (item_count_ >= capacity_)
        {
            size_t new_capacity = capacity_ * 2;

            
            T* new_buffer = static_cast<T*>(malloc(sizeof(T) * new_capacity));

            for (size_t i = 0; i < item_count_; ++i)
            {
                //move each item to the new buffer
                ::new (static_cast<void*>(new_buffer + i)) T(std::move(buffer_[i]));

                //destruct the old item
                buffer_[i].~T();
            }

            free(buffer_);
            buffer_ = new_buffer;
            capacity_ = new_capacity;
        }

        //placement copy construct
        ::new (static_cast<void*>(buffer_ + item_count_)) T(item);
        ++item_count_;

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
};