#include "test_main.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

void init_tests() { ::testing::InitGoogleMock(); }

void test() {
    if (RUN_ALL_TESTS())
        ;
}

#if defined(ARDUINO)
#include <Arduino.h>

void setup() {
    delay(5000);
    init_tests();
    ::testing::InitGoogleMock();
    test();
}

void loop() {}
#else
int main(int argc, char **argv) {
    init_tests();
    test();
    return 0;
}
#endif
