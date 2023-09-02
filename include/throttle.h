#pragma once

#include "scheduler.h"

struct ThrottleData {
    bool enabled;
    int16_t limit;
    int16_t current;
};

void throttle_init(Scheduler* sched);
void throttle_enable(bool enable);
void throttle_set_limit(int16_t pt);
