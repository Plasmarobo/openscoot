#include "motor_driver.h"

#if defined(ARDUINO)
#include <Adafruit_MCP2515.h>

#include <ringbuffer.hpp>
#include <vector>

#include "errors.h"
#include "scheduler.h"
#include "trace.h"

#define CAN_UPDATE_MS (5)

#define CS_PIN (5)
#define INT_PIN (6)
#define CAN_BAUD (1000000)
#define CAN_DATA_SIZE (8)
// Minimum is 2
#define RX_BUFFER_SIZE (32)
// Minimum is 2
#define TX_BUFFER_SIZE (8)

#define VESC_COUNT (2)
#define VESC_FRONT_INDEX (0)
#define VESC_REAR_INDEX (1)

#define CMD_SET_DUTY (0)
#define CMD_SET_CURRENT (1)
#define CMD_SET_CURRENT_BRAKE (2)
#define CMD_SET_RPM (3)
#define CMD_SET_POS (4)
#define CMD_SET_CURRENT_REL (10)
#define CMD_SET_CURRENT_BRAKE_REL (11)
#define CMD_SET_CURRENT_HANDBRAKE (12)
#define CMD_SET_CURRENT_HANDBRAKE_REL (13)

#define STATUS1_ID (9)
#define STATUS2_ID (14)
#define STATUS3_ID (15)
#define STATUS4_ID (16)
#define STATUS5_ID (27)
#define STATUS6_ID (28)

#define TO_LITTLE_ENDIAN(x) (reverse_bytes((uint8_t*)&x, sizeof(x)))

namespace {
struct CANMessage {
    uint8_t cmd_id;
    uint8_t vesc_id;
    uint16_t length;
    uint8_t data[CAN_DATA_SIZE];
};
struct STATUS1 {
    int32_t erpm;
    int16_t current;
    int16_t duty_cycle;
};
struct STATUS2 {
    int32_t amp_hours_used;
    int32_t amp_hours_charged;
};
struct STATUS3 {
    int32_t watt_hours;
    int32_t watt_hours_charged;
};
struct STATUS4 {
    int16_t temp_fet;
    int16_t temp_mot;
    int16_t current_in;
    int16_t pid_pos;
};
struct STATUS5 {
    int32_t tachometer;
    int16_t volts_in;
};
struct STATUS6 {
    int16_t adc1;
    int16_t adc2;
    int16_t adc3;
    int16_t ppm;
};

void update_can_cb(void* ctx);
void parse_can_message(CANMessage* msg);

Adafruit_MCP2515 mcp(CS_PIN);
RingBuffer<CANMessage, RX_BUFFER_SIZE> rx_buffer;
uint16_t incoming_packet_length;
uint16_t requested_length;
RingBuffer<CANMessage, TX_BUFFER_SIZE> tx_buffer;
uint32_t can_flags;
uint8_t task_handle;
VESCState motor_state[VESC_COUNT];

void reverse_bytes(uint8_t* bytes, size_t size) {
    uint8_t carry;
    if (size > 1) {
        // Reverse byte order
        for (uint8_t idx = 0; idx < size; ++idx) {
            uint8_t rev_idx = (size - idx) - 1;
            carry = bytes[idx];
            bytes[idx] = bytes[rev_idx];
            bytes[rev_idx] = carry;
        }
    }
}

void can_tx(int id, uint8_t* buffer, size_t length) {
    // Up to 8 bytes
    mcp.beginExtendedPacket(id);
    mcp.write(buffer, length);
    mcp.endPacket();
}

void handle_can_rx_cb(int packet_size) {
    if (mcp.packetRtr()) {
        requested_length = mcp.packetDlc();
    } else {
        if (rx_buffer.full()) {
            // Buffer is full
            report_error("CAN RX OVRFLW");
        } else {
            CANMessage msg;
            long id = mcp.packetId();
            msg.cmd_id = (id >> 8) & 0xFF;
            msg.vesc_id = id & 0xFF;
            msg.length = incoming_packet_length;
            if (msg.length > 0) {
                mcp.readBytes(msg.data, msg.length);
            }
            rx_buffer.push(&msg);
        }
    }
}

void update_can_cb(void* ctx) {
    CANMessage msg;
    if (rx_buffer.pop(&msg)) {
        parse_can_message(&msg);
    }
    if (!tx_buffer.empty()) {
        tx_buffer.pop(&msg);
        long id = (long)(msg.vesc_id | (msg.cmd_id << 8));
        TRACEF("Sending on CAN %02x %02x %04x %08x \n", msg.vesc_id, msg.cmd_id,
               msg.length, ((uint64_t*)(msg.data)));
        can_tx(id, msg.data, msg.length);
    }
}

bool push_message(CANMessage* source) {
    if (source != NULL) {
        if (tx_buffer.full()) {
            report_error("CAN TX OVRFLW\n");
        } else {
            return tx_buffer.push(source);
        }
    }
    return false;
}

void parse_can_message(CANMessage* msg) {
    VESCState* state = NULL;
    if (msg->vesc_id == VESC_FRONT_ADDRESS) {
        state = &motor_state[VESC_FRONT_INDEX];
    } else if (msg->vesc_id == VESC_REAR_ADDRESS) {
        state = &motor_state[VESC_REAR_INDEX];
    } else {
        return;
    }
    switch (msg->cmd_id) {
        case STATUS1_ID: {
            STATUS1* data = (STATUS1*)(&msg->data[0]);
            TO_LITTLE_ENDIAN(data->erpm);
            state->erpm = data->erpm;  // 1 = 1rpm
            TO_LITTLE_ENDIAN(data->current);
            state->drive_current = ((int32_t)data->current);  // 10 = 1A
            TO_LITTLE_ENDIAN(data->duty_cycle);
            state->duty_cycle = ((int32_t)data->duty_cycle);  // 1000 = 0.001%
        } break;
        case STATUS2_ID: {
            STATUS2* data = (STATUS2*)(&msg->data[0]);
            TO_LITTLE_ENDIAN(data->amp_hours_used);
            state->ah_used = ((int32_t)data->amp_hours_used);  // 10 000 = 1Ah
            TO_LITTLE_ENDIAN(data->amp_hours_charged);
            state->ah_charged =
                ((int32_t)data->amp_hours_charged);  // 10 000 = 1Ah
        } break;
#ifdef MD_PARSE_STATUS3
        case STATUS3_ID: {
            STATUS3* data = (STATUS3*)(&msg->data[0]);
            TO_LITTLE_ENDIAN(data->watt_hours);
            int32_t wh_used =
                ((int32_t)data->watt_hours) * 10000;  // 10 000 = 1Wh
            TO_LITTLE_ENDIAN(data->watt_hours_charged);
            int32_t wh_charged =
                ((int32_t)data->watt_hours_charged) * 10000;  // 10 000 = 1Wh
        } break;
#endif
        case STATUS4_ID: {
            STATUS4* data = (STATUS4*)(&msg->data[0]);
            TO_LITTLE_ENDIAN(data->temp_fet);
            state->temp_fet = ((int32_t)data->temp_fet);  // 10 = 1C
            TO_LITTLE_ENDIAN(data->temp_mot);
            state->temp_mot =
                ((int32_t)data->temp_mot);  // 10 = 1C DISCONNECTED
            TO_LITTLE_ENDIAN(data->current_in);
            state->current_in = ((int32_t)data->current_in);  // 10 = 1A
            TO_LITTLE_ENDIAN(data->pid_pos);
            state->pid_pos = ((int32_t)data->pid_pos);  // 50 = 1deg
        } break;
        case STATUS5_ID: {
            STATUS5* data = (STATUS5*)(&msg->data[0]);
            TO_LITTLE_ENDIAN(data->tachometer);
            state->tachometer = ((int32_t)data->tachometer);  // 6 = 1EREV
            TO_LITTLE_ENDIAN(data->volts_in);
            state->volts_in = ((int32_t)data->volts_in);  // 10 = 1V
        } break;
#ifdef MD_PARSE_STATUS6
        case STATUS6_ID: {
            STATUS6* data = (STATUS6*)(&msg->data[0]);
            TO_LITTLE_ENDIAN(data->adc1);
            state->adc1 = ((int32_t)data->adc1);  // 1000 = 1V
            TO_LITTLE_ENDIAN(data->adc2);
            state->adc2 = ((int32_t)data->adc2);  // 1000 = 1V
            TO_LITTLE_ENDIAN(data->adc3);
            state->adc3 = ((int32_t)data->adc3);  // 1000 = 1V
            TO_LITTLE_ENDIAN(data->ppm);
            state->ppm = ((int32_t)data->ppm);
        } break;
#endif
        default:
            break;
    }
}

CANMessage build_can_message(uint8_t vesc, uint8_t command, uint8_t* value,
                             size_t value_length) {
    // Pack big-endian
    CANMessage msg;
    msg.vesc_id = vesc;
    msg.cmd_id = command;
    msg.length = value_length < 8 ? value_length : 8;
    for (uint8_t idx = 0; idx < msg.length; ++idx) {
        msg.data[idx] = value[(value_length - 1) - idx];
    }
    return msg;
}
}  // namespace

void motor_driver_init(Scheduler* sched) {
    if (!mcp.begin(CAN_BAUD)) {
        report_error("CAN init failure");
    }

    mcp.onReceive(INT_PIN, handle_can_rx_cb);
    if (NULL != sched) {
        task_handle = sched->register_task(SCHED_MILLISECONDS(CAN_UPDATE_MS),
                                           update_can_cb, TASK_FLAG_ENABLED);
    }
}

void motor_driver_set_rpm(uint8_t vesc, int32_t rpm) {
    CANMessage msg =
        build_can_message(vesc, CMD_SET_RPM, (uint8_t*)&rpm, sizeof(int32_t));
    push_message(&msg);
}

// From -100_000 to 100_000
void motor_driver_set_duty_cycle(uint8_t vesc, int32_t cycle) {
    CANMessage msg = build_can_message(vesc, CMD_SET_DUTY, (uint8_t*)&cycle,
                                       sizeof(int32_t));
    push_message(&msg);
}

// in units of mA
void motor_driver_set_current_ma(uint8_t vesc, int32_t current) {
    CANMessage msg = build_can_message(vesc, CMD_SET_CURRENT,
                                       (uint8_t*)&current, sizeof(int32_t));
    push_message(&msg);
}

VESCState motor_driver_get_state(uint8_t vesc) {
    VESCState state = {0};
    if (vesc == VESC_FRONT_ADDRESS) {
        state = motor_state[VESC_FRONT_INDEX];
    } else if (vesc == VESC_REAR_ADDRESS) {
        state = motor_state[VESC_REAR_INDEX];
    }
    return state;
}
#else

void motor_driver_init(Scheduler* sched) {}
void motor_driver_set_rpm(uint8_t vesc, int32_t rpm) {}
void motor_driver_set_duty_cycle(uint8_t vesc, int32_t cycle) {}
void motor_driver_set_current(uint8_t vesc, int32_t current) {}
VESCState motor_driver_get_state(uint8_t vesc) { return {}; }
#endif
