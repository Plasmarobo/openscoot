#include <framework.h>
#include <gtest/gtest.h>
#include <test_main.h>
#include <trace.h>

#include "scheduler.h"

namespace {
uint32_t call_count = 0;
void task_callback(void* ctx) { call_count += 1; }
}  // namespace

TEST(Scheduler, TaskCalled) {
    Scheduler sched;
    call_count = 0;
    uint8_t handle = sched.register_task(SCHED_MILLISECONDS(10), task_callback);
    EXPECT_NE(handle, TASK_INVALID);
    sched.start_scheduler();
    delay(5);
    sched.run();
    sched.start_task(handle);
    EXPECT_EQ(call_count, 0);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 0);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 1);
    delay(10);
    sched.run();
    EXPECT_EQ(call_count, 2);
    sched.run();
    EXPECT_EQ(call_count, 2);
    delay(10);
    sched.run();
    EXPECT_EQ(call_count, 3);
}

TEST(Scheduler, TaskStopped) {
    Scheduler sched;
    call_count = 0;
    uint8_t handle = sched.register_task(SCHED_MILLISECONDS(10), task_callback);
    EXPECT_NE(handle, TASK_INVALID);
    sched.start_scheduler();
    delay(5);
    sched.run();
    sched.start_task(handle);
    sched.run();
    EXPECT_EQ(call_count, 0);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 0);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 1);
    sched.stop_task(handle);
    delay(10);
    sched.run();
    EXPECT_EQ(call_count, 1);
    delay(10);
    sched.run();
    EXPECT_EQ(call_count, 1);
}

TEST(Scheduler, DeferCalledOnce) {
    Scheduler sched;
    call_count = 0;
    DeferredTask call(SCHED_MILLISECONDS(10), task_callback);
    sched.defer_call(&call);
    sched.start_scheduler();
    EXPECT_EQ(call_count, 0);
    sched.run();
    EXPECT_EQ(call_count, 0);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 0);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 1);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 1);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 1);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 1);
}

TEST(Scheduler, DeferRecycled) {
    Scheduler sched;
    call_count = 0;
    DeferredTask call(SCHED_MILLISECONDS(10), task_callback);
    sched.defer_call(&call);
    sched.start_scheduler();
    EXPECT_EQ(call_count, 0);
    sched.run();
    EXPECT_EQ(call_count, 0);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 0);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 1);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 1);
    sched.defer_call(&call);
    sched.run();
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 1);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 2);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 2);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 2);
}

namespace {
uint8_t counters[7];
void tc_a(void* ctx) { counters[0] += 1; }
void tc_b(void* ctx) { counters[1] += 1; }
void tc_c(void* ctx) { counters[2] += 1; }
void tc_d(void* ctx) { counters[3] += 1; }
void tc_e(void* ctx) { counters[4] += 1; }
void tc_f(void* ctx) { counters[5] += 1; }
void tc_g(void* ctx) { counters[6] += 1; }
}  // namespace

TEST(Scheduler, ManyDeferred) {
    Scheduler sched;
    for (uint8_t i = 0; i < 7; ++i) {
        counters[i] = 0;
    }
    DeferredTask call_a(SCHED_MILLISECONDS(10), &tc_a);
    DeferredTask call_b(SCHED_MILLISECONDS(10), &tc_b);
    DeferredTask call_c(SCHED_MILLISECONDS(11), &tc_c);
    DeferredTask call_d(SCHED_MILLISECONDS(15), &tc_d);
    DeferredTask call_e(SCHED_MILLISECONDS(12), &tc_e);
    DeferredTask call_f(SCHED_MILLISECONDS(30), &tc_f);
    DeferredTask call_g(SCHED_MILLISECONDS(31), &tc_g);
    sched.start_scheduler();
    sched.run();
    sched.defer_call(&call_a);
    sched.defer_call(&call_b);
    sched.defer_call(&call_c);
    sched.defer_call(&call_d);
    sched.defer_call(&call_e);
    sched.defer_call(&call_f);
    sched.defer_call(&call_g);
    for (uint8_t i = 0; i < 7; ++i) {
        EXPECT_EQ(counters[i], 0);
    }
    sched.run();
    for (uint8_t i = 0; i < 7; ++i) {
        EXPECT_EQ(counters[i], 0);
    }
    delay(5);
    sched.run();
    for (uint8_t i = 0; i < 7; ++i) {
        EXPECT_EQ(counters[i], 0);
    }
    delay(5);  // at 10, a, aa
    sched.run();

    EXPECT_EQ(counters[0], 1);
    EXPECT_EQ(counters[1], 1);
    EXPECT_EQ(counters[2], 0);
    EXPECT_EQ(counters[3], 0);
    EXPECT_EQ(counters[4], 0);
    EXPECT_EQ(counters[5], 0);
    EXPECT_EQ(counters[6], 0);

    delay(1);  // at 11, aaa
    sched.run();
    EXPECT_EQ(counters[0], 1);
    EXPECT_EQ(counters[1], 1);
    EXPECT_EQ(counters[2], 1);
    EXPECT_EQ(counters[3], 0);
    EXPECT_EQ(counters[4], 0);
    EXPECT_EQ(counters[5], 0);
    EXPECT_EQ(counters[6], 0);
    delay(1);
    sched.run();  // at 12, bb
    EXPECT_EQ(counters[0], 1);
    EXPECT_EQ(counters[1], 1);
    EXPECT_EQ(counters[2], 1);
    EXPECT_EQ(counters[3], 0);
    EXPECT_EQ(counters[4], 1);
    EXPECT_EQ(counters[5], 0);
    EXPECT_EQ(counters[6], 0);
    delay(4);  // at 15, b
    sched.run();
    EXPECT_EQ(counters[0], 1);
    EXPECT_EQ(counters[1], 1);
    EXPECT_EQ(counters[2], 1);
    EXPECT_EQ(counters[3], 1);
    EXPECT_EQ(counters[4], 1);
    EXPECT_EQ(counters[5], 0);
    EXPECT_EQ(counters[6], 0);
    delay(5);
    sched.run();  // at 20
    EXPECT_EQ(counters[0], 1);
    EXPECT_EQ(counters[1], 1);
    EXPECT_EQ(counters[2], 1);
    EXPECT_EQ(counters[3], 1);
    EXPECT_EQ(counters[4], 1);
    EXPECT_EQ(counters[5], 0);
    EXPECT_EQ(counters[6], 0);
    delay(5);
    sched.run();  // at 25
    EXPECT_EQ(counters[0], 1);
    EXPECT_EQ(counters[1], 1);
    EXPECT_EQ(counters[2], 1);
    EXPECT_EQ(counters[3], 1);
    EXPECT_EQ(counters[4], 1);
    EXPECT_EQ(counters[5], 0);
    EXPECT_EQ(counters[6], 0);
    delay(4);
    sched.run();  // at 30, c
    EXPECT_EQ(counters[0], 1);
    EXPECT_EQ(counters[1], 1);
    EXPECT_EQ(counters[2], 1);
    EXPECT_EQ(counters[3], 1);
    EXPECT_EQ(counters[4], 1);
    EXPECT_EQ(counters[5], 1);
    EXPECT_EQ(counters[6], 0);
    delay(30);
    sched.run();  // at 60, cc
    EXPECT_EQ(counters[0], 1);
    EXPECT_EQ(counters[1], 1);
    EXPECT_EQ(counters[2], 1);
    EXPECT_EQ(counters[3], 1);
    EXPECT_EQ(counters[4], 1);
    EXPECT_EQ(counters[5], 1);
    EXPECT_EQ(counters[6], 1);
    delay(10);
    sched.run();  // at 70
    EXPECT_EQ(counters[0], 1);
    EXPECT_EQ(counters[1], 1);
    EXPECT_EQ(counters[2], 1);
    EXPECT_EQ(counters[3], 1);
    EXPECT_EQ(counters[4], 1);
    EXPECT_EQ(counters[5], 1);
    EXPECT_EQ(counters[6], 1);
}

TEST(Scheduler, ManyTasks) {
    Scheduler sched;
    call_count = 0;
    uint8_t handle_a =
        sched.register_task(SCHED_MILLISECONDS(10), task_callback);
    uint8_t handle_b =
        sched.register_task(SCHED_MILLISECONDS(15), task_callback);
    EXPECT_NE(handle_a, TASK_INVALID);
    EXPECT_NE(handle_b, TASK_INVALID);
    sched.start_scheduler();
    sched.run();
    delay(5);
    sched.run();
    sched.start_task(handle_a);
    sched.start_task(handle_b);
    sched.run();
    EXPECT_EQ(call_count, 0);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 0);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 1);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 2);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 3);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 3);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 5);
    delay(5);
    sched.run();
    EXPECT_EQ(call_count, 5);
}

namespace {

uint8_t tick_a_cnt = 0;
uint8_t tick_b_cnt = 0;
uint8_t tick_c_cnt = 0;
void tick_a(void* ctx) { ++tick_a_cnt; }
void tick_b(void* ctx) { ++tick_b_cnt; }
void tick_c(void* ctx) { ++tick_c_cnt; }
}  // namespace

TEST(Scheduler, Loop) {
    Scheduler sched;

    DeferredTask defer(SCHED_MILLISECONDS(50), tick_c);
    uint32_t cycles = 1001;
    sched.start_scheduler();
    sched.register_task(SCHED_MILLISECONDS(100), tick_a, TASK_FLAG_ENABLED);
    sched.register_task(SCHED_MILLISECONDS(300), tick_b, TASK_FLAG_ENABLED);
    sched.defer_call(&defer);
    while (cycles > 0) {
        sched.run();
        --cycles;
        delay(1);
    }
    EXPECT_EQ(tick_a_cnt, 10);
    EXPECT_EQ(tick_b_cnt, 3);
    EXPECT_EQ(tick_c_cnt, 1);
}

TEST(Scheduler, Global) {
    Scheduler* sched = get_global_scheduler();

    tick_c_cnt = 0;
    sched->start_scheduler();
    call_deferred(SCHED_MILLISECONDS(100), tick_c, __PRETTY_FUNCTION__);
    uint32_t cycles = 200;
    while (cycles > 0) {
        sched->run();
        --cycles;
        delay(1);
    }
    EXPECT_EQ(tick_c_cnt, 1);
}

namespace {
int ticks = 0;
int tocks = 0;
void tick(void* ctx) {
    ticks += 1;
    call_deferred(SCHED_MILLISECONDS(1), tick, __PRETTY_FUNCTION__);
}
void tock(void* ctx) {
    tocks += 1;
    call_deferred(SCHED_MILLISECONDS(5), tock, __PRETTY_FUNCTION__);
}
}  // namespace

TEST(Scheduler, TickTock) {
    Scheduler* sched = get_global_scheduler();
    sched->start_scheduler();
    tick(NULL);
    tock(NULL);
    uint32_t cycles = 20;
    while (cycles > 0) {
        delay(1);
        sched->run();
        --cycles;
    }
    // Should be cycles + 1
    EXPECT_EQ(ticks, 21);
    // schould be (cycles / 5) + 1
    EXPECT_EQ(tocks, 5);
}
