#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <iostream>
#include <deque>
#include <mutex>
#include <condition_variable>


template <class T>
class BlockQueue
{
private:
    bool is_close;  //队列是否关闭
    size_t capacity_; //队列容量
    std::deque<T> deque_; //队列
    std::mutex mtx_; //互斥锁
    std::condition_variable condition_producer_; //生产者条件变量
    std::condition_variable condition_consumer_; //消费者条件变量
public:
    BlockQueue(int max_size = 1024);
    ~BlockQueue();

    bool empty();
    bool full();
    size_t size();
    size_t capacity();
    void clear();

    void push_front(const T& item);
    void push_back(const T& item);
    bool pop(T& item);
    bool pop(T& item, int timeout);

    T front();
    T back();
    
    void flush();
    void close();
};


template <class T>
BlockQueue<T>::BlockQueue(int max_size): capacity_(max_size){
    assert(max_size > 0);
    is_close = false;
}

template <class T>
BlockQueue<T>::~BlockQueue(){
    close();
}   

template <class T>
bool BlockQueue<T>::empty(){
    std::lock_guard<std::mutex>locker(mtx_);
    return deque_.empty();
}


template <class T>
bool BlockQueue<T>::full(){
    std::lock_guard<std::mutex>locker(mtx_);
    return deque_.size() >= capacity_;
}

template <class T>
size_t BlockQueue<T>::size(){
    std::lock_guard<std::mutex>locker(mtx_);
    return deque_.size();
}

template <class T>
size_t BlockQueue<T>::capacity(){
    std::lock_guard<std::mutex>locker(mtx_);
    return capacity_;
}


template <class T>
void BlockQueue<T>::clear(){
    std::lock_guard<std::mutex>locker(mtx_);
    deque_.clear();
}

template <class T>
void BlockQueue<T>::push_front(const T& item){
    std::unique_lock<std::mutex>locker(mtx_);
    while(deque_.size() >= capacity_){
        condition_producer_.wait(locker);
    }
    deque_.push_front(item);
    condition_consumer_.notify_one();
}

template <class T>
void BlockQueue<T>::push_back(const T& item){
    std::unique_lock<std::mutex>locker(mtx_);
    while(deque_.size() >= capacity_){
        condition_producer_.wait(locker);
    }
    deque_.push_back(item);
    condition_consumer_.notify_one();
}

template <class T>
bool BlockQueue<T>::pop(T& item){
    std::unique_lock<std::mutex>locker(mtx_);
    while(deque_.empty()){
        if(is_close){
            return false;
        }
        condition_consumer_.wait(locker);
    }
    item = deque_.front();
    deque_.pop_front();
    condition_producer_.notify_one();
    return true;
}

template <class T>
bool BlockQueue<T>::pop(T& item, int timeout){
    std::unique_lock<std::mutex>locker(mtx_);
    const std::cv_status TIMEOUT_STATUS = std::cv_status::timeout;
    while(deque_.empty()){
        if(is_close){
            return false;
        }
        if(condition_consumer_.wait_for(locker, std::chrono::milliseconds(timeout)) == TIMEOUT_STATUS){
            return false;
        }
    }
    item = deque_.front();
    deque_.pop_front();
    condition_producer_.notify_one();
    return true;
}

template <class T>
T BlockQueue<T>::front(){
    std::lock_guard<std::mutex>locker(mtx_);
    return deque_.front();
}
template <class T>
T BlockQueue<T>::back(){
    std::lock_guard<std::mutex>locker(mtx_);
    return deque_.back();
}

//唤醒消费者
template <class T>
void BlockQueue<T>::flush(){
    condition_consumer_.notify_one();
}

//关闭阻塞队列，唤醒所有生产者和消费者
template <class T>
void BlockQueue<T>::close(){
    {
        std::lock_guard<std::mutex>locker(mtx_);
        deque_.clear();
        is_close = true;
    }
    condition_consumer_.notify_all();
    condition_producer_.notify_all();
}

#endif