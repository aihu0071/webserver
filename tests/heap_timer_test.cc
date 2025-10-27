#include "../code/heap_timer/heap_timer.h"
#include "../code/log/log.h"
#include <cassert>

void CallbackFunction() {
    LOG_DEBUG("Callback function is called!")
}

// 测试 add 函数
void TestAdd() {
    HeapTimer timer;
    int id = 1;
    int timeout = 1000;  // 超时时间为 1000 毫秒

    timer.Add(id, timeout, CallbackFunction);
    assert(timer.Size() == 1);
}

// 测试 tick 函数
void TestTick() {
    HeapTimer timer;
    int id = 1;
    int timeout = 100;  // 超时时间为 100 毫秒

    timer.Add(id, timeout, CallbackFunction);

    // 等待一段时间，确保定时器超时
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    timer.Tick();
    assert(timer.Size() == 0);
}

// 测试 GetNextTick 函数
void TestGetNextTick() {
    HeapTimer timer;
    int id = 1;
    int timeout = 10000;  // 超时时间为 10000 毫秒

    timer.Add(id, timeout, CallbackFunction);
    int next_tick = timer.GetNextTick();
    assert(next_tick >= 0);
}

int main() {
    Log* logger = Log::GetInstance();
    logger->Init(0, "./logs/", ".log", 1024);

    TestAdd();
    TestTick();
    TestGetNextTick();
    return 0;
}
