#pragma once

#include "common/macros.h"
#include "packet/packet.h"

class PacketFactory {
public:
    ~PacketFactory();
    proto::Packet* const create_packet(const uint32_t packet_id);
    void destory_packet(const proto::Packet* const packet);

private:

    SINGLETON_DECLARE(PacketFactory);
};
