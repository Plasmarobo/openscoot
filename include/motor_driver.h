#pragma once

#include <stdint.h>

#include "scheduler.h"

#define VESC_FRONT_ADDRESS (47)
#define VESC_REAR_ADDRESS (79)

struct VESCState {
    int32_t erpm;
    int32_t drive_current;
    int32_t duty_cycle;
    int32_t ah_used;
    int32_t ah_charged;
#ifdef MD_PARSE_STATUS3
    int32_t wh_used;
    int32_t wh_charged;
#endif
    int32_t temp_fet;
    int32_t temp_mot;  // Unsupported in the N9-22
    int32_t current_in;
    int32_t pid_pos;
    int32_t tachometer;
    int32_t volts_in;
#ifdef MD_PARSE_STATUS6
    int32_t adc1;
    int32_t adc2;
    int32_t adc3;
    int32_t ppm;
#endif
};

void motor_driver_init(Scheduler* sched);
void motor_driver_set_rpm(uint8_t vesc, int32_t rpm);
void motor_driver_set_duty_cycle(uint8_t vesc, int32_t cycle);
void motor_driver_set_current(uint8_t vesc, int32_t current);
VESCState motor_driver_get_state(uint8_t vesc);
