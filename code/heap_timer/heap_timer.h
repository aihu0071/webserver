#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <cassert>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <functional>

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef std::chrono::high_resolution_clock::time_point TimeStamp;


struct TimerNode{
    int id; //定时器id
    TimeStamp expires; //到期时间
    TimeoutCallBack cb; //回调函数

    TimerNode(int id_, TimeStamp expires_, TimeoutCallBack cb_)
        : id(id_), expires(expires_), cb(cb_) {}

    bool operator<(const TimerNode& t) const {
        return expires < t.expires;
    }
    bool operator>(const TimerNode& t) const {
        return expires > t.expires;
    }
};

class HeapTimer{
public:
    HeapTimer() { heap_.reserve(64); }
    ~HeapTimer() { Clear(); }

    void Add(int id, int timeout, const TimeoutCallBack& cb);
    void Adjust(int id, int timeout);
    void DoWork(int id);
    void Tick();
    int GetNextTick();
    void Pop();
    void Clear();

    size_t Size() { return heap_.size(); }

private:
    void SwapNode(size_t i, size_t j);
    void SiftUp(size_t child);
    bool SiftDown(size_t parent, size_t n);   //
    void Delete(size_t i);

    std::vector<TimerNode> heap_;   
    std::unordered_map<int, size_t> ref_; //id -> 堆中位置
};

#endif