/*
 *  Print last reset reason of ESP32
 *  =================================
 *
 *  Use either of the methods print_reset_reason
 *  or verbose_print_reset_reason to display the
 *  cause for the last reset of this device.
 *
 *  Public Domain License.
 *
 *  Author:
 *  Evandro Luis Copercini - 2017
 */
#include <trace.h>
#if defined(ARDUINO)
#ifdef ESP_IDF_VERSION_MAJOR  // IDF 4+
#if CONFIG_IDF_TARGET_ESP32   // ESP32/PICO-D4
#include "esp32/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32s2/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32C3
#include "esp32c3/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S3
#include "esp32s3/rom/rtc.h"
#else
#error Target CONFIG_IDF_TARGET is not supported
#endif
#else  // ESP32 Before IDF 4.0
#include "rom/rtc.h"
#endif

#define uS_TO_S_FACTOR \
    1000000 /* Conversion factor for micro seconds to seconds */

void print_reset_reason(int reason) {
    switch (reason) {
        case 1:
            TRACELN("POWERON_RESET");
            break; /**<1,  Vbat power on reset*/
        case 3:
            TRACELN("SW_RESET");
            break; /**<3,  Software reset digital core*/
        case 4:
            TRACELN("OWDT_RESET");
            break; /**<4,  Legacy watch dog reset digital core*/
        case 5:
            TRACELN("DEEPSLEEP_RESET");
            break; /**<5,  Deep Sleep reset digital core*/
        case 6:
            TRACELN("SDIO_RESET");
            break; /**<6,  Reset by SLC module, reset digital core*/
        case 7:
            TRACELN("TG0WDT_SYS_RESET");
            break; /**<7,  Timer Group0 Watch dog reset digital core*/
        case 8:
            TRACELN("TG1WDT_SYS_RESET");
            break; /**<8,  Timer Group1 Watch dog reset digital core*/
        case 9:
            TRACELN("RTCWDT_SYS_RESET");
            break; /**<9,  RTC Watch dog Reset digital core*/
        case 10:
            TRACELN("INTRUSION_RESET");
            break; /**<10, Instrusion tested to reset CPU*/
        case 11:
            TRACELN("TGWDT_CPU_RESET");
            break; /**<11, Time Group reset CPU*/
        case 12:
            TRACELN("SW_CPU_RESET");
            break; /**<12, Software reset CPU*/
        case 13:
            TRACELN("RTCWDT_CPU_RESET");
            break; /**<13, RTC Watch dog Reset CPU*/
        case 14:
            TRACELN("EXT_CPU_RESET");
            break; /**<14, for APP CPU, reseted by PRO CPU*/
        case 15:
            TRACELN("RTCWDT_BROWN_OUT_RESET");
            break; /**<15, Reset when the vdd voltage is not stable*/
        case 16:
            TRACELN("RTCWDT_RTC_RESET");
            break; /**<16, RTC Watch dog reset digital core and rtc module*/
        default:
            TRACELN("NO_MEAN");
    }
}

void verbose_print_reset_reason(int reason) {
    switch (reason) {
        case 1:
            TRACELN("Vbat power on reset");
            break;
        case 3:
            TRACELN("Software reset digital core");
            break;
        case 4:
            TRACELN("Legacy watch dog reset digital core");
            break;
        case 5:
            TRACELN("Deep Sleep reset digital core");
            break;
        case 6:
            TRACELN("Reset by SLC module, reset digital core");
            break;
        case 7:
            TRACELN("Timer Group0 Watch dog reset digital core");
            break;
        case 8:
            TRACELN("Timer Group1 Watch dog reset digital core");
            break;
        case 9:
            TRACELN("RTC Watch dog Reset digital core");
            break;
        case 10:
            TRACELN("Instrusion tested to reset CPU");
            break;
        case 11:
            TRACELN("Time Group reset CPU");
            break;
        case 12:
            TRACELN("Software reset CPU");
            break;
        case 13:
            TRACELN("RTC Watch dog Reset CPU");
            break;
        case 14:
            TRACELN("for APP CPU, reseted by PRO CPU");
            break;
        case 15:
            TRACELN("Reset when the vdd voltage is not stable");
            break;
        case 16:
            TRACELN("RTC Watch dog reset digital core and rtc module");
            break;
        default:
            TRACELN("NO_MEAN");
    }
}

void dump_reset_reason() {
    verbose_print_reset_reason(rtc_get_reset_reason(0));
    verbose_print_reset_reason(rtc_get_reset_reason(1));
}
#else
void dump_reset_reason() {}
#endif
