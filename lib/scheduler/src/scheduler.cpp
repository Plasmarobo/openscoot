#include "scheduler.h"

#include <framework.h>

#include <mempool.hpp>

#define PERF(name, event) \
    if (performance_callback) performance_callback(schedule_time(), name, event)

namespace {
MemPool<DeferredTask, MAX_GLOBAL_DEFERS> global_defer_queue;
}  // namespace

DeferredTask::DeferredTask() {
    delay = 0;
    callback = NULL;
    elapsed = 0;
    next = NULL;
    onfree = NULL;
}

DeferredTask::DeferredTask(Timespan_t start_delay, TaskHandler_t handler) {
    delay = start_delay;
    callback = handler;
    elapsed = 0;
    next = NULL;
    onfree = NULL;
}

DeferredTask::DeferredTask(const DeferredTask& rhs) {
    delay = rhs.delay;
    callback = rhs.callback;
    elapsed = rhs.elapsed;
    next = rhs.next;
    onfree = rhs.onfree;
}

Scheduler::Scheduler() { init(); }

void Scheduler::init() {
    performance_callback = NULL;
    next_id = 0;
    for (auto& task : task_vector) {
        task = {0};
    }
    last_tick = schedule_time();
    root = NULL;
}

uint8_t Scheduler::id_to_index(uint8_t id) {
    if (id == TASK_INVALID) {
        return MAX_TASKS;
    }
    id = id - 1;
    return id;
}

uint8_t Scheduler::register_task(const char* name, Timespan_t period,
                                 TaskHandler_t handler, uint32_t flags,
                                 Timespan_t delay) {
    if (next_id < MAX_TASKS) {
        task_vector[next_id].period = period;
        task_vector[next_id].elapsed = -delay;
        task_vector[next_id].flags = flags;
        task_vector[next_id].callback = handler;
        task_vector[next_id].name = name;
        return ++next_id;
    } else {
        TRACELN("Failed to register task");
        return 0;
    }
}

bool Scheduler::defer_call(DeferredTask* task) {
    // Add our task into the linkedlist
    if (NULL == task || task->next != NULL) {
        return false;
    }
    task->elapsed = 0;
    if (NULL == root) {
        // Set as head
        root = task;
    } else {
        DeferredTask* iter = root;
        while (iter->next != NULL) {
            iter = iter->next;
        }
        iter->next = task;
    }

    task->next = NULL;
    return true;
}

bool Scheduler::defer_priority(DeferredTask* task) {
    // Place task at front of queue and set timer to zero
    if (NULL == task || task->next != NULL) {
        return false;
    }
    task->elapsed = task->delay + 1;
    if (NULL == root) {
        root = task;
    } else {
        task->next = root;
        root = task;
    }
    return true;
}

void Scheduler::start_scheduler() { last_tick = schedule_time(); }

void Scheduler::run() {
    PERF("Scheduler", "Enter");
    Timespan_t delta = schedule_time() - last_tick;
    if (delta < 0) {
        // Skip a timeslice
        delta = 0;
    }
    if (delta == 0) {
        // Don't call the same time slice (prevent double+ calls)
        PERF("Sheduler", "Skip");
        return;
    }
    last_tick = schedule_time();
    for (uint8_t i = 0; i < next_id; ++i) {
        Task& task = task_vector[i];
        if (task.flags & TASK_FLAG_ENABLED) {
            task.elapsed += delta;
            // Check timing
            if (task.elapsed >= task.period) {
                if (NULL != task.callback) {
                    PERF(task.name, "Enter");
                    task.callback(task.context);
                    PERF(task.name, "Exit");
                }
                task.elapsed = 0;
            }
        }
    }
    // Walk the list, prune if possible
    DeferredTask* iter = root;
    DeferredTask* prev = NULL;
    DeferredTask* call_list = NULL;
    DeferredTask* last_call = NULL;
    PERF("Scheduler", "ScanDeferred");
    while (NULL != iter) {
        iter->elapsed += delta;
        if (iter->elapsed >= iter->delay) {
            // Save pointer, remove from list
            iter->elapsed = 0;
            DeferredTask* invoke = iter;
            if (prev == NULL) {
                // Call root
                root = root->next;
                iter = root;
            } else {
                // Patch previous to reference next
                prev->next = iter->next;
                iter = iter->next;
            }
            if (NULL == call_list) {
                call_list = invoke;
                last_call = call_list;
                last_call->next = NULL;
            } else {
                last_call->next = invoke;
                last_call = last_call->next;
                last_call->next = NULL;
            }
        } else {
            prev = iter;
            iter = iter->next;
        }
    }
    PERF("Scheduler", "ExecDeferred");
    while (NULL != call_list) {
        DeferredTask* ptr = call_list;
        call_list = ptr->next;
        if (ptr->callback != NULL) {
            ptr->callback(NULL);
        }
        if (ptr->onfree != NULL) {
            ptr->onfree(ptr);
        }
    }
    PERF("Scheduler", "Exit");
}
void Scheduler::start_task(uint8_t id, Timespan_t delay) {
    id = id_to_index(id);
    if (id < MAX_TASKS) {
        task_vector[id].flags |= TASK_FLAG_ENABLED;
        task_vector[id].elapsed = -delay;
    }
}
void Scheduler::stop_task(uint8_t id) {
    id = id_to_index(id);
    if (id < MAX_TASKS) {
        task_vector[id].flags &= ~TASK_FLAG_ENABLED;
    }
}
void Scheduler::delay_task(uint8_t id, Timespan_t delay) {
    id = id_to_index(id);
    if (id < MAX_TASKS) {
        task_vector[id].elapsed -= delay;
    }
}

Timespan_t Scheduler::schedule_time() { return micros(); }

Timespan_t Scheduler::milliseconds_since(Timespan_t t) {
    return (schedule_time() - t) / 1000;
}

Timespan_t Scheduler::microseconds_since(Timespan_t t) {
    return schedule_time() - t;
}

void Scheduler::set_performance_callback(PerformanceHook_t cb) {
    performance_callback = cb;
}

void free_deferred(void* context) {
    global_defer_queue.free((DeferredTask*)context);
}

bool call_deferred(Scheduler* sched, Timespan_t delay, TaskHandler_t handler,
                   const char* name) {
    DeferredTask* ptr = global_defer_queue.alloc();
    if (NULL == ptr) {
        if (NULL != name) {
            TRACEF("%s:", name);
        }
        TRACE("Alloc failure\n");

        return false;
    }
    ptr->name = name;
    ptr->elapsed = 0;
    ptr->next = NULL;
    ptr->delay = delay;
    ptr->callback = handler;
    ptr->onfree = free_deferred;
    if (NULL != sched) {
        sched->defer_call(ptr);
    }
    return true;
}
