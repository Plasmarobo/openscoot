#include <framework.h>

#define DEBUG
#define _DEBUG

#include <trace.h>

#include "battery.h"
#include "buttons.h"
#include "display.h"
#include "heartbeat.h"
#include "motor_driver.h"
#include "pwr.h"
#include "reset_reason.h"
#include "scheduler.h"
#include "throttle.h"

void post_boot_init(void* ctx);

namespace {
Scheduler* gs;

void toggle_lockscreen() {
    ENTER;
    static bool locked = true;
    locked = !locked;
    display_set_locked(locked);
    EXIT;
}
}  // namespace

void init() {
    gs = get_global_scheduler();
    gs->init();
    gs->start_scheduler();
    init_pwr();
    enable_pwr();
    heartbeat_init();
    buttons_init();
    buttons_set_callback(KEY_B, toggle_lockscreen);
    call_deferred(SCHED_MILLISECONDS(PWR_STARTUP_DELAY_MS), post_boot_init);
}

#if defined(ARDUINO)
void setup() {
    Serial.begin(115200);
    delay(1000);
    dump_reset_reason();
    Serial.println("- Boot");
    init();
}

void loop() { gs->run(); }
#else
int main(int argc, char** argv) {
    bool run = true;
    init();
    while (run) {
        gs->run();
        delay_us(1);
    }
    return 0;
}
#endif

void post_boot_init(void* ctx) {
    static int count = 0;
    ENTER;
    display_init(gs);
    motor_driver_init(gs);
    throttle_init(gs);
    throttle_set_limit(255);
    battery_init(gs);
    EXIT;
}
