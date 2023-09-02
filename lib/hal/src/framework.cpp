#include "framework.h"
#if !defined(ARDUINO)
#include <chrono>
#include <thread>
void attachInterrupt(uint8_t pin, void (*)(), uint8_t mode) {}
void pinMode(uint8_t pin, uint8_t mode) {}
void digitalWrite(uint8_t pin, uint8_t value) {}
uint8_t digitalRead(uint8_t pin) { return 0; }
int16_t analogRead(uint8_t pin) { return 0; }
static uint64_t micros_count = 0;
void delay(uint64_t ms) {
    micros_count += ms * 1000;
    // std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
void delay_us(uint64_t us) { micros_count += us; }
uint64_t micros() { return micros_count; }
uint64_t millis() { return micros() / 1000; }
#endif
