#pragma once

#include "scheduler.h"

#define HEARTBEAT_PERIOD_MS (900)
#define HEARTBEAT_DURATION_MS (100)

void heartbeat_init(Scheduler* sched);
void heartbeat_start();
void heartbeat_once();
