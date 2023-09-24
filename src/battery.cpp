
// Floating point representation of voltage

#include <framework.h>
#include <stdint.h>

#include "display.h"
#include "errors.h"
#include "motor_driver.h"
#include "scheduler.h"
#include "telemetry.h"
#include "trace.h"

// Volts are measured in decavolts (1/10th of a volt)
#define PACK_MIN_VOLTAGE (350)
#define PACK_SAFETY_VOLTAGE (450)
#define PACK_MAX_VOLTAGE (540)
#define PACK_AH (36)
#define SCALED_PACK_AH (PACK_AH * 10000)
#define PACK_INVALID_VOLTAGE (-1)

// We don't need fast battery updates, every 30+ seconds should be fine
#define BATTERY_UPDATE_PERIOD_MS (30000)

namespace {
void update_battery_cb(void* ctx);

void update_battery_cb(void* ctx) {
    VESCState state = motor_driver_get_state(VESC_FRONT_ADDRESS);
    if (state.volts_in < PACK_SAFETY_VOLTAGE) {
        report_error("Battery requires charge");
    }
    if (state.volts_in < PACK_MIN_VOLTAGE) {
        report_fault("Battery SoC critical");
    }
    // AH used is in 1/10000 of an amp
    int32_t remaining_ah = (SCALED_PACK_AH - state.ah_used);
    int32_t percent = (100 * remaining_ah) / SCALED_PACK_AH;
    display_set_battery_charge(percent, state.volts_in);
}
}  // namespace

void battery_init(Scheduler* sched) {
    ENTER;

    if (NULL != sched) {
        sched->register_task(
            "battery", SCHED_MILLISECONDS(BATTERY_UPDATE_PERIOD_MS),
            update_battery_cb, TASK_FLAG_ENABLED, SCHED_MILLISECONDS(5000));
    }
    EXIT;
}
