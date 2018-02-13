#ifndef GU_EDGENT_PEERDISCOVERYANOUNCEMENTPACKET_HPP
#define GU_EDGENT_PEERDISCOVERYANOUNCEMENTPACKET_HPP

#include <cstdint>
#include <cstring>
#include <utility>
#include <malloc.h>

#include "network/peerdiscovery/packet/PeerDiscoveryPacketTypes.hpp"

class PeerDiscoveryReplyPacket {
    uint8_t packet_type = PEER_DISCOVERY_REPLY_PACKET_TYPE;
    uint16_t port_number;
    size_t stream_id_length;
    const char *stream_id;

    bool valid = false;

    char *data_ptr = nullptr;

    size_t min_packet_size = sizeof(uint8_t) + sizeof(size_t) + sizeof(uint16_t) + 1;

public:

    explicit PeerDiscoveryReplyPacket(std::pair<size_t, void*> data) {
        size_t data_length = data.first;
        if (data_length < min_packet_size) {
            std::cout << "A" << std::endl;
            return;
        }

        this->data_ptr = (char *) data.second;
        auto ptr = this->data_ptr;

        packet_type = *((uint8_t *) ptr);
        if (packet_type != PEER_DISCOVERY_REPLY_PACKET_TYPE) {
            std::cout << "B" << std::endl;
            return;
        }
        ptr += sizeof(uint8_t);

        port_number = *((uint16_t *) ptr);
        ptr += sizeof(uint16_t);

        stream_id_length = *((size_t *) ptr);
        if (data_length - sizeof(size_t) - sizeof(uint8_t) - sizeof(uint16_t) != stream_id_length) {
            std::cout << "C" << std::endl;
            return;
        }
        ptr += sizeof(size_t);

        stream_id = ptr;
        valid = true;
    }

    const char *get_stream_id() {
        return stream_id;
    }

    bool is_valid() {
        return valid;
    }

    uint16_t get_port_number() {
        return port_number;
    }

    std::pair<size_t, void*> get_packet_data() {
        size_t packet_size = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(size_t) + stream_id_length;
        auto *packet = (char *) malloc(packet_size);
        char *ptr = packet;

        *((uint8_t *) ptr) = packet_type;
        ptr += sizeof(uint8_t);

        *((uint16_t *) ptr) = port_number;
        ptr += sizeof(uint16_t);

        *((size_t *) ptr) = stream_id_length;
        ptr += sizeof(size_t);

        memcpy(ptr, stream_id, stream_id_length);

        return {packet_size, packet};
    }

    PeerDiscoveryReplyPacket(uint16_t port_number, const char *stream_id) :
            port_number(port_number), stream_id(stream_id) {
        stream_id_length = strlen(stream_id) + 1; // + 1 for \0
        valid = true;
    }
};

#endif //GU_EDGENT_PEERDISCOVERYANOUNCEMENTPACKET_HPP
