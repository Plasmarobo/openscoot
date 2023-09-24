#pragma once

#if defined(ARDUINO)
#include <RH_RF95.h>
#include <SPI.h>
#define RADIO_MESSAGE_MAX_LEN (RH_RF95_MAX_MESSAGE_LEN)
#else
#define RADIO_MESSAGE_MAX_LEN (250)
#endif
#include <framework.h>
#include <stdint.h>

#include "scheduler.h"

typedef struct {
    uint8_t data[RADIO_MESSAGE_MAX_LEN];
    uint8_t size;
} RadioMessageBuffer;

void radio_init(Scheduler* sched);
bool radio_message_available();
bool radio_get_message(RadioMessageBuffer& msg);
bool radio_queue_send(RadioMessageBuffer& msg);
