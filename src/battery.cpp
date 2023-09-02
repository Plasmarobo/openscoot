
// Floating point representation of voltage

#include <framework.h>
#include <stdint.h>

#include "display.h"
#include "errors.h"
#include "motor_driver.h"
#include "scheduler.h"
#include "trace.h"

#define PACK_MIN_VOLTAGE (35)
#define PACK_SAFETY_VOLTAGE (45)
#define PACK_MAX_VOLTAGE (54)
#define PACK_AH (36)
#define PACK_INVALID_VOLTAGE (-1)

// We don't need fast battery updates, every 30+ seconds should be fine
#define BATTERY_UPDATE_PERIOD_MS (30000)

namespace {
void update_battery_cb(void* ctx);
volatile int16_t battery_voltage;

void update_battery_cb(void* ctx) {
    ENTER;
    VESCState state = motor_driver_get_state(VESC_FRONT_ADDRESS);
    battery_voltage = state.volts_in / 10;
    if (battery_voltage < PACK_SAFETY_VOLTAGE) {
        report_error("Battery requires charge");
    }
    if (battery_voltage < PACK_MIN_VOLTAGE) {
        report_fault("Battery SoC critical");
    }
    // AH used is in 1/10000 of an amp
    int32_t percent =
        (((PACK_AH * 10000) - state.ah_used) * 100) / (PACK_AH * 10000);
    display_set_battery_charge(percent);
    EXIT;
}
}  // namespace

void battery_init(Scheduler* sched) {
    battery_voltage = PACK_INVALID_VOLTAGE;
    if (NULL != sched) {
        sched->register_task(SCHED_MILLISECONDS(BATTERY_UPDATE_PERIOD_MS),
                             update_battery_cb, TASK_FLAG_ENABLED);
    }
}
void set_battery_voltage(int16_t voltage) { battery_voltage = voltage; }
