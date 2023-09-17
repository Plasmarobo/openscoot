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
    static_assert(N > 0, "Empty pool not allowed");
    static_assert(N % 8 == 0, "Template parameter should be a multiple of 8");
    T pool[N];
    // Fast ceiling here to calc number of 8 bit blocks required in freelist
    uint8_t freelist[N + 7 / 8];

   public:
    MemPool() {
        for (uint32_t i = 0; i < N; ++i) {
            freelist[i] = 0xFF;
        }
    }

    T* alloc() {
        // Scan for a free block
        uint32_t idx = 0;
        for (uint32_t block = 0; block < N; ++block) {
            for (uint8_t bit = 0; bit < 8; ++bit) {
                if (freelist[block] & (1 << bit)) {
                    idx = (block * 8) + bit;
                    freelist[block] &= ~(1 << bit);
                    break;
                }
            }
        }

        T* ptr = &pool[idx];
        return ptr;
    }

    void free(T* ptr) {
        uintptr_t item = (uintptr_t)ptr;
        uintptr_t root = (uintptr_t)&pool[0];
        if (item >= root) {
            uintptr_t idx = (item - root) / sizeof(T);
            if (idx >= N) {
                return;
            }
            // Mark the freelist
            uint32_t block = idx / 8;
            uint8_t bit = idx % 8;
            freelist[block] |= (1 << bit);
        }
    }
};
