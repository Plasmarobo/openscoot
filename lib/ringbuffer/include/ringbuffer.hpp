#pragma once

#include <stddef.h>
#include <stdint.h>
#include <trace.h>

#include <cstring>

template <typename T, size_t N>
class RingBuffer {
   private:
    bool _empty;
    uint8_t head;
    uint8_t tail;
    T buffer[N];

   public:
    RingBuffer();
    bool push(T* t);
    T* writeable();
    bool pop(T* t);
    T* readable();
    bool full();
    bool empty();
};

template <typename T, size_t N>
RingBuffer<T, N>::RingBuffer() {
    static_assert(N < 256, "RingBuffer size cannot exceed 255");
    static_assert(N > 1, "RingBuffer must have at least 2 elements");
    _empty = true;
    head = 0;
    tail = 0;
}

template <typename T, size_t N>
bool RingBuffer<T, N>::push(T* t) {
    if (full()) {
        TRACE("Ringbuffer full\n");
        return false;
    }
    if (NULL == t) {
        TRACE("NULL object\n");
        return false;
    }
    memcpy((uint8_t*)&buffer[head], (uint8_t*)t, sizeof(T));
    _empty = false;
    head = (head + 1) % N;
    return true;
}

// Dangerous function, use with caution
template <typename T, size_t N>
T* RingBuffer<T, N>::writeable() {
    if (full()) {
        return NULL;
    }
    T* ptr = &buffer[head];
    _empty = false;
    head = (head + 1) % N;
    return ptr;
}

template <typename T, size_t N>
bool RingBuffer<T, N>::pop(T* t) {
    if (empty()) {
        return false;
    }
    if (NULL == t) {
        return false;
    }
    memcpy((uint8_t*)t, (uint8_t*)&buffer[tail], sizeof(T));
    tail = (tail + 1) % N;
    if (tail == head) {
        _empty = true;
    }
    return true;
}

template <typename T, size_t N>
T* RingBuffer<T, N>::readable() {
    if (empty()) {
        return NULL;
    }
    T* ptr = &buffer[tail];
    tail = (tail + 1) % N;
    if (tail == head) {
        _empty = true;
    }
    return ptr;
}

template <typename T, size_t N>
bool RingBuffer<T, N>::full() {
    return (this->head == this->tail) && !_empty;
}

template <typename T, size_t N>
bool RingBuffer<T, N>::empty() {
    return (this->head == this->tail) && _empty;
}
