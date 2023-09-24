#pragma once

#include "scheduler.h"

#define THROTTLE_STATE_UNLOCKED (0x00)
#define THROTTLE_STATE_LOCKED (0x01)
#define THROTTLE_STATE_BRAKE (0x02)

struct ThrottleData {
    uint8_t state;
    int16_t limit;
    int16_t current;
};

void throttle_init(Scheduler* sched);
void throttle_set_state(uint8_t state);
void throttle_reset_state(uint8_t state);
void throttle_set_limit(int16_t pt);
