#include "connector/connector_manager.h"

#include <algorithm>

#include "common/utils.h"
#include "common/config/connector_flags.h"
#include "common/global.h"
#include "log/log.h"

ConnectorManager::ConnectorManager() {
    init();
}

ConnectorManager::~ConnectorManager() {
    std::for_each(connector_threads_.begin(), connector_threads_.end(),
        [](std::shared_ptr<ConnectorThread>& elem) {
            elem->stop();});
}

void ConnectorManager::init() {
    const char* conf_path = "data/config/connector.conf";
    char flag_file[128] = {0};
    snprintf(flag_file, sizeof(flag_file), "%s%s", g_prefix_res_path, conf_path);
    google::SetCommandLineOption("flagfile", flag_file);
    thread_num_ = static_cast<int>(get_procnum() * FLAGS_thread_num_ratio);
    DEBUG_LOG("get thread number success, processor number = %d, ratio = %lf, thread number = %d",
        get_procnum(), FLAGS_thread_num_ratio, thread_num_);

    connector_threads_.resize(thread_num_);

    for (int i = 0; i < thread_num_; ++i) {
        connector_threads_[i] = std::make_shared<ConnectorThread>();
        connector_threads_[i]->init();
    }
    DEBUG_LOG("create %d connector thread success!", thread_num_);
}

void ConnectorManager::accept_connector(const int socket_fd) {
    int index = hash_index(socket_fd, thread_num_);
    connector_threads_[index]->accept_connector(socket_fd);
}
