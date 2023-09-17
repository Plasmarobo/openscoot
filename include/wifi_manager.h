#pragma once

#include <scheduler.h>

// Initializes wifi task
void wifi_init(Scheduler* sched);
bool wifi_connected();
