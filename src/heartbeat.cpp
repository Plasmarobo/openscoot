#include "heartbeat.h"

#include <framework.h>

#include "scheduler.h"
#include "trace.h"

#define HEARTBEAT_LED_PIN (13)

namespace {
void heartbeat_cb(void* ctx);

uint8_t heartbeat_tick = 0;
bool once = false;

void heartbeat_cb(void* ctx) {
    switch (heartbeat_tick) {
        case 0:
            heartbeat_tick = 1;
            set_heart(false);
            if (!once) {
                call_deferred(SCHED_MILLISECONDS(HEARTBEAT_PERIOD_MS),
                              heartbeat_cb);
            }
            break;
        case 1:
            heartbeat_tick = 0;
            set_heart(true);
            call_deferred(SCHED_MILLISECONDS(HEARTBEAT_DURATION_MS),
                          heartbeat_cb);
            break;
        default:
            heartbeat_tick = 0;
            break;
    }
}
}  // namespace

void heartbeat_init() {
    ENTER;
    pinMode(HEARTBEAT_LED_PIN, OUTPUT);
    digitalWrite(HEARTBEAT_LED_PIN, LOW);
    EXIT;
}

void heartbeat_start() {
    heartbeat_tick = 0;
    once = false;
    call_deferred(SCHED_MILLISECONDS(HEARTBEAT_PERIOD_MS), heartbeat_cb);
}

void heartbeat_once() {
    once = true;
    heartbeat_tick = 1;
    call_deferred(SCHED_IMMEDIATE, heartbeat_cb);
}

void set_heart(bool on) { digitalWrite(HEARTBEAT_LED_PIN, on ? HIGH : LOW); }
