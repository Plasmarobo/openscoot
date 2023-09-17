#pragma once

#include <scheduler.h>
#include <stdint.h>

#define TELEMETRY_MAX_LENGTH (64)

struct TeleMessage {
    char* topic;
    char data[TELEMETRY_MAX_LENGTH + 1];
};

void telemetry_init(Scheduler* sched);
bool telemetry_send_message(TeleMessage& msg);
