#include "user/user.h"

#include "connector/connector.h"
#include "log/log.h"
#include "packet/packet_id.pb.h"

User::User(Connector& cor)
  : connector_(cor) {
}

User::~User() {}

void User::exec_packet(const uint32_t packet_id, const proto::Packet& packet) {
    proto::PacketId e_id = static_cast<proto::PacketId>(packet_id);
    if (e_id == proto::PacketId::CS_PING)
        exec_packet(static_cast<const proto::CS_Ping&>(packet));
}

void User::exec_packet(const proto::CS_Ping& packet) {
    DEBUG_LOG("receive packet CS_Ping, param = %s", packet.param().c_str());

    proto::SC_RetPing ret_packet;
    ret_packet.set_ret_param("received your message ##new message##");
    send_packet(static_cast<int>(proto::PacketId::SC_RET_PING), ret_packet);
}

void User::send_packet(const int packet_id, const proto::Packet& packet) {
    connector_.send_packet(packet_id, packet);
}
