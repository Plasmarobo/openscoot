#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include <stddef.h>
#include <stdint.h>

#include <cstdarg>
#include <cstdio>
#include <iostream>

#if defined(ARDUINO)
#include <Arduino.h>
#else

#define PROGMEM

#define LOW (0)
#define HIGH (1)

#define RISING (0)
#define FALLING (1)
#define CHANGE (2)

#define OUTPUT (1)
#define INPUT (2)
#define INPUT_PULLUP (3)

#define NEOPIXEL_POWER (0)
#define A0 (1)
#define A1 (2)
#define A2 (3)
#define A3 (4)
#define TFT_I2C_POWER (5)

void attachInterrupt(uint8_t pin, void (*)(), uint8_t mode);
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);
uint8_t digitalRead(uint8_t pin);
int16_t analogRead(uint8_t pin);
void delay(uint64_t ms);
void delay_us(uint64_t us);
uint64_t micros();
uint64_t millis();

#endif

#endif  // FRAMEWORK_H
