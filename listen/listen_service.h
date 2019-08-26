#pragma once

#include <thread>

#include "common/macros.h"

class ListenService {
public:
    ListenService();
    virtual ~ListenService();
    virtual bool init();
    virtual void run();
    virtual void run_logic();
    virtual void main_loop() = 0;
    virtual void stop();

protected:
    int listen_fd_ = invalid_id;
    int epoll_fd_ = invalid_id;

    bool stop_ = false;

private:
    virtual void init_config() = 0;
    virtual bool init_socket() = 0;

    std::shared_ptr<std::thread> thread_;
};
