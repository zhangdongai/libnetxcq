#include "common/thread/thread.h"

Thread::Thread() {
}

void Thread::init() {
    thread_ = std::make_shared<std::thread>([this]() {
            this->main_loop();});
}

Thread::~Thread() {
    thread_->join();
    thread_.reset();
}

void Thread::stop() {
    stop_ = true;
}
