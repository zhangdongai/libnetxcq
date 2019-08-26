#include <csignal>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <iostream>
#include <string.h>
#include <string>
#include <vector>

#include "packet/packet_id.pb.h"
#include "packet/connector.pb.h"

std::vector<int> g_sockets;

static constexpr int PACKET_BYTES_SIZE = sizeof(uint32_t);
static constexpr int PACKET_ID_SIZE = sizeof(uint16_t);
static constexpr int PACKET_HEADER_SIZE = PACKET_BYTES_SIZE + PACKET_ID_SIZE;

static void signal_callback(int signal_id) {
    std::cout << "catch signal" << std::endl;
    for (const auto& e : g_sockets)
        close(e);
    sleep(1);
    exit(0);
}

void udp_send() {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1) {
        std::cout << "create socket failed" << std::endl;
        return;
    }
    
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9527);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    proto::CS_Ping packet;
    packet.set_param("##new message##");
    uint32_t packet_size = packet.ByteSize();
    packet_size = packet_size + PACKET_HEADER_SIZE;
    uint32_t tmp_packet_size = ntohl(packet_size);
    char packet_size_s[64] = {0};
    memcpy(packet_size_s, (char*)&tmp_packet_size, PACKET_BYTES_SIZE);
    uint16_t packet_id = ntohs(static_cast<uint32_t>(proto::PacketId::CS_PING));
    char packet_id_s[64] = {0};
    memcpy(packet_id_s, (char*)&packet_id, PACKET_ID_SIZE);
    char buff[1024] = {0};
    packet.SerializeToArray(buff, sizeof(buff));
    char message[2048] = {0};
    memcpy(message, packet_size_s, PACKET_BYTES_SIZE);
    memcpy(message + PACKET_BYTES_SIZE, packet_id_s, PACKET_BYTES_SIZE);
    memcpy(message + PACKET_HEADER_SIZE, buff, strlen(buff));

    int ret = sendto(socket_fd, message, packet_size, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    if (ret == -1) { 
        std::cout << "udp send failed" << std::endl;
        return;
    }
    std::cout << "udp send success! " << ret << std::endl;

    memset(message, 0, sizeof(message));
    ret = recvfrom(socket_fd, message, sizeof(message), 0, nullptr, nullptr);
    if (ret == -1) {
        std::cout << "udp receive failed" << std::endl;
        return;
    }
    std::cout << "udp recv success! content = " << message << std::endl;
}

void create_socket() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        std::cout << "create socket failed" << std::endl;
        return;
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9527);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        std::cout << "connect failed" << std::endl;
        return;
    }
    std::cout << "create socket success!" << std::endl;
    g_sockets.push_back(socket_fd);
}

void send_msg() {
    for(const auto& e : g_sockets) {
        proto::CS_Ping packet;
        packet.set_param("##new message##");
        uint32_t packet_size = packet.ByteSize();
        packet_size = packet_size + PACKET_HEADER_SIZE;
        uint32_t tmp_packet_size = ntohl(packet_size);
        char packet_size_s[64] = {0};
        memcpy(packet_size_s, (char*)&tmp_packet_size, PACKET_BYTES_SIZE);
        uint16_t packet_id = ntohs(static_cast<uint32_t>(proto::PacketId::CS_PING));
        char packet_id_s[64] = {0};
        memcpy(packet_id_s, (char*)&packet_id, PACKET_ID_SIZE);
        char buff[1024] = {0};
        packet.SerializeToArray(buff, sizeof(buff));
        char message[2048] = {0};
        memcpy(message, packet_size_s, PACKET_BYTES_SIZE);
        memcpy(message + PACKET_BYTES_SIZE, packet_id_s, PACKET_BYTES_SIZE);
        memcpy(message + PACKET_HEADER_SIZE, buff, strlen(buff));
//        const char* buff = "1234567";
        send(e, message, packet_size, 0);
    }
}

void recv_msg() {
    for (const auto& e : g_sockets) {
        char header[64] = {0};
        int n = recv(e, header, PACKET_HEADER_SIZE, 0);
        if (n <= 0)
            continue;

        uint32_t packet_size = 0;
        memcpy(&packet_size, header + 0, PACKET_BYTES_SIZE);
        packet_size = htonl(packet_size);
        uint16_t packet_id = 0;
        memcpy(&packet_id, header + PACKET_BYTES_SIZE, PACKET_ID_SIZE);
        packet_id = htons(packet_id);

        char buff[1024] = {0};
        recv(e, buff, packet_size - PACKET_HEADER_SIZE, 0);
        std::cout << "receive message from server. packet_size = "
                  << packet_size
                  << ", packet_id = "
                  << packet_id
                  << ", content = "
                  << buff
                  << std::endl;
    }
}

int main(int, char**) {
    signal(SIGABRT, signal_callback);
    signal(SIGINT, signal_callback);

    while (1) {
        char c;
        std::cin >> c;
        switch(c) {
        case 'c':
            create_socket();
        break;
        case 's':
            send_msg();
        break;
        case 'r':
            recv_msg();
        break;
        case 'u':
            udp_send();
        break;
        }
    }
    return 0;
}
