#include "connector/connector_thread.h"

#include "common/macros.h"
#include "log/log.h"

ConnectorThread::ConnectorThread() {
}

ConnectorThread::~ConnectorThread() {
    if (epoll_fd_ != invalid_id)
        close(epoll_fd_);
}

void ConnectorThread::init() {
    Thread::init();

    epoll_fd_ = epoll_create(MAX_EVENT);
    if (epoll_fd_ == invalid_id) {
        ERROR_LOG("create epoll failed in ConnectorThread");
    }
}

void ConnectorThread::main_loop() {
    DEBUG_LOG("connector thread begin to loop");
    struct epoll_event events[MAX_EVENT];
    while(!stop_) {
        int fds = epoll_wait(epoll_fd_, events, MAX_EVENT, 100);
        for (int i = 0; i < fds; ++i) {
            struct epoll_event& tmp_ev = events[i];
            if (tmp_ev.events & EPOLLIN) {
                int socket_fd = tmp_ev.data.fd;
                exec_read(socket_fd);
                exec_parse(socket_fd);
            }
            else if (tmp_ev.events & EPOLLOUT) {
                int socket_fd = tmp_ev.data.fd;
                exec_write(socket_fd);
            }
            else if (tmp_ev.events & EPOLLHUP) {
                int socket_fd = tmp_ev.data.fd;
                DEBUG_LOG("close by client EPOLLHUP, socket_fd = %d", socket_fd);
                delete_connector(socket_fd);
            }
            else if (tmp_ev.events & EPOLLERR) {
                int socket_fd = tmp_ev.data.fd;
                DEBUG_LOG("close by client EPOLLERR, socket_fd = %d", socket_fd);
                delete_connector(socket_fd);
            }
        }
    }
}

// this function is called by another thread
void ConnectorThread::accept_connector(const int socket_fd) {
    std::shared_ptr<Connector> connector_ptr = std::make_shared<Connector>();
    bool create_ret = connector_ptr->create_socket(socket_fd, epoll_fd_);
    if (!create_ret) {
        ERROR_LOG("create new connection failed!");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connectors_[connector_ptr->get_socketid()] =
            connector_ptr;
    }
    DEBUG_LOG("create connector success, socketid = %d", socket_fd);

}

void ConnectorThread::delete_connector(const int socket_fd) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (connectors_.find(socket_fd) != connectors_.end()) {
        connectors_[socket_fd]->close_socket();
        connectors_.erase(socket_fd);
    }
}

void ConnectorThread::exec_read(const int socket_fd) {
    std::shared_ptr<Connector> connector_ptr = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connectors_.find(socket_fd) != connectors_.end())
            connector_ptr = connectors_[socket_fd];
    }

    if (!connector_ptr)
        return;

    // recv() is a time-consuming operation, not good to call in lock scope
    int ret = connector_ptr->exec_read();
    if (ret == 0) {
        DEBUG_LOG("socket is closed by opposite, socket_fd = %d", socket_fd);
        delete_connector(socket_fd);
    }
    else if (ret == invalid_id) {
        record_errno();
        delete_connector(socket_fd);
    }
}

void ConnectorThread::exec_parse(const int socket_fd) {
    std::shared_ptr<Connector> connector_ptr = nullptr;
    { 
        std::lock_guard<std::mutex> lock(mutex_);
        if (connectors_.find(socket_fd) != connectors_.end())
            connector_ptr = connectors_[socket_fd];
    }

    if (!connector_ptr)
        return;

    connector_ptr->exec_parse();
}

void ConnectorThread::exec_write(const int socket_fd) {
    std::shared_ptr<Connector> connector_ptr = nullptr;
    {   
        std::lock_guard<std::mutex> lock(mutex_);
        if (connectors_.find(socket_fd) != connectors_.end())
            connector_ptr = connectors_[socket_fd];
    }
    
    if (!connector_ptr)
        return;

    // send() is a time-consuming operation, not good to call in lock scope
    int ret = connector_ptr->exec_write();
    if (ret == invalid_id) {
        record_errno();
        delete_connector(socket_fd);
    }
}

void ConnectorThread::record_errno() {
    const char* error = nullptr;
    switch (errno) {
    case EAGAIN:
        error = "The socket is marked nonblocking and the receive operation would block, "
                "or a receive timeout had been set and the timeout expired before data was received";
    break;
    case EBADF:
        error = "The argument sockfd is an invalid descriptor.";
    break;
    case ECONNRESET:
        error = "A connection was forcibly closed by a peer.";
    break;
    case ECONNREFUSED:
        error = "A remote host refused to allow the network connection";
    break;
    case EFAULT:
        error = "The receive buffer pointer(s) point outside the process's address space.";
    case EPIPE:
        error = "The socket is shut down for writing, "
                "or the socket is connection-mode and is no longer connected.";
    break;
    case EINTR:
        error = "The receive was interrupted by delivery of a signal before any data were available.";
    break;
    case EINVAL:
        error = "Invalid argument passed.";
    break;
    case ENOMEM:
        error = "Could not allocate memory";
    break;
    }

    ERROR_LOG(error);
}
