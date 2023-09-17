#include "heartbeat.h"

#include <framework.h>

#include "scheduler.h"
#include "telemetry.h"
#include "trace.h"

#define HEARTBEAT_LED_PIN (13)

namespace {
void heartbeat_cb(void* ctx);
Scheduler* sched_ref;
uint8_t heartbeat_tick = 0;
bool once = false;

void set_heart(bool on) { digitalWrite(HEARTBEAT_LED_PIN, on ? HIGH : LOW); }

void heartbeat_cb(void* ctx) {
    switch (heartbeat_tick) {
        case 0:
            heartbeat_tick = 1;
            set_heart(false);
            if (!once) {
                call_deferred(sched_ref,
                              SCHED_MILLISECONDS(HEARTBEAT_PERIOD_MS),
                              heartbeat_cb, __PRETTY_FUNCTION__);
            }
            break;
        case 1:
            heartbeat_tick = 0;
            set_heart(true);
            telemetry_send_message("heartbeat", "VEHICLE0");
            call_deferred(sched_ref, SCHED_MILLISECONDS(HEARTBEAT_DURATION_MS),
                          heartbeat_cb, __PRETTY_FUNCTION__);
            break;
        default:
            heartbeat_tick = 0;
            break;
    }
}
}  // namespace

void heartbeat_init(Scheduler* sched) {
    ENTER;
    sched_ref = sched;
    pinMode(HEARTBEAT_LED_PIN, OUTPUT);
    digitalWrite(HEARTBEAT_LED_PIN, LOW);
    EXIT;
}

void heartbeat_start() {
    heartbeat_tick = 0;
    once = false;
    call_deferred(sched_ref, SCHED_MILLISECONDS(HEARTBEAT_PERIOD_MS),
                  heartbeat_cb, __PRETTY_FUNCTION__);
}

void heartbeat_once() {
    once = true;
    heartbeat_tick = 1;
    call_deferred(sched_ref, SCHED_IMMEDIATE, heartbeat_cb,
                  __PRETTY_FUNCTION__);
}
