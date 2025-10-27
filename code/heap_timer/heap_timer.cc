#include "heap_timer.h"

void HeapTimer::Add(int id, int timeout, const TimeoutCallBack& cb) {
    if(ref_.count(id)) {
        Adjust(id, timeout);
        heap_[ref_[id]].cb = cb;
    } else {
        size_t n = heap_.size();
        ref_[id] = n;
        heap_.emplace_back(id, Clock::now() + MS(timeout), cb);
        SiftUp(n);
    }
}

void HeapTimer::Adjust(int id, int timeout) {
    assert(ref_.count(id));
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);
    if (!SiftDown(ref_[id], heap_.size()))
        SiftUp(ref_[id]);
}

//执行定时器任务
void HeapTimer::DoWork(int id) {
    if(heap_.empty() || ref_.count(id)){
        return;
    }
    size_t i = ref_[id];
    heap_[i].cb();
    Delete(i);
}

//清除到期的定时器
void HeapTimer::Tick() {
    if (heap_.empty())
        return;
    while (!heap_.empty()) {
        TimerNode node = heap_.front();
        if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0)
            break;
        node.cb();
        Pop();
    }
}

int HeapTimer::GetNextTick() {
    Tick();
    int res = -1;
    if (!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if (res < 0) res = 0;   
    }
    return res;
}

void HeapTimer::Pop() {
    assert(!heap_.empty());
    Delete(0);
}

void HeapTimer::Clear() {
    heap_.clear();
    ref_.clear();
}


void HeapTimer::SwapNode(size_t i, size_t j) {
    assert(i < heap_.size());
    assert(j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

// 向上调整算法
void HeapTimer::SiftUp(size_t child) {
    assert(child < heap_.size());
    size_t parent = (child - 1) / 2;
    while (parent > 0) {
        if (heap_[parent] > heap_[child]) {
            SwapNode(child, parent);
            child = parent;
            parent = (child - 1) / 2;
        } else {
            break;
        }
    }
}

// 向下调整算法
// 返回：是否调整
bool HeapTimer::SiftDown(size_t parent, size_t n) {
    assert(parent < heap_.size());
    assert(n <= heap_.size());
    size_t child = 2 * parent + 1; // 左孩子
    size_t pos = parent;
    while (child < n) {
        if (child + 1 < n && heap_[child + 1] < heap_[child])
            ++child; // 右孩子
        if (heap_[parent] > heap_[child]) {
            SwapNode(child, parent);
            parent = child;
            child = 2 * parent + 1;
        } else {
            break;
        }
    }
    return pos < parent;
}

void HeapTimer::Delete(size_t i) {
    assert(!heap_.empty() && i < heap_.size());
    size_t n = heap_.size() - 1;
    if (i < n - 1) { // 跳过n
        SwapNode(i, n);
        if (!SiftDown(i, n))
            SiftUp(i);
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}
