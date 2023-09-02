#pragma once

#include <stdint.h>

#include <string>

#define NFC_MESSAGE_SIZE (40)
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

void nfc_init();
NFCMessageBuffer get_nfc_data();
void set_nfc_data(NFCMessageBuffer data);
