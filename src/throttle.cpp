
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

#define THROTTLE_UPDATE_RATE_MS (10)
#define THROTTLE_DEFAULT_LIMIT (128)

#define THROTTLE_PIN (A0)
// Hall effect IO at 3.3v
#define THROTTLE_VMAX (2.585)
#define THROTTLE_VOFF (0.85)
#define ADC_MIN (0)
#define ADC_MAX (4095)

#define THROTTLE_SAMPLE_WINDOW (5)
#define THROTTLE_SMOOTHING (16)
#define MAX_THROTTLE_VALUE (3110)
#define MIN_THROTTLE_VALUE (985)

// Based on 10' diameter wheel, aprox 40kph
#define RPM_MAX (836)
#define RPM_MIN (0)
#define DUTY_CYCLE_MAX (100000)

namespace {
void update_throttle_cb(void* ctx);
ThrottleData throttle_data;
int16_t throttle_samples[THROTTLE_SAMPLE_WINDOW];
uint8_t sample_idx;
uint8_t throttle_task;

void update_throttle_cb(void* ctx) {
    throttle_samples[sample_idx] = analogRead(THROTTLE_PIN);
    sample_idx = (sample_idx + 1) % THROTTLE_SAMPLE_WINDOW;
    int16_t desired_throttle = 0;
    for (uint8_t idx = 0; idx < THROTTLE_SAMPLE_WINDOW; ++idx) {
        desired_throttle += throttle_samples[idx];
    }
    desired_throttle /= THROTTLE_SAMPLE_WINDOW;
    uint8_t projected_throttle =
        (255 * (desired_throttle - MIN_THROTTLE_VALUE)) /
        (MAX_THROTTLE_VALUE - MIN_THROTTLE_VALUE);
    int16_t delta = projected_throttle - throttle_data.current;
    if (delta > THROTTLE_SMOOTHING) {
        delta = THROTTLE_SMOOTHING;
    } else if (delta < -THROTTLE_SMOOTHING) {
        delta = -THROTTLE_SMOOTHING;
    }
    if (delta != 0) {
        throttle_data.current += delta;
        if (throttle_data.current > throttle_data.limit) {
            throttle_data.current = throttle_data.limit;
        }
        // Map throttle to motor command
        int32_t cycle = (throttle_data.current * DUTY_CYCLE_MAX) / 255;
        display_set_kph(KPH_RESOLUTION * throttle_data.current);
        display_printf("DC: %06d", cycle);
        motor_driver_set_duty_cycle(VESC_FRONT_ADDRESS, cycle);
        motor_driver_set_duty_cycle(VESC_REAR_ADDRESS, cycle);
    }
}
}  // namespace

void throttle_init(Scheduler* sched) {
    if (NULL != sched) {
        throttle_task =
            sched->register_task(SCHED_MILLISECONDS(THROTTLE_UPDATE_RATE_MS),
                                 update_throttle_cb, TASK_FLAG_ENABLED);
    }
    memset(&throttle_samples, 0, sizeof(int16_t) * THROTTLE_SAMPLE_WINDOW);
    sample_idx = 0;
    throttle_data.limit = THROTTLE_DEFAULT_LIMIT;
}

void throttle_enable(bool enable) { throttle_data.enabled = enable; }

void throttle_set_limit(int16_t limit) {
    throttle_data.limit = limit;
    display_set_kph_limit(limit);
}
#else
void throttle_init(Scheduler* sched) {}
void throttle_enable(bool enable) {}
void throttle_set_limit(int16_t pt) {}
#endif
