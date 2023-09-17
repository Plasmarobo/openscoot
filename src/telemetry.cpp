
#include "telemetry.h"

#include <PubSubClient.h>
#include <WiFi.h>
#include <WifiClient.h>
#include <scheduler.h>

#include <ringbuffer.hpp>

#include "display.h"
#include "errors.h"
#include "wifi_manager.h"

#define TX_QUEUE_LENGTH (8)
#define TELEMETRY_TASK_PERIOD_MS (200)

namespace {
Scheduler* sched_ref;
RingBuffer<TeleMessage, TX_QUEUE_LENGTH> tx_queue;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
}  // namespace

void telemetry_cb(void* ctx) {
    bool connected = mqtt.connected();
    if (connected || mqtt.connect("opnsct")) {
        TeleMessage msg;
        while (!tx_queue.empty()) {
            tx_queue.pop(&msg);
            if (!mqtt.publish(msg.topic, msg.data)) {
                report_error("Telem failure");
            }
        }
    } else {
        report_error("Telem disconnect");
    }
}

void telemetry_init(Scheduler* sched) {
    mqtt.setServer(IPAddress(192, 168, 1, 185), 1883);
    if (sched) {
        sched_ref = sched;
        sched->register_task("telemetry", TELEMETRY_TASK_PERIOD_MS,
                             telemetry_cb, TASK_FLAG_ENABLED);
    }
}

bool telemetry_send_message(TeleMessage& msg) { return tx_queue.push(&msg); }
bool telemetry_send_message(const char* topic, const char* fmt, ...) {
    TeleMessage msg;
    msg.topic = topic;
    va_list argptr;
    va_start(argptr, fmt);
    vsnprintf(msg.data, TELEMETRY_MAX_LENGTH, fmt, argptr);
    va_end(argptr);
    msg.data[TELEMETRY_MAX_LENGTH] = '\0';
    return telemetry_send_message(msg);
}
