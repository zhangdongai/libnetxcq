#pragma once

#include <memory>
#include <mutex>
#include <thread>

class Thread {
public:
    Thread();
    virtual ~Thread();
    virtual void init();
    virtual void main_loop() = 0;
    virtual void stop();

protected:
    bool stop_ = false;
    std::mutex mutex_;

private:
    std::shared_ptr<std::thread> thread_;
};
