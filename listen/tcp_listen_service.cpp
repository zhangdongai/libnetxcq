#include "listen/tcp_listen_service.h"

#include <string.h>

#include "log/log.h"
#include "common/config/listen_flags.h"
#include "connector/connector_manager.h"

TCPListenService::TCPListenService() {
    stop_ = false;
}

TCPListenService::~TCPListenService() {
    stop();
    close(listen_fd_);
}

void TCPListenService::stop() {
    ListenService::stop();
}

void TCPListenService::init_config() {
}

bool TCPListenService::init_socket() {
    DEBUG_LOG("listen port = %d", FLAGS_port);
    DEBUG_LOG("communication type = %s", FLAGS_communication_type.c_str());
    struct sockaddr_in listen_addr;
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&listen_addr, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(FLAGS_port);
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret = bind(listen_fd_, (struct sockaddr*)&listen_addr, sizeof(listen_addr));
    if (ret == invalid_id) {
        const char* error = "error occured while bind";
        switch (errno) {
        case EADDRINUSE:
            error = "The given address is already in use";
        break;
        case EINVAL:
            error = "The socket is already bound to an address.";
        break;
        default:
        break;
        }
        ERROR_LOG(error);
        close(listen_fd_);
        return false;
    }
    listen(listen_fd_, 256);

    epoll_fd_ = epoll_create(MAX_EVENT);
    if (epoll_fd_ == invalid_id) {
        ERROR_LOG("create epoll error!");
        close(listen_fd_);
        return false;
    }

    struct epoll_event event;
    event.data.fd = listen_fd_;
    event.events = EPOLLIN | EPOLLHUP |EPOLLERR;

    ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &event);
    if (ret == invalid_id) {
        ERROR_LOG("add epoll error!");
        close(listen_fd_);
        return false;
    }

    DEBUG_LOG("create socket success!");
    return true;
}

void TCPListenService::run_logic() {
    ListenService::run_logic();

    // init ConnectorManager
    ConnectorManager::instance();
}

void TCPListenService::main_loop() {
    struct epoll_event events[MAX_EVENT];
    while(!stop_) {
        int fds = epoll_wait(epoll_fd_, events, MAX_EVENT, 100);
        for (int i = 0; i < fds; ++i) {
            struct epoll_event& tmp_ev = events[i];
            if (tmp_ev.data.fd == listen_fd_) {
                accept_connection();
            }
            else if (tmp_ev.events & EPOLLIN) {
            }
            else if (tmp_ev.events & EPOLLOUT) {
            }
        }
    }
}

bool TCPListenService::accept_connection() {
    struct sockaddr_in socket_in;
    socklen_t socket_len = sizeof(struct sockaddr_in);
    int new_socket_fd = accept(listen_fd_, (struct sockaddr*)&socket_in, &socket_len);
    if (new_socket_fd == -1) {
        const char* err_con = nullptr;
        switch(errno) {
        case EAGAIN:
            err_con = "The socket is marked nonblocking and no connections are "
                      "present to be accepted";
        break;
        case EBADF:
            err_con = "sockfd is not an open file descriptor";
        break;
        case EFAULT:
            err_con = "The addr argument is not in a writable part of the user "
                      "address space";
        break;
        case EINTR:
            err_con = "The system call was interrupted by a signal";
        break;
        case EINVAL:
            err_con = "Socket is not listening for connections, or addrlen is invalid";
        break;
        case ENOMEM:
            err_con = "Not enough free memory";
        break;
        case EPERM:
            err_con = "Firewall rules forbid connection";
        break;
        default:
            err_con = "unknown error";
        break;
        }
        ERROR_LOG(err_con);
        return false;
    }
    const char* addr_in = inet_ntoa(socket_in.sin_addr);
    DEBUG_LOG("establish connection from %s", addr_in);

    int flag = fcntl(new_socket_fd, F_GETFL, 0);
    int ret = fcntl(new_socket_fd, F_SETFL, flag | O_NONBLOCK);
    if (ret == invalid_id) {
        ERROR_LOG("set non block for socket %d failed, errno = %d", new_socket_fd, ret);
        return false;
    }

    ConnectorManager::instance()->accept_connector(new_socket_fd);
    return true;
}
