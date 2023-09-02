#pragma once

#include <stdint.h>

#include "scheduler.h"

void battery_init(Scheduler* sched);
void set_battery_voltage(int16_t voltage);
