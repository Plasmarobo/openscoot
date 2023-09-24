
#include "telemetry.h"

#include <PubSubClient.h>
#include <WiFi.h>
#include <WifiClient.h>
#include <scheduler.h>

#include <ringbuffer.hpp>

#include "display.h"
#include "errors.h"
#include "radio.h"
#include "wifi_manager.h"

#define TX_QUEUE_LENGTH (32)
#define TELEMETRY_TASK_PERIOD_MS (5000)

namespace {
const uint32_t TELEM_SERIAL = 0x01;
const uint32_t TELEM_WIFI = 0x02;
const uint32_t TELEM_RADIO = 0x04;
const uint32_t TELEM_CELL = 0x08;
uint32_t telem_flags;
Scheduler* sched_ref;
RingBuffer<TeleMessage, TX_QUEUE_LENGTH> tx_queue;
WiFiClient wifiClient;

PubSubClient mqtt(wifiClient);
bool telemetry_connected;
}  // namespace

void telemetry_cb(void* ctx) {
    TeleMessage msg;
    while (!tx_queue.empty()) {
        tx_queue.pop(&msg);
        // Wifi/Cell etc
        if (wifi_connected()) {
            bool connected = mqtt.connected();
            if (connected || mqtt.connect("opnsct")) {
                telemetry_connected = true;
                if (!mqtt.publish(msg.topic, msg.data)) {
                    report_error("MQTT pub failure");
                }
            } else {
                telemetry_connected = false;
                report_error("MQTT disconnect");
            }
        }
        // Radio
        RadioMessageBuffer rmsg;
        rmsg.size = strlen(msg.data);
        if (rmsg.size > RADIO_MESSAGE_MAX_LEN) {
            rmsg.size = RADIO_MESSAGE_MAX_LEN;
        }
        memcpy(rmsg.data, msg.data, rmsg.size);
        radio_queue_send(rmsg);
        // Serial
    }
}

void telemetry_init(Scheduler* sched) {
    mqtt.setServer(IPAddress(192, 168, 1, 185), 1883);
    if (sched) {
        sched_ref = sched;
        sched->register_task("telemetry", TELEMETRY_TASK_PERIOD_MS,
                             telemetry_cb, TASK_FLAG_ENABLED);
    }
    telemetry_connected = false;
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

bool telemetry_is_connected() { return telemetry_connected; }
