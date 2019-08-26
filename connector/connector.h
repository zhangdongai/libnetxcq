#pragma once

#include <memory>

#include "socket/socket.h"
#include "user/user.h"

class Connector {
public:
    Connector();
    virtual ~Connector();

    bool create_socket(const int listen_fd, const int epoll_fd);
    void close_socket();
    int exec_read();
    int exec_write();
    virtual void exec_parse();
    virtual void send_packet(const int packet_id, const proto::Packet& packet);

    std::shared_ptr<Socket> get_socket() const {return socket_;}
    const int get_socketid() const {return socket_->get_socketid();}

private:
    std::shared_ptr<Socket> socket_;
    User user_;
};
