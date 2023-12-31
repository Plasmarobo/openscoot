#include <framework.h>

#define DEBUG
#define _DEBUG

#include <trace.h>

#include "battery.h"
#include "brake.h"
#include "buttons.h"
#include "config.h"
#include "display.h"
#include "gps.h"
#include "heartbeat.h"
#include "motor_driver.h"
#include "nfc.h"
#include "pwr.h"
#include "radio.h"
#include "reset_reason.h"
#include "scheduler.h"
#include "speed.h"
#include "telemetry.h"
#include "throttle.h"
#include "vehicle_status.h"
#include "wifi_manager.h"

void core0_init(void* params);
void core1_init(void* params);

namespace {
Scheduler core0_sched;
Scheduler core1_sched;
TaskHandle_t core0_task;
TaskHandle_t core1_task;
bool display_lock;

void perf_cb(Timespan_t ts, const char* name, const char* ev) {
    TRACEF("[%d - %s] %s\n", ts, name, ev);
}

void toggle_lockscreen() {
    display_lock = !display_lock;
    display_set_locked(display_lock);
    if (display_lock) {
        throttle_set_state(THROTTLE_STATE_LOCKED);
    } else {
        throttle_reset_state(THROTTLE_STATE_LOCKED);
    }
    if (!display_lock) {
        throttle_set_limit(CONFIG_THROTTLE_LIMIT);
    }
}

void perf_trace(Timespan_t timestamp, const char* name, const char* event) {
    TRACEF("[%d] %s %s\n", timestamp, name, event);
}

void dispatch_heartbeat() { heartbeat_once(); }
}  // namespace

void core0_init(void* params) {
    ENTER;
    motor_driver_init(&core0_sched);
    brake_init(&core0_sched);
    throttle_init(&core0_sched);
    throttle_set_state(THROTTLE_STATE_LOCKED);
    display_set_locked(display_lock);
    throttle_set_limit(CONFIG_THROTTLE_LIMIT);
    EXIT;
}

void core1_init(void* params) {
    ENTER;
    display_init(&core1_sched);
    battery_init(&core1_sched);
    heartbeat_init(&core1_sched);
    buttons_init(&core1_sched);
    buttons_set_callback(KEY_A, dispatch_heartbeat);
    buttons_set_callback(KEY_B, toggle_lockscreen);
    nfc_init(&core1_sched);
    gps_init(&core1_sched);
    speed_init(&core1_sched);
    wifi_init(&core1_sched);
    radio_init(&core1_sched);
    telemetry_init(&core1_sched);
    vehicle_status_init(&core1_sched);
    EXIT;
}

void core0_task_cb(void* ctx) {
    core0_init(NULL);
    for (;;) {
        core0_sched.run();
        taskYIELD();
    }
}

void core1_task_cb(void* ctx) {
    core1_init(NULL);
    for (;;) {
        core1_sched.run();
        taskYIELD();
    }
}

void basic_init() {
    ENTER;
    init_pwr();
    enable_pwr();
    display_lock = true;
    core0_sched.init();
    core0_sched.set_performance_callback(&perf_trace);
    core1_sched.init();
    core0_sched.start_scheduler();
    core1_sched.start_scheduler();
    xTaskCreatePinnedToCore(core0_task_cb, "core0t", 16384, NULL, 1,
                            &core0_task, 0);
    xTaskCreatePinnedToCore(core1_task_cb, "core1t", 16384, NULL, 1,
                            &core1_task, 1);
    EXIT;
}

#if defined(ARDUINO)

void setup() {
    dump_reset_reason();
    basic_init();
}

void loop() {
    // NOOP, will run in tasks
}
#else
int main(int argc, char** argv) {
    bool run = true;
    basic_init();
    while (run) {
        core0_sched.run();
        core1_sched.run();
        delay_us(1);
    }
    return 0;
}
#endif
