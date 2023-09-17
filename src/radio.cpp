#include "radio.h"

#if defined(ARDUINO)
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <framework.h>
#include <stdint.h>

#include <ringbuffer.hpp>

#include "errors.h"
#include "scheduler.h"
#include "trace.h"

#define RFM95_CS (33)
#define RFM95_INT (27)
#define RFM95_RST (13)
#define RF95_FREQ (915.0)
#define RF95_POWER (23)

#define INIT_DELAY_MS (10)
#define RADIO_UPDATE_PERIOD_MS (10)

#define SCOOTER_ADDRESS (0x10)
#define SERVER_ADDRESS (0x08)

#define RADIO_TIMEOUT_MS (2500)
#define RADIO_RETRIES (3)

#define RX_QUEUE_LEN (16)
#define TX_QUEUE_LEN (4)

namespace {
void delayed_init_cb(void* ctx);
void radio_update_cb(void* ctx);
enum radio_state { RADIO_UNINITIALIZED = 0, RADIO_READY, RADIO_ERROR };
radio_state driver_state;
RH_RF95 radio(RFM95_CS, RFM95_INT);
RHReliableDatagram client(radio, SCOOTER_ADDRESS);
RingBuffer<RadioMessageBuffer, RX_QUEUE_LEN> rx_ring;
RingBuffer<RadioMessageBuffer, TX_QUEUE_LEN> tx_ring;
RadioMessageBuffer tx_working_buffer;
RadioMessageBuffer rx_working_buffer;
uint8_t task_handle;
Scheduler* sched_ref;

void delayed_init_cb(void* ctx) {
    if (driver_state == RADIO_UNINITIALIZED) {
        digitalWrite(RFM95_RST, HIGH);
        call_deferred(sched_ref, SCHED_MILLISECONDS(INIT_DELAY_MS),
                      delayed_init_cb, __PRETTY_FUNCTION__);
    } else {
        if (!radio.init()) {
            report_error("Radio init failed");
            driver_state = RADIO_ERROR;
        }
        if (!radio.setFrequency(RF95_FREQ)) {
            report_error("Radio ferquency failed to set");
            driver_state = RADIO_ERROR;
            return;
        }
        radio.setTxPower(RF95_POWER, false);
        if (sched_ref != NULL) {
            sched_ref->start_task(task_handle);
        } else {
            report_error("Radio: global scheduler not available");
        }
    }
}

void radio_update_cb(void* ctx) {
    // Priority for RX
    if (radio.available()) {
        // Process RX packets
        rx_working_buffer.size = RH_RF95_MAX_MESSAGE_LEN;
        if (!radio.recv(rx_working_buffer.data, &rx_working_buffer.size)) {
            report_error("Radio RX error");
        }
        if (!rx_ring.push(&rx_working_buffer)) {
            report_error("Radio rx queue full, dropping message");
        }
    } else {
        if (!tx_ring.empty()) {
            if (!radio.isChannelActive()) {
                // Play nice with other transmissions
                if (tx_ring.pop(&tx_working_buffer)) {
                    // Note: may block if we're sending at too high a rate
                    radio.send(tx_working_buffer.data, tx_working_buffer.size);
                }
            }
        }
    }
}
}  // namespace

void radio_init(Scheduler* sched) {
    ENTER;
    sched_ref = sched;
    task_handle = 0;
    digitalWrite(RFM95_RST, LOW);
    driver_state = RADIO_UNINITIALIZED;
    if (NULL != sched) {
        task_handle = sched->register_task(
            "radio", SCHED_MILLISECONDS(RADIO_UPDATE_PERIOD_MS),
            radio_update_cb);
        call_deferred(sched_ref, SCHED_MILLISECONDS(INIT_DELAY_MS),
                      delayed_init_cb, __PRETTY_FUNCTION__);
    }
    EXIT;
}

bool radio_message_available() { return !rx_ring.empty(); }
bool radio_get_message(RadioMessageBuffer& msg) { return rx_ring.pop(&msg); }
bool radio_queue_send(RadioMessageBuffer& msg) { return tx_ring.push(&msg); }
#else
void radio_init(Scheduler* sched) {}
bool radio_message_available() { return false; }
bool radio_get_message(RadioMessageBuffer& msg) { return false; }
bool radio_queue_send(RadioMessageBuffer& msg) { return false; }
#endif
