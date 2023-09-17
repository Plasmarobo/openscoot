
#include "telemetry.h"

#include <PubSubClient.h>
#include <WiFi.h>
#include <WifiClient.h>
#include <scheduler.h>

#include <ringbuffer.hpp>

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
    if (wifi_connected()) {
        bool connected = mqtt.connected();
        if (!connected) {
            if (!(connected = mqtt.connect("openscooter"))) {
                TeleMessage msg;
                while (!tx_queue.empty()) {
                    tx_queue.pop(&msg);
                    mqtt.publish(msg.topic, msg.data);
                }
            } else {
                report_error("Telemetry not connected");
            }
        }
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

bool telemetry_send_message(TeleMessage& msg) { tx_queue.push(&msg); }
