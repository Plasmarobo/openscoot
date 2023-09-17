#include "trace.h"

#include <string.h>
#if !defined(ARDUINO)
#include <iostream>
#endif

#include "framework.h"
#if defined(ENABLE_TRACE)
#define BUFFER_LENGTH (256)

bool _trace(const char* fmt, ...) {
    static char buffer[BUFFER_LENGTH];
    memset(buffer, '\0', BUFFER_LENGTH);
    va_list argptr;
    va_start(argptr, fmt);
    vsnprintf(buffer, BUFFER_LENGTH - 1, fmt, argptr);
    va_end(argptr);
    buffer[BUFFER_LENGTH - 1] = '\0';
#if defined(ARDUINO)
    Serial.print((const char*)(&buffer[0]));
    Serial.flush();
#else
    std::cout << (const char*)(&buffer[0]);
#endif
    return true;
}
#endif
