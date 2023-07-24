#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

// 异步写日志的日志队列
template <typename T>
class LockQueue
{
public:
    // 多个worker线程都会写日志
    void push(const T &data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_condvariable.notify_one();
    }
    // 一个线程读日志写进本地日志文件
    T pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            m_condvariable.wait(lock);
        }
        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;
};