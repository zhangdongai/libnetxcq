#pragma once

#include "common/thread/thread.h"

#include <map>

#include "connector/connector.h"

class ConnectorThread : public Thread {
public:
    ConnectorThread();
    ~ConnectorThread();
    void init();
    void main_loop();

    void accept_connector(const int socket_fd);

private:
    void exec_read(const int socket_fd);
    void exec_parse(const int socket_fd);
    void exec_write(const int socket_fd);

    void delete_connector(const int socket_fd);
    void record_errno();

    std::map<int, std::shared_ptr<Connector>> connectors_;
    int epoll_fd_;
};
