//
// Created by sanek on 02/12/2025.
//

#ifndef ANTDSB_THREAD_POOL_H
#define ANTDSB_THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

class ThreadPool {
public:
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency());
    ~ThreadPool();

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;

    size_t size() const noexcept;
    size_t pendingTasks() const noexcept;

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    mutable std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};

#include "ThreadPool.tpp"

#endif //ANTDSB_THREAD_POOL_H