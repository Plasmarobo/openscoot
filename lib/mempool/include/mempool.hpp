#pragma once

#include <math.h>
#include <stddef.h>
#include <string.h>
#include <trace.h>

#define _DEBUG

constexpr bool is_powerof2(int v) { return v && ((v & (v - 1)) == 0); }

template <typename T, size_t N>
class MemPool {
   private:
    static_assert(is_powerof2(N), "Template parameter should be a power of 2");
    static_assert(N < 64, "Template parameter should be less than 64");
    T pool[N];
    uint64_t freelist;

    uint8_t get_free_idx();

   public:
    MemPool() { freelist = 0xFFFFFFFFFFFFFFFF; }

    T* alloc() {
        // Scan for a free block
        if (freelist == 0) {
            return NULL;
        }

        uint32_t idx = 0;
        for (uint32_t i = 0; i < N; ++i) {
            uint64_t mask = (1 << i);
            if (freelist & mask) {
                idx = i;
                break;
            }
        }

        T* ptr = &pool[idx];
        freelist &= ~(1 << idx);
        return ptr;
    }

    void free(T* ptr) {
        uintptr_t item = (uintptr_t)ptr;
        uintptr_t root = (uintptr_t)&pool[0];
        if (item >= root) {
            uintptr_t idx = (item - root) / sizeof(T);
            // Mark the freelist
            if (idx < N) {
                freelist |= (1 << idx);
            }
        }
    }
};
