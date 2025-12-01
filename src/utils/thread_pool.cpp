//
// Created by sanek on 02/12/2025.
//

#include "thread_pool.h"
#include <stdexcept>
#include <utility>

ThreadPool::ThreadPool(size_t threads)
    : stop(false)
{
    if (threads == 0) {
        threads = 1;  // защита на случай 0
    }

    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock lock(queue_mutex);
                    condition.wait(lock, [this]{ return stop.load() || !tasks.empty(); });
                    if (stop.load() && tasks.empty())
                        return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    stop.store(true);
    condition.notify_all();
    for (auto& worker : workers) {
        if(worker.joinable())
            worker.join();
    }
}

size_t ThreadPool::size() const noexcept
{
    return workers.size();
}

size_t ThreadPool::pendingTasks() const noexcept
{
    std::scoped_lock lock(queue_mutex);
    return tasks.size();
}