#pragma once

#include "packet/packet.h"
#include "packet/connector.pb.h"

class Connector;

class User {
public:
    explicit User(Connector& cor);
    ~User();

    void exec_packet(const uint32_t packet_id, const proto::Packet& packet);
    void exec_packet(const proto::CS_Ping& packet );

private:
    void send_packet(const int packet_id, const proto::Packet& packet);
    Connector& connector_;
};
