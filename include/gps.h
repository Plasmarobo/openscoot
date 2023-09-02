#pragma once

#include "scheduler.h"

typedef struct {
    double latitude;
    double longitude;
    double angle;
    double altitude;
    bool fix;
} GPSData;

void gps_init(Scheduler* sched);
GPSData get_gps_data();
