#pragma once

#define PWR_STARTUP_DELAY_MS (25)

// Wraps I2C/TFT power pin
void init_pwr();
void enable_pwr();
void disable_pwr();
