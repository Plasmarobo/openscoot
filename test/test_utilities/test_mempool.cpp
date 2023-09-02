#include <gtest/gtest.h>
#include <test_main.h>

#include <mempool.hpp>

#define POOL_SIZE (32)

namespace {
struct TestStruct {
    uint8_t a;
    uint32_t b;
    uint8_t c;
};
}  // namespace

TEST(Mempool, Alloc) {
    MemPool<TestStruct, POOL_SIZE> pool;
    TestStruct* item = pool.alloc();
    EXPECT_NE((uintptr_t)item, NULL);
    item->a = 1;
    item->b = 2;
    item->c = 3;
}

TEST(Mempool, SequentialFree) {
    MemPool<TestStruct, POOL_SIZE> pool;
    uint8_t n = 8;
    TestStruct* item[8];
    for (uint8_t i = 0; i < n; ++i) {
        item[i] = pool.alloc();
        EXPECT_NE((uintptr_t)item[i], NULL);
    }
    for (uint8_t i = 0; i < n; ++i) {
        pool.free(item[i]);
    }
}

TEST(Mempool, AllocAll) {
    const uint8_t n = 8;
    MemPool<TestStruct, n> pool;
    TestStruct* item[n];
    for (uint8_t j = 0; j < 4; ++j) {
        for (uint8_t i = 0; i < n; ++i) {
            item[i] = pool.alloc();
            EXPECT_NE((uintptr_t)item[i], NULL);
        }
        for (uint8_t i = 0; i < n; ++i) {
            pool.free(item[i]);
        }
    }
}

TEST(Mempool, RandomFree) {
    const uint8_t n = 8;
    MemPool<TestStruct, n> pool;
    uint8_t index_buffer[] = {6, 1, 2, 4, 3, 7, 0, 5};
    TestStruct* item[n];
    for (uint8_t j = 0; j < 4; ++j) {
        for (uint8_t i = 0; i < n; ++i) {
            item[i] = pool.alloc();
            EXPECT_NE((uintptr_t)item[i], NULL);
        }
        for (auto index : index_buffer) {
            pool.free(item[index]);
        }
    }
}
