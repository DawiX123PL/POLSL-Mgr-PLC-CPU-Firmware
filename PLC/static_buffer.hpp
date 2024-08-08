#pragma once

#include <inttypes.h>

template <typename T, uint32_t _capacity>
class StaticBuffer
{
    T buffer[_capacity];
    uint32_t _size;

public:
    StaticBuffer() : _size(0) {};

    bool Full()
    {
        return _size >= _capacity;
    }

    void Clear()
    {
        _size = 0;
    }

    void PushBack(T element)
    {
        if (Full())
            return;
        buffer[_size++] = element;
    }

    uint32_t Size() const
    {
        return _size;
    }

    const T *Get() const
    {
        return buffer;
    }
};
