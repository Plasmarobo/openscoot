#include "heartbeat.h"

#include <framework.h>

#include "scheduler.h"
#include "trace.h"

#define HEARTBEAT_LED_PIN (13)

namespace {
void heartbeat_cb(void* ctx);

uint8_t heartbeat_tick = 0;

void heartbeat_cb(void* ctx) {
    switch (heartbeat_tick) {
        case 0:
            heartbeat_tick = 1;
            digitalWrite(HEARTBEAT_LED_PIN, LOW);
            call_deferred(SCHED_MILLISECONDS(HEARTBEAT_PERIOD_MS),
                          heartbeat_cb);
            break;
        case 1:
            heartbeat_tick = 0;
            digitalWrite(HEARTBEAT_LED_PIN, HIGH);
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
    heartbeat_tick = 0;
    call_deferred(SCHED_MILLISECONDS(HEARTBEAT_PERIOD_MS), heartbeat_cb);
    EXIT;
}
