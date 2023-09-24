#include "vehicle_status.h"

#include "gps.h"
#include "motor_driver.h"
#include "telemetry.h"

#define STATUS_PERIOD_MS (5000)

namespace {
Scheduler* sched_ref;
void vehicle_status_cb(void* ctx) {
    VESCState state = motor_driver_get_state(VESC_FRONT_ADDRESS);
    telemetry_send_message("status/battery", "{'V': %d.%d, 'A': %d}",
                           state.volts_in / 10, state.volts_in % 10,
                           state.ah_used);
    telemetry_send_message("status/motor", "{'RPM':%d,'A':%d}", state.erpm,
                           state.drive_current);
    telemetry_send_message("status/tach", "{'T':%d}", state.tachometer);
    GPSData gps = get_gps_data();
    if (!gps.fix) {
        telemetry_send_message("status/location", "{'fix': false}");
    } else {
        telemetry_send_message(
            "status/location",
            "{'fix': true,'lat': %d,'long':%d,'angle':%d,'alt':%d}",
            gps.latitude, gps.longitude, gps.angle, gps.altitude);
    }
}
}  // namespace

void vehicle_status_init(Scheduler* sched) {
    sched_ref = sched;
    if (NULL != sched) {
        sched->register_task("vstatus", SCHED_MILLISECONDS(STATUS_PERIOD_MS),
                             vehicle_status_cb, TASK_FLAG_ENABLED,
                             SCHED_MILLISECONDS(STATUS_PERIOD_MS));
    }
}
