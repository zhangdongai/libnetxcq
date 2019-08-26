#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

template <typename T>
class Queue {
public:
    void push(const T& t);
    bool pop(T* const t);

private:
    std::queue<T> queue_;
    std::mutex mutex_;
};

template <typename T>
void Queue<T>::push(const T& t) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.emplace(t);
}
template <typename T>
bool Queue<T>::pop(T* const t) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
        return false;
    }

    *t = queue_.front();
    queue_.pop();
    return true;
}

class ThreadPool {
public:
    explicit ThreadPool(int32_t thread_count);
    ~ThreadPool();

    template <typename F, typename... Args>
    auto push(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;
        //-> std::future<decltype(f(args...))>;

private:
    Queue<std::function<void()>> q_;
    std::vector<std::thread> thread_list_;
    std::atomic<bool> stop_ = {false};
};

inline ThreadPool::ThreadPool(int32_t thread_count) {
    for (int32_t i = 0; i < thread_count; ++i) {
        thread_list_.emplace_back([this](){
            while (!stop_) {
                std::function<void()> f;
                if (q_.pop(&f)) {
                    f();
                    continue;
                }
                std::this_thread::yield();
            }
        });
    }
}

template <typename F, typename... Args>
auto ThreadPool::push(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    //-> std::future<decltype(f(args...))> {
    using ret_type = typename std::result_of<F(Args...)>::type;

    auto exec = std::make_shared<std::packaged_task<ret_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::function<void()> callback_f([exec](){
        (*exec)();
    });
    q_.push(callback_f);

    std::future<ret_type> ret = exec->get_future();
    return ret;
}

inline ThreadPool::~ThreadPool() {
    stop_.store(true);
    for (auto& e : thread_list_) {
        if (e.joinable())
            e.join();
    }
}
