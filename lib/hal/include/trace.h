#ifndef TRACE_H
#define TRACE_H

// #define ENABLE_TRACE

#ifdef ENABLE_TRACE
#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif
bool _trace(const char* fmt, ...);
#define TRACE(str) _trace(str)
#define TRACEF(fmt, ...) _trace(fmt, __VA_ARGS__)
#define TRACELN(str) _trace("%s\n", str)
#define ENTER _trace("%s ENTER\n", __PRETTY_FUNCTION__)
#define EXIT _trace("%s EXIT\n", __PRETTY_FUNCTION__)
#else
#define TRACE(str) false
#define TRACEF(fmt, ...) false
#define TRACELN(str) false
#define ENTER false
#define EXIT false
#endif

#endif  // TRACE_H
