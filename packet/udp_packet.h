#include <string>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

// udp connection object, use to send packet to client
struct UDPPacket {
    int socket_fd_;
    struct sockaddr_in in_sock_;
    int packet_id_;
    std::string packet_msg_;
};
