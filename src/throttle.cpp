
// Throttle pinout
// 3 pin:
// Black - GND
// Red - V+
// Green HALL VOUT (not divided) (1-52V)
// 2 pin:
// Yellow V+
// Blue VOUT

#include "throttle.h"

#if defined(ARDUINO)
#include <framework.h>

#include <cstring>

#include "display.h"
#include "motor_driver.h"
#include "scheduler.h"
#include "trace.h"

#define THROTTLE_UPDATE_RATE_MS (100)
#define THROTTLE_DEFAULT_LIMIT (255)

#define THROTTLE_PIN (A0)

// Hall effect IO at 3.3v
#define THROTTLE_VMAX (2.585)
#define THROTTLE_VOFF (0.85)
#define ADC_MIN (0)
#define ADC_MAX (4095)

#define THROTTLE_LOWPASS_LENGTH (3)
#define THROTTLE_SMOOTHING (16)
#define MAX_THROTTLE_VALUE (3110)
#define MIN_THROTTLE_VALUE (970)
// Clamp throttle to zero below this value
#define THROTTLE_LOWER_DEADBAND (32)
#define THROTTLE_UPPER_DEADBAND (250)
#define THROTTLE_EFFECTIVE_MAXIMUM \
    (THROTTLE_UPPER_DEADBAND - THROTTLE_LOWER_DEADBAND)
// Based on 10' diameter wheel, aprox 40kph
#define RPM_MAX (836)
#define RPM_MIN (0)
#define DUTY_CYCLE_MAX (100000)

namespace {
void update_throttle_cb(void* ctx);
ThrottleData throttle_data;
uint32_t throttle_lowpass;
uint8_t throttle_task;

void update_throttle_cb(void* ctx) {
    // Run a low-pass on throttle data
    uint32_t throttle_sample = analogRead(THROTTLE_PIN);
    throttle_lowpass =
        (throttle_lowpass << THROTTLE_LOWPASS_LENGTH) - throttle_lowpass;
    throttle_lowpass += throttle_sample;
    throttle_lowpass >>= THROTTLE_LOWPASS_LENGTH;
    // Project into 0-255 fixed point
    throttle_data.current = ((255 * (throttle_lowpass - MIN_THROTTLE_VALUE)) /
                             (MAX_THROTTLE_VALUE - MIN_THROTTLE_VALUE)) -
                            THROTTLE_LOWER_DEADBAND;
    if (throttle_data.current < 0) {
        throttle_data.current = 0;
    }
    if (throttle_data.current >
        (THROTTLE_UPPER_DEADBAND - THROTTLE_LOWER_DEADBAND)) {
        throttle_data.current = THROTTLE_UPPER_DEADBAND;
    }
    // Scale throttle to limit
    throttle_data.current = (throttle_data.current * throttle_data.limit) >> 8;
    if (throttle_data.state == THROTTLE_STATE_UNLOCKED) {
        display_set_kph(throttle_data.current);
        // Map throttle to motor command
        int32_t cycle = (throttle_data.current * DUTY_CYCLE_MAX) /
                        THROTTLE_EFFECTIVE_MAXIMUM;
        motor_driver_set_duty_cycle(VESC_FRONT_ADDRESS, cycle);
        motor_driver_set_duty_cycle(VESC_REAR_ADDRESS, cycle);
    }
}
}  // namespace

void throttle_init(Scheduler* sched) {
    ENTER;
    pinMode(THROTTLE_PIN, INPUT);
    if (NULL != sched) {
        throttle_task = sched->register_task(
            "throttle", SCHED_MILLISECONDS(THROTTLE_UPDATE_RATE_MS),
            update_throttle_cb, TASK_FLAG_ENABLED);
    }
    throttle_lowpass = 0;
    throttle_data.limit = THROTTLE_DEFAULT_LIMIT;
    EXIT;
}

void throttle_set_state(uint8_t state) {
    throttle_data.state |= state;
    if (throttle_data.state != THROTTLE_STATE_UNLOCKED) {
        throttle_data.current = 0;
        throttle_lowpass = 0;
    }
}

void throttle_reset_state(uint8_t state) { throttle_data.state &= ~state; }

void throttle_set_limit(int16_t limit) { throttle_data.limit = limit; }
#else
void throttle_init(Scheduler* sched) {}
void throttle_enable(bool enable) {}
void throttle_set_limit(int16_t pt) {}
#endif
