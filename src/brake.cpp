#include "brake.h"

#include <framework.h>

#include <cstring>

#include "display.h"
#include "motor_driver.h"
#include "scheduler.h"
#include "throttle.h"
#include "trace.h"

#define BRAKE_UPDATE_RATE_MS (100)

#if defined(ARDUINO)
#define BRAKE_PIN (A5)
// Hall effect IO at 3.3v
#define BRAKE_VMAX (2.585)
#define BRAKE_VOFF (0.85)
#define ADC_MIN (0)
#define ADC_MAX (4095)

#define BRAKE_LOWPASS_LENGTH (2)
#define BRAKE_SMOOTHING (16)
#define MAX_BRAKE_VALUE (3110)
#define MIN_BRAKE_VALUE (970)
// Clamp brake to zero below this value
#define BRAKE_LOWER_DEADBAND (90)
#define BRAKE_UPPER_DEADBAND (250)
#define BRAKE_EFFECTIVE_MAXIMUM (BRAKE_UPPER_DEADBAND - BRAKE_LOWER_DEADBAND)

namespace {
void update_brake_cb(void* ctx);
BrakeData brake_data;
uint32_t brake_lowpass;
uint8_t brake_task;

void update_brake_cb(void* ctx) {
    // Run a low-pass on brake data
    uint32_t brake_sample = analogRead(BRAKE_PIN);
    brake_lowpass = (brake_lowpass << BRAKE_LOWPASS_LENGTH) - brake_lowpass;
    brake_lowpass += brake_sample;
    brake_lowpass >>= BRAKE_LOWPASS_LENGTH;
    // Project into 0-255 fixed point
    brake_data.value = ((255 * (brake_lowpass - MIN_BRAKE_VALUE)) /
                        (MAX_BRAKE_VALUE - MIN_BRAKE_VALUE)) -
                       BRAKE_LOWER_DEADBAND;

    if (brake_data.value < 0) {
        brake_data.value = 0;
    }
    if (brake_data.value > (BRAKE_UPPER_DEADBAND - BRAKE_LOWER_DEADBAND)) {
        brake_data.value = BRAKE_UPPER_DEADBAND;
    }
    if (brake_data.value > 0) {
        display_printf("BRAKE");
        throttle_enable(false);
        motor_driver_set_duty_cycle(VESC_FRONT_ADDRESS, 0);
        motor_driver_set_duty_cycle(VESC_REAR_ADDRESS, 0);
    } else {
        display_printf("");
        throttle_enable(true);
    }
}
}  // namespace

void brake_init(Scheduler* sched) {
    ENTER;
    if (NULL != sched) {
        brake_task = sched->register_task(
            "brake", SCHED_MILLISECONDS(BRAKE_UPDATE_RATE_MS), update_brake_cb,
            TASK_FLAG_ENABLED);
    }
    brake_lowpass = 0;
    EXIT;
}
#else
void brake_init(Scheduler* sched) {}
#endif
