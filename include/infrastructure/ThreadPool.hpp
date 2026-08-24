#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

namespace tradeflow {

class ThreadPool {
public:
    explicit ThreadPool(std::size_t workers = std::max<std::size_t>(1U, std::thread::hardware_concurrency()))
        : stop_(false) {
        for (std::size_t i = 0; i < workers; ++i) {
            workers_.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex_);
                        condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
                        if (stop_ && tasks_.empty()) {
                            return;
                        }
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        shutdown();
    }

    template <class F>
    auto submit(F&& task) -> std::future<decltype(task())> {
        using ReturnType = decltype(task());
        auto packaged = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<F>(task));
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if (stop_) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            tasks_.emplace([packaged]() { (*packaged)(); });
        }
        condition_.notify_one();
        return packaged->get_future();
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stop_;
};

}  // namespace tradeflow
