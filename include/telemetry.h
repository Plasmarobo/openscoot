#pragma once

#include <scheduler.h>
#include <stdint.h>

#define TELEMETRY_MAX_LENGTH (128)

struct TeleMessage {
    const char* topic;
    char data[TELEMETRY_MAX_LENGTH + 1];
};

void telemetry_init(Scheduler* sched);
bool telemetry_send_message(TeleMessage& msg);
bool telemetry_send_message(const char* topic, const char* fmt, ...);
bool telemetry_is_connected();
