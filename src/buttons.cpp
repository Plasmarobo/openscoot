#include "buttons.h"

#include <framework.h>
#include <stdint.h>

#include "scheduler.h"
#include "trace.h"

#define BUTTON_SCAN_PERIOD_MS (25)
#define BUTTON_DEBOUNCE_MS (25)

namespace {
void button_update_cb(void* ctx);
void scan_keys_isr();
uint8_t buttons_state;
volatile bool pending_call;
ButtonHandler_t callbacks[KEY_MAX] = {NULL};
const uint8_t key_pin_map[KEY_MAX] = {
    A1,
    A2,
};

void scan_keys_isr() {
    if (!pending_call) {
        pending_call = true;
        call_deferred(SCHED_MILLISECONDS(BUTTON_DEBOUNCE_MS), button_update_cb);
    }
}

void button_update_cb(void* ctx) {
    ENTER;
    for (uint8_t i = 0; i < KEY_MAX; ++i) {
        uint8_t sample = digitalRead(key_pin_map[i]);
        uint8_t idx = (1 << i);
        if (sample == LOW) {
            // Low is pressed, check if JUST pressed
            if (!(buttons_state & idx)) {
                // Flag is not set, trigger and set flag
                if (NULL != callbacks[i]) {
                    callbacks[i]();
                }
                buttons_state |= idx;
            }
        } else {
            // Clear the button state
            buttons_state &= ~idx;
        }
    }
    pending_call = false;
    EXIT;
}
}  // namespace

void buttons_init() {
    pending_call = false;
    for (uint8_t i = KEY_A; i < KEY_MAX; ++i) {
        pinMode(key_pin_map[i], INPUT_PULLUP);
        attachInterrupt(key_pin_map[i], scan_keys_isr, FALLING);
    }
    for (uint8_t i = 0; i < KEY_MAX; ++i) {
        callbacks[i] = NULL;
    }
}

uint8_t get_button_state() { return buttons_state; }

bool button_is_pressed(Button_t btn) {
    if (btn < KEY_MAX) {
        return buttons_state & (1 << btn);
    }
    return false;
}

void buttons_set_callback(Button_t btn, ButtonHandler_t handler) {
    if (btn < KEY_MAX) {
        callbacks[btn] = handler;
    }
}
