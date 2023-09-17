#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <scheduler.h>

#include <ringbuffer.hpp>

#include "display.h"

#define SSID ("HomeAwayFromHome2.4")
#define PSK ("internetplease")
#define WIFI_TASK_PERIOD_MS (15000)
#define WIFI_MESSAGE_QUEUE_LENGTH (8)

namespace {
Timespan_t wifi_scan_timer;
int status;
Scheduler* sched_ref;
WiFiMulti wifim;
};  // namespace

void wifi_cb(void* ctx) {
    if (WL_CONNECTED != status) {
        status = wifim.run();
        wifi_scan_timer = sched_ref->schedule_time();
    }
}

void wifi_init(Scheduler* sched) {
    wifim.addAP(SSID, PSK);
    status = wifim.run();
    if (sched) {
        sched_ref = sched;
        sched->register_task("wifi", WIFI_TASK_PERIOD_MS, wifi_cb,
                             TASK_FLAG_ENABLED);
        wifi_scan_timer = sched_ref->schedule_time();
    }
}

bool wifi_connected() { return status == WL_CONNECTED; }
