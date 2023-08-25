#include <gtest/gtest.h>

#include "ringbuffer.hpp"

namespace {
const size_t N_ELEMENTS = 8;
}

TEST(RingbufferTest, PushElement) {
    RingBuffer<int32_t, N_ELEMENTS> buffer;
    int32_t test_elem = 42;
    EXPECT_TRUE(buffer.empty());
    EXPECT_TRUE(buffer.push(&test_elem));
    EXPECT_EQ(test_elem, 42);
    EXPECT_FALSE(buffer.empty());
    EXPECT_FALSE(buffer.full());
}
TEST(RingbufferTest, PopElement) {
    RingBuffer<int32_t, N_ELEMENTS> buffer;
    int32_t test_elem = 32;
    EXPECT_TRUE(buffer.empty());
    EXPECT_TRUE(buffer.push(&test_elem));
    EXPECT_FALSE(buffer.empty());
    EXPECT_FALSE(buffer.full());
    int32_t check = 0;
    EXPECT_TRUE(buffer.pop(&check));
    EXPECT_EQ(check, test_elem);
    EXPECT_FALSE(buffer.pop(&check));
    EXPECT_EQ(check, test_elem);
}

TEST(RingbufferTest, Full) {
    RingBuffer<int32_t, 2> buffer;
    int32_t a = 255;
    int32_t b = 76;
    EXPECT_TRUE(buffer.empty());
    EXPECT_TRUE(buffer.push(&a));
    EXPECT_TRUE(buffer.push(&b));
    EXPECT_FALSE(buffer.empty());
    EXPECT_TRUE(buffer.full());
    int32_t c, d, e;
    c = d = e = 0;
    EXPECT_TRUE(buffer.pop(&c));
    EXPECT_TRUE(buffer.pop(&d));
    EXPECT_FALSE(buffer.pop(&e));
    EXPECT_EQ(c, a);
    EXPECT_EQ(d, b);
    EXPECT_EQ(e, 0);
}

TEST(RingBufferTest, Empty) {
    RingBuffer<int32_t, N_ELEMENTS> buffer;
    int32_t test_elem = 32;
    EXPECT_TRUE(buffer.empty());
    EXPECT_TRUE(buffer.push(&test_elem));
    EXPECT_FALSE(buffer.empty());
    EXPECT_FALSE(buffer.full());
    int32_t check = 0;
    EXPECT_TRUE(buffer.pop(&check));
    EXPECT_EQ(check, test_elem);
    EXPECT_FALSE(buffer.pop(&check));
    EXPECT_EQ(check, test_elem);
    EXPECT_TRUE(buffer.empty());
}

#include "../src/test_main.cpp"
