#include "connector/connector.h"

#include <chrono>

#include "log/log.h"
#include "packet/packet.h"
#include "packet/packet_factory.h"
#include "user/user.h"

// Packet content size(4 bytes), and Packet ID(2 bytes)
static constexpr int PACKET_BYTES_SIZE = sizeof(uint32_t);
static constexpr int PACKET_ID_SIZE = sizeof(uint16_t);
static constexpr int PACKET_HEADER_SIZE = PACKET_BYTES_SIZE + PACKET_ID_SIZE;

Connector::Connector()
    : user_(*this) {
}

Connector::~Connector() {
}

bool Connector::create_socket(const int socket_fd, const int epoll_fd) {
    socket_ = std::make_shared<Socket>();
    return socket_->init(socket_fd, epoll_fd);
}

void Connector::close_socket() {
    socket_->close_socket();
    socket_.reset();
}

int Connector::exec_read() {
    return socket_->exec_read();
}

int Connector::exec_write() {
    return socket_->exec_write();
}

void Connector::exec_parse() {
    if (0) {
        auto tmp_func = [this]() {
            char buff[64] = {0};
            this->get_socket()->fetch_content(buff, 7);
            DEBUG_LOG("fetch content, content = %s", buff);
        };
        tmp_func();
        return;
    }
    char packet_header[PACKET_HEADER_SIZE] = {0};
    socket_->fetch_header(packet_header, PACKET_HEADER_SIZE);

    uint32_t packet_size = 0;
    memcpy(&packet_size, &packet_header[0], sizeof(uint32_t));
    packet_size = htonl(packet_size);

    uint16_t packet_id = 0;
    memcpy(&packet_id, &packet_header[sizeof(uint32_t)], sizeof(uint16_t));
    packet_id = htons(packet_id);

    uint32_t content_size = packet_size - PACKET_HEADER_SIZE;
    // ###will be replaced by tcmalloc at later###
    char* packet_cont = new char[content_size];
    socket_->fetch_content(packet_cont, content_size);
    proto::Packet* packet = PacketFactory::instance()->create_packet(packet_id);
    packet->ParseFromArray(packet_cont, content_size);
    user_.exec_packet(packet_id, *packet);
    PacketFactory::instance()->destory_packet(packet);;
    delete[] packet_cont;
}

void Connector::send_packet(const int packet_id, const proto::Packet& packet) {
    uint32_t packet_size = packet.ByteSize();
    uint32_t total_packet_size = packet_size + PACKET_HEADER_SIZE;
    uint32_t packet_size_n = ntohl(total_packet_size);
    char packet_size_s[64] = {0};
    memcpy(packet_size_s, &packet_size_n, PACKET_BYTES_SIZE);

    uint16_t packet_id_n = ntohs(packet_id);
    char packet_id_s[64] = {0};
    memcpy(packet_id_s, &packet_id_n, PACKET_ID_SIZE);

    char* buff = new char[packet_size];
    memset(buff, 0, packet_size);
    packet.SerializeToArray(buff, packet_size);

    char* message = new char[total_packet_size];
    memset(message, 0, total_packet_size);
    memcpy(message, packet_size_s, PACKET_BYTES_SIZE);
    memcpy(message + PACKET_BYTES_SIZE, packet_id_s, PACKET_ID_SIZE);
    memcpy(message + PACKET_HEADER_SIZE, buff, packet_size);

    socket_->write(message, total_packet_size);

    SAFE_DELETE(buff);
    SAFE_DELETE(message);
}
