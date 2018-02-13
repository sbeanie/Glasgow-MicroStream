#ifndef GU_EDGENT_PEERDISCOVERYPACKETTYPES_HPP
#define GU_EDGENT_PEERDISCOVERYPACKETTYPES_HPP

#define PEER_DISCOVERY_UNKNOWN_PACKET_TYPE 0
#define PEER_DISCOVERY_QUERY_PACKET_TYPE 1
#define PEER_DISCOVERY_REPLY_PACKET_TYPE 2


class PeerDiscoveryPacket {

public:

    static uint8_t get_packet_type(void *data) {
        uint8_t packet_type = *((uint8_t *) data);
        if (packet_type >= PEER_DISCOVERY_QUERY_PACKET_TYPE && packet_type <= PEER_DISCOVERY_REPLY_PACKET_TYPE) {
            return packet_type;
        } else {
            return PEER_DISCOVERY_UNKNOWN_PACKET_TYPE;
        }
    }
};

#include "network/peerdiscovery/packet/PeerDiscoveryReplyPacket.hpp"
#include "network/peerdiscovery/packet/PeerDiscoveryQueryPacket.hpp"


#endif //GU_EDGENT_PEERDISCOVERYPACKETTYPES_HPP
