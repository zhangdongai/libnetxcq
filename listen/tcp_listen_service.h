#pragma once

#include <map>

#include "listen/listen_service.h"
#include "connector/connector.h"

class TCPListenService :public ListenService {
public:
    TCPListenService();
    ~TCPListenService();
    void main_loop();
    void stop();

private:
    void init_config();
    bool init_socket();
    void run_logic();
    bool accept_connection();

    std::map<int, std::shared_ptr<Connector>> connectors_;
};
