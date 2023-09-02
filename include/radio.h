#pragma once

#if defined(ARDUINO)
#include <RH_RF95.h>
#include <SPI.h>
#else
#define RH_RF95_MAX_MESSAGE_LEN (8)
#endif
#include <framework.h>
#include <stdint.h>

#include "scheduler.h"

typedef struct {
    uint8_t data[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t size;
} RadioMessageBuffer;

void radio_init(Scheduler* sched);
bool radio_message_available();
bool radio_get_message(RadioMessageBuffer& msg);
bool radio_queue_send(RadioMessageBuffer& msg);
