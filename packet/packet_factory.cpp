#include "packet/packet_factory.h"

#include <unordered_map>

#include "common/macros.h"
#include "common/utils.h"
#include "packet/packet_id.pb.h"
#include "packet/connector.pb.h"

using ObjectFunc = std::function<proto::Packet*()>;
using proto::PacketId::CS_PING;
using proto::PacketId::SC_RET_PING;

static const std::unordered_map<proto::PacketId, ObjectFunc, EnumHash> object_funcs = {
    {CS_PING, []() -> proto::Packet* {return new proto::CS_Ping();}},
    {SC_RET_PING, []() -> proto::Packet* {return new proto::SC_RetPing();}},
};

PacketFactory::PacketFactory() {
}

PacketFactory::~PacketFactory() {
}

proto::Packet* const PacketFactory::create_packet(const uint32_t packet_id) {
    proto::PacketId e_id = static_cast<proto::PacketId>(packet_id);
    if (object_funcs.find(e_id) != object_funcs.end())
        return object_funcs.at(e_id)();
    return nullptr;
}

void PacketFactory::destory_packet(const proto::Packet* const packet) {
    SAFE_DELETE(packet);
}
