#include "pwr.h"

#include <framework.h>

void init_pwr() {
    pinMode(TFT_I2C_POWER, OUTPUT);
    pinMode(NEOPIXEL_POWER, OUTPUT);
}

void enable_pwr() {
    digitalWrite(TFT_I2C_POWER, HIGH);
    digitalWrite(NEOPIXEL_POWER, HIGH);
}

void disable_pwr() {
    digitalWrite(TFT_I2C_POWER, LOW);
    digitalWrite(NEOPIXEL_POWER, LOW);
}
