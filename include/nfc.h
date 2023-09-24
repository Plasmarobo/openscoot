#pragma once

#include <stdint.h>

#include <string>

#include "scheduler.h"

#define NFC_MESSAGE_SIZE (40)
#define B64_MESSAGE_SIZE (4 * NFC_MESSAGE_SIZE / 3)
#define NFC_DATA_SIZE (NFC_MESSAGE_SIZE - sizeof(NFCMessageHeader))

typedef struct __attribute__((packed)) {
    uint32_t message_counter;
    uint32_t topic;
    uint32_t crc;
} NFCMessageHeader;
// 256 bytes of fast read/write
typedef struct __attribute__((packed)) {
    NFCMessageHeader header;
    char data[NFC_DATA_SIZE];
} NFCMessageBuffer;

void nfc_init(Scheduler* sched);
NFCMessageBuffer get_nfc_data();
void set_nfc_data(NFCMessageBuffer data);
