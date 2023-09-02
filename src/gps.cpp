#include "gps.h"

#if defined(ARDUINO)
#include <Adafruit_GPS.h>

#include "scheduler.h"
#include "trace.h"

#define GPS_POLLRATE_MS (1000)
#define GPSSerial (Serial1)
namespace {
Adafruit_GPS GPS(&GPSSerial);
GPSData data_buffer;
uint8_t task_handle;
};  // namespace

#define GPSECHO (false)

void gps_task_cb(void* ctx);

void gps_init(Scheduler* sched) {
    task_handle = 0;
    GPS.begin(9600);
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    if (NULL != sched) {
        task_handle = sched->register_task(SCHED_MILLISECONDS(GPS_POLLRATE_MS),
                                           gps_task_cb);
        sched->start_task(task_handle);
    }
}

GPSData get_gps_data() { return data_buffer; }

void gps_task_cb(void* ctx) {
    ENTER;
    char c = GPS.read();
    if (GPS.newNMEAreceived()) {
        if (!GPS.parse(GPS.lastNMEA())) {
            return;
        }
        data_buffer.fix = GPS.fix;
        if (GPS.fix) {
            data_buffer.latitude = GPS.latitude;
            data_buffer.longitude = GPS.longitude;
            data_buffer.angle = GPS.angle;
            data_buffer.altitude = GPS.altitude;
        }
    }
    EXIT;
}
#else
void gps_init(Scheduler* sched) {}
GPSData get_gps_data() { return {}; }
#endif
