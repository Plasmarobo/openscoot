#pragma once

#include "scheduler.h"

struct BrakeData {
    int16_t value;
};

void brake_init(Scheduler* sched);
