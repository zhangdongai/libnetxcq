#include "listen/listen_service.h"

#include <string.h>

#include "log/log.h"
#include "common/config/listen_flags.h"
#include "connector/connector_manager.h"

ListenService::ListenService() {
    stop_ = false;
}

ListenService::~ListenService() {
    stop();
}

void ListenService::stop() {
    stop_ = true;
}

bool ListenService::init() {
    init_config();
    if (!init_socket())
        return false;
    return true;
}

void ListenService::run() {
    run_logic();
}

void ListenService::run_logic() {
    thread_ = std::make_shared<std::thread>([this](){
        DEBUG_LOG("begin to loop!");
        this->main_loop();});
}
