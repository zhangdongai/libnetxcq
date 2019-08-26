#include "socket/udp_socket.h"

#include <string.h>

#include "log/log.h"
#include "packet/connector.pb.h"

// Packet content size(4 bytes), and Packet ID(2 bytes)
static constexpr int PACKET_BYTES_SIZE = sizeof(uint32_t);
static constexpr int PACKET_ID_SIZE = sizeof(uint16_t);
static constexpr int PACKET_HEADER_SIZE = PACKET_BYTES_SIZE + PACKET_ID_SIZE;

UDPSocket::UDPSocket() {
}

UDPSocket::~UDPSocket() {
    DEBUG_LOG("destruction called, socket_fd = %d", socket_fd_);
    close_socket();
}

void UDPSocket::close_socket() {
    if (socket_fd_ != invalid_id) {
        close(socket_fd_);
        socket_fd_ = invalid_id;
    }
}

bool UDPSocket::init(const int socket_fd, const int epoll_fd) {
    socket_fd_ = socket_fd;
    epoll_fd_ = epoll_fd;

    event_.data.fd = socket_fd_;
    // use Level Trigger as default
    // catch EPOLLIN & EPOLLOUT
    event_.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_fd_, &event_);

    DEBUG_LOG("create socket success, socketid = %d", socket_fd);
    return true;
}

int UDPSocket::exec_read(UDPPacket* const udp_packet) {
    char packet[20480] = {0};
    struct sockaddr_in in_sock;
    socklen_t socklen = sizeof(in_sock);
    int received_bytes = recvfrom(socket_fd_, packet, sizeof(packet),
                                  0, (struct sockaddr*)&in_sock, &socklen);
    if (received_bytes <= 0) {
        ERROR_LOG("received header failed, received length = %d",
            received_bytes);
        return 0;
    }

    int packet_size = 0;
    memcpy(&packet_size, &packet[0], PACKET_BYTES_SIZE);
    packet_size = htonl(packet_size);

    int packet_id = 0;
    memcpy(&packet_id, &packet[PACKET_BYTES_SIZE], PACKET_ID_SIZE);
    packet_id = htons(packet_id);

    int port = ntohs(in_sock.sin_port);
    DEBUG_LOG("received packet from %s, %d, packet_id = %d, content = %s",
              inet_ntoa(in_sock.sin_addr),
              port,
              packet_id,
              packet + PACKET_HEADER_SIZE);

    udp_packet->socket_fd_ = socket_fd_;
    udp_packet->in_sock_ = in_sock;
    udp_packet->packet_id_ = packet_id;
    udp_packet->packet_msg_ = (packet + PACKET_HEADER_SIZE);

    return received_bytes;
}

int UDPSocket::exec_write() {
    return 0;
}
