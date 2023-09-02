#pragma once

#define LOGGER_INTERNET (1)
#define LOGGER_SDCARD (2)
#define LOGGER_SERIAL (3)
#define LOGGER_DISPLAY (4)

void logf(const char* fmt, ...);
void log(const char* fmt, ...);
