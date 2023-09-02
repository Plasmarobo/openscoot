#include "errors.h"

#include <framework.h>
#include <stdint.h>

#include "display.h"
#include "trace.h"

namespace {
uint32_t error_flags;
uint32_t fault_flags;
}  // namespace

// Reports error, may allow limited function
void report_error(const char* message) {
    // TODO: write to SRAM
    TRACEF("Error: %s", message);
    display_printf("Err: %s", message);
}
// Reports error, and disables system until external intervention
void report_fault(const char* message) {
    TRACEF("%s\n", message);
    display_printf("Err: %s", message);
    // TODO: shutdown safely
}

bool active_fault() { return fault_flags != 0; }
