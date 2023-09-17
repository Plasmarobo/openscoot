#include "buttons.h"

#include <framework.h>
#include <stdint.h>

#include "scheduler.h"
#include "trace.h"

#define BUTTON_DEBOUNCE_MS (9)
#define TRIGGERED_INVALID (0)

namespace {
void button_update_cb(void* ctx);
void scan_keys_isr();

struct KeyState {
    uint8_t pin;
    ButtonHandler_t callback;
};
Scheduler* sched_ref;
Timespan_t triggered;
KeyState key_map[KEY_MAX] = {
    {A1, NULL},
    {A2, NULL},
};

void scan_keys_isr() {
    if (triggered == TRIGGERED_INVALID) {
        triggered = sched_ref->schedule_time();
        call_deferred(sched_ref, SCHED_MILLISECONDS(BUTTON_DEBOUNCE_MS),
                      button_update_cb, "keyisr");
    }
}

void button_update_cb(void* ctx) {
    triggered = TRIGGERED_INVALID;
    for (uint8_t i = 0; i < KEY_MAX; ++i) {
        uint8_t sample = digitalRead(key_map[i].pin);
        if (LOW == sample) {
            if (NULL != key_map[i].callback) {
                key_map[i].callback();
            }
        }
    }
}
}  // namespace

void buttons_init(Scheduler* sched) {
    ENTER;
    sched_ref = sched;
    triggered = TRIGGERED_INVALID;
    for (uint8_t i = KEY_A; i < KEY_MAX; ++i) {
        pinMode(key_map[i].pin, INPUT_PULLUP);
        attachInterrupt(key_map[i].pin, scan_keys_isr, FALLING);
    }
    EXIT;
}

bool button_is_pressed(Button_t btn) {
    if (btn < KEY_MAX) {
        return digitalRead(key_map[btn].pin) == LOW;
    }
    return false;
}

void buttons_set_callback(Button_t btn, ButtonHandler_t handler) {
    if (btn < KEY_MAX) {
        key_map[btn].callback = handler;
    }
}
