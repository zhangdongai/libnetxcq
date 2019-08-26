#pragma once

#include "common/macros.h"
#include "packet/udp_packet.h"

class UDPSocket {
public:
    UDPSocket();
    ~UDPSocket();

    bool init(const int socket_fd, const int epoll_fd);
    void close_socket();
    int exec_read(UDPPacket* const udp_packet);
    int exec_write();

    int get_socketid() const {return socket_fd_;}

private:
    int socket_fd_ = invalid_id;
    int epoll_fd_ = invalid_id;
    struct epoll_event event_;
};
