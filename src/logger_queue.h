#pragma once

#include "logger_queue.h"
#include <cstddef>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace log {

template<typename T>
class LoggerQueue final {
public:
    explicit LoggerQueue(size_t capacity) : m_capacity(capacity) {}
    ~LoggerQueue() = default;
    LoggerQueue(const LoggerQueue&) = delete;
    LoggerQueue& operator=(const LoggerQueue&) = delete;

    void Push(T&& element) {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.size() + 1 > m_capacity) { // is full
            m_notfull.wait(lock);
        }
        m_queue.push(std::move(element));
        m_notempty.notify_one();
    }

    void Pop(T* element) {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty()) { // is empty
            m_notempty.wait(lock);
        }
        if (element != nullptr) {
            *element = std::move(m_queue.front());
        }
        m_queue.pop();
        m_notfull.notify_one();
    }

private:
    const size_t m_capacity;
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_notfull;
    std::condition_variable m_notempty;
};

} // namespace log
