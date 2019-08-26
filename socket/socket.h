#pragma once

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "common/macros.h"
#include "socket/socket_input.h"
#include "socket/socket_output.h"

class Socket {
public:
    Socket();
    ~Socket();

    bool init(const int socket_fd, const int epoll_fd);
    void close_socket();
    int exec_read();
    int exec_write();

    int get_socketid() const {return socket_fd_;}

    void fetch_header(char* const header, const int header_size);
    void fetch_content(char* const content, const int size);

    void write(const char* const buff, const int size);

private:
    void fetch(char* const buff, const int size);

    int socket_fd_ = invalid_id;
    int epoll_fd_ = invalid_id;
    struct epoll_event event_;

    SocketInput socket_input_;
    SocketOutput socket_output_;
};
