#pragma once

// Reports error, may allow limited function
void report_error(const char* message);
// Reports error, and disables system until external intervention
void report_fault(const char* message);

bool active_fault();
