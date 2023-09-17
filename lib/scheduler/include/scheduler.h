#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <stddef.h>
#include <stdint.h>

#define MAX_TASKS (32)
#define MAX_GLOBAL_DEFERS (16)

#define TASK_INVALID (0)

#define TASK_FLAG_ENABLED (0x1)
#define TASK_FLAG_ONCE (0x2)
#define TASK_FLAG_DELAYED (0x4)
#define SCHED_MILLISECONDS(x) (1000 * x)
#define SCHED_MICROSECONDS(x) (x)
#define SCHED_IMMEDIATE (1)

static_assert(MAX_TASKS < 255, "Max tasks must be less than 255");

typedef void (*TaskHandler_t)(void* context);
typedef int64_t Timespan_t;

typedef void (*PerformanceHook_t)(Timespan_t timestamp, const char* name,
                                  const char* event);

class DeferredTask {
   public:
    const char* name;
    Timespan_t delay;
    TaskHandler_t callback;
    TaskHandler_t onfree;
    Timespan_t elapsed;
    DeferredTask* next;

    DeferredTask();
    DeferredTask(Timespan_t start_delay, TaskHandler_t handler);
    DeferredTask(const DeferredTask& rhs);
};

class Scheduler {
   private:
    struct Task {
        const char* name;
        Timespan_t period;
        TaskHandler_t callback;
        uint32_t flags;
        Timespan_t elapsed;
        void* context;
    };
    uint32_t next_id;
    Timespan_t last_tick;
    Task task_vector[MAX_TASKS];
    DeferredTask* root;
    PerformanceHook_t performance_callback;

    uint8_t id_to_index(uint8_t id);

   public:
    Scheduler();

    uint8_t register_task(const char* name, Timespan_t period,
                          TaskHandler_t handler, uint32_t flags = 0,
                          Timespan_t delay = 0);

    bool defer_call(DeferredTask* task);
    bool defer_priority(DeferredTask* task);
    void init();
    void start_scheduler();
    void run();
    void start_task(uint8_t id, Timespan_t delay = 0);
    void stop_task(uint8_t id);
    void delay_task(uint8_t id, Timespan_t delay);
    void stop_all();
    void resume_all();
    Timespan_t schedule_time();
    Timespan_t milliseconds_since(Timespan_t t);
    Timespan_t microseconds_since(Timespan_t t);
    void dump_tasks();
    void set_performance_callback(PerformanceHook_t cb);
};

// Uses global scheduler for convenient oneshot tasking, fails if queue full
bool call_deferred(Scheduler* sched, Timespan_t delay, TaskHandler_t callback,
                   const char* name = NULL);

#endif  // _RIOS_H_
