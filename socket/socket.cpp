#include "socket/socket.h"

#include <string.h>

#include "log/log.h"
#include "packet/connector.pb.h"

Socket::Socket() {
}

Socket::~Socket() {
    DEBUG_LOG("destruction called, socket_fd = %d", socket_fd_);
    close_socket();
}

void Socket::close_socket() {
    if (socket_fd_ != invalid_id) {
        close(socket_fd_);
        socket_fd_ = invalid_id;
    }
}

bool Socket::init(const int socket_fd, const int epoll_fd) {
    socket_fd_ = socket_fd;
    epoll_fd_ = epoll_fd;

    event_.data.fd = socket_fd_;
    // use Level Trigger as default
    // catch EPOLLIN & EPOLLOUT
    event_.events = EPOLLIN | EPOLLOUT| EPOLLHUP | EPOLLERR;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_fd_, &event_);

    DEBUG_LOG("create socket success, socketid = %d", socket_fd);
    return true;
}

int Socket::exec_read() {
    return socket_input_.feed(socket_fd_);
/*    memset(socketin_buf_, 0, MAX_BUFFER_LEN + 1);
    int recv_len = recv(socket_fd_, socketin_buf_, MAX_BUFFER_LEN, 0);
    if (recv_len > 0) {
        DEBUG_LOG("receive message %s, len = %d", socketin_buf_, recv_len);
        proto::CS_Ping packet;
        packet.ParseFromString(socketin_buf_);
        DEBUG_LOG("parse proto object success, param = %s", packet.param().data());
    }

    return recv_len;
*/
//    event_.events = EPOLLOUT | EPOLLET;
//    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket_fd_, &event_);
}

int Socket::exec_write() {
    return socket_output_.refresh(socket_fd_);
}

void Socket::fetch_header(char* const header, const int header_size) {
    fetch(header, header_size);
}

void Socket::fetch_content(char* const content, const int size) {
    fetch(content, size);
}

void Socket::fetch(char* const buff, const int size) {
    socket_input_.fetch(buff, size);
}

void Socket::write(const char* const buff, const int size) {
    socket_output_.write(buff, size);
}
