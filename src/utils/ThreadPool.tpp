#ifndef ANTDSB_THREAD_POOL_TPP
#define ANTDSB_THREAD_POOL_TPP

#include <type_traits>
#include <future>
#include <functional>

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>>
{
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();

    {
        std::scoped_lock lock(queue_mutex);
        if (stop.load()) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks.emplace([task](){ (*task)(); });
    }

    condition.notify_one();
    return res;
}

#endif // ANTDSB_THREAD_POOL_TPP