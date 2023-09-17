
#include "speed.h"

#include <scheduler.h>

#include "display.h"
#include "motor_driver.h"

#define SPEED_PERIOD_MS (200)
#define WHEEL_CIRC_MM (798)

namespace {
Scheduler* sched_ref;
int32_t mmph;

void speed_update_cb(void* ctx) {
    VESCState state = motor_driver_get_state(VESC_FRONT_ADDRESS);
    // Millimeter per minute
    int32_t mmpm = (state.erpm * WHEEL_CIRC_MM) / 20;
    // Meter per minute
    int32_t mpm = (mmpm / 1000);
    // Meters per hour
    int32_t mph = mpm * 60;
    // 10s of kph (factor of 10)
    int16_t kph_fixedpoint = mph / 100;
    display_set_kph(kph_fixedpoint);
}
}  // namespace

void speed_init(Scheduler* sched) {
    sched_ref = sched;

    display_set_kph(0);
    if (sched) {
        sched->register_task("speed", SCHED_MILLISECONDS(SPEED_PERIOD_MS),
                             speed_update_cb, TASK_FLAG_ENABLED);
    }
}
