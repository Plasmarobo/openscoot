#include "trace.h"

#include <string.h>

#include "framework.h"

#define BUFFER_LENGTH (256)

bool _trace(const char* fmt, ...) {
    char buffer[BUFFER_LENGTH];
    memset(buffer, '\0', 256);
    va_list argptr;
    va_start(argptr, fmt);
    vsnprintf(buffer, BUFFER_LENGTH, fmt, argptr);
    va_end(argptr);
#if defined(ARDUINO)
    Serial.print(buffer);
    Serial.flush();
#else
    std::cout << buffer;
#endif

    return true;
}
