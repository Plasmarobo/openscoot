#pragma once

#include <stdint.h>

#include "scheduler.h"

#define KPH_RESOLUTION (10)
#define STATUS_LINE_MAX (128)

struct DisplayData {
    uint16_t kph;
    uint16_t limit;
    uint8_t charge;
    uint8_t radio_sig;
    uint8_t wifi_sig;
    uint8_t nfc_status;
    bool locked;
    char status_line[STATUS_LINE_MAX];
};

struct ColorData {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

void display_init(Scheduler* sched);
void set_backlight(bool on);
void update_display_cb(void);

// Fixed point kph
void display_set_kph(uint16_t kph);
void display_set_kph_limit(uint16_t limit);
void display_set_battery_charge(uint8_t percent_charge);
void display_set_radio_signal(uint8_t signal);
void display_set_wifi_signal(uint8_t signal);
void display_set_nfc_status(uint8_t status);
void display_set_locked(bool locked);
void display_set_status_line(const char* status);
void display_printf(const char* fmt, ...);
