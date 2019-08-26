#include "listen/udp_listen_service.h"

#include <string.h>

#include "log/log.h"
#include "common/config/listen_flags.h"
#include "common/utils.h"
#include "packet/packet_id.pb.h"
#include "socket/socket.h"

UDPListenService::UDPListenService() {
    stop_ = false;
}

UDPListenService::~UDPListenService() {
    stop();
    close(listen_fd_);
}

void UDPListenService::stop() {
    ListenService::stop();
}

void UDPListenService::init_config() {
}

bool UDPListenService::init_socket() {
    DEBUG_LOG("listen port = %d", FLAGS_port);
    DEBUG_LOG("communication type = %s", FLAGS_communication_type.c_str());
    struct sockaddr_in listen_addr;
    listen_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
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

    epoll_fd_ = epoll_create(MAX_EVENT);
    if (epoll_fd_ == invalid_id) {
        ERROR_LOG("create epoll error!");
        close(listen_fd_);
        return false;
    }

    socket_ = std::make_shared<UDPSocket>();
    socket_->init(listen_fd_, epoll_fd_);

    DEBUG_LOG("create socket success!");
    return true;
}

void UDPListenService::run_logic() {
    ListenService::run_logic();

    thread_pool_ = std::make_shared<ThreadPool>(get_procnum());
    DEBUG_LOG("create thread pool success, thread number = %d", get_procnum());
}

void UDPListenService::main_loop() {
    struct epoll_event events[MAX_EVENT];
    while(!stop_) {
        int fds = epoll_wait(epoll_fd_, events, MAX_EVENT, 100);
        for (int i = 0; i < fds; ++i) {
            struct epoll_event& tmp_ev = events[i];
            if (tmp_ev.data.fd == listen_fd_) {
                UDPPacket udp_packet;
                int ret = socket_->exec_read(&udp_packet);

                if (ret > 0) {
                    // distributed in thread pool
                    thread_pool_->push(&UDPListenService::exec_packet, udp_packet);
                }
            }
            else if (tmp_ev.events & EPOLLIN) {
                DEBUG_LOG("epoll in");
            }
            else {
                DEBUG_LOG("epoll out");
            }
        }
    }
}

bool UDPListenService::test_recv() {
    struct sockaddr_in connection_addr;
    socklen_t socklen = sizeof(connection_addr);
    char message[1024] = {0};
    int ret = recvfrom(listen_fd_, message, sizeof(message), 0, (struct sockaddr*)&connection_addr, &socklen);
    if (ret > 0) {
        int port = ntohs(connection_addr.sin_port);
        DEBUG_LOG("receive message from %s, %d, content = %s",
            inet_ntoa(connection_addr.sin_addr),
            port,
            message);
        snprintf(message, sizeof(message), "%s%d", message, port);
        sendto(listen_fd_, message, strlen(message), 0, (struct sockaddr*)&connection_addr, socklen);
    }
    return true;
}

void UDPListenService::exec_packet(const UDPPacket& packet) {
    DEBUG_LOG("execute packet, packet_id = %d, content = %s",
        packet.packet_id_, packet.packet_msg_.c_str());

    proto::PacketId e_id = static_cast<proto::PacketId>(packet.packet_id_);
    if (e_id == proto::PacketId::CS_PING) {
        const char* msg = "receive your message, CSPING";
        socklen_t socklen = sizeof(packet.in_sock_);
        sendto(packet.socket_fd_, msg, strlen(msg), 0,
            (struct sockaddr*)&(packet.in_sock_), socklen);
    }
    else {
    }
}
