#pragma once

#include <stdint.h>

typedef void (*ButtonHandler_t)(void);

typedef enum {
    KEY_A = 0,
    KEY_B,
    KEY_MAX,
} Button_t;

void buttons_init();
uint8_t get_button_state();
bool button_is_pressed(Button_t btn);
void buttons_set_callback(Button_t btn, ButtonHandler_t handler);
