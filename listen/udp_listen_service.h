#pragma once

#include <map>

#include "common/thread/thread_pool.h"
#include "listen/listen_service.h"
#include "socket/udp_socket.h"

class UDPListenService :public ListenService {
public:
    UDPListenService();
    ~UDPListenService();
    void main_loop();
    void stop();

    static void exec_packet(const UDPPacket& packet);
    static void test(int a);

private:
    void init_config();
    bool init_socket();
    void run_logic();
    bool test_recv();

    std::shared_ptr<UDPSocket> socket_;
    std::shared_ptr<ThreadPool> thread_pool_;
};
