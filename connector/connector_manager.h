#pragma once

#include <vector>

#include "connector/connector_thread.h"
#include "common/macros.h"

class ConnectorManager {
public:
    ~ConnectorManager();
    void accept_connector(const int socket_fd);

private:
    void init();

    std::vector<std::shared_ptr<ConnectorThread>> connector_threads_;
    int thread_num_ = invalid_index;

    SINGLETON_DECLARE(ConnectorManager);
};
