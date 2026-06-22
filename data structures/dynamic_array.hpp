#pragma once

<template typename T>
class DynamicArray
{
private:
    T* buffer_;
    size_t capacity_; 

public:
    DynamicArray(capacity) : capacity_(capacity)
    {}

    void push_back(const T& item)
    {

    }

    void erase(const T& item)
    {

    }

    void emplace_back(const T& item)
    {

    }

    T& operator[](size_t index)
    {
        return buffer_[index];
    }

};