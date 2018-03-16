#ifndef GU_EDGENT_PEERDISCOVERYANOUNCEMENTPACKET_HPP
#define GU_EDGENT_PEERDISCOVERYANOUNCEMENTPACKET_HPP

#include <cstdint>
#include <cstring>
#include <utility>
#include <malloc.h>

#include "PeerDiscoveryPacketTypes.hpp"

namespace NAMESPACE_NAME {

    class PeerDiscoveryReplyPacket {
        uint8_t packet_type = PEER_DISCOVERY_REPLY_PACKET_TYPE;
        uint16_t port_number;
        uint32_t stream_id_length;
        const char *stream_id;

        bool valid = false;

        char *data_ptr = nullptr;

        // Minimum packet size should be size of packet type, size of port number, sizeof stream_id_length, + 1 for minimum stream_id_size
        size_t min_packet_size = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint16_t) + 1;

    public:

        explicit PeerDiscoveryReplyPacket(std::pair<uint32_t, void *> data) {
            uint32_t data_length = data.first;
            if (data_length < min_packet_size) {
                std::cerr << "Minimum reply packet length not met.  Discarding..." << std::endl;
                return;
            }

            this->data_ptr = (char *) data.second;
            auto ptr = this->data_ptr;

            packet_type = *((uint8_t *) ptr);
            if (packet_type != PEER_DISCOVERY_REPLY_PACKET_TYPE) {
                std::cerr << "Data provided to parse is not a reply packet. Discarding..." << std::endl;
                return;
            }
            ptr += sizeof(uint8_t);

            port_number = *((uint16_t *) ptr);
            ptr += sizeof(uint16_t);

            stream_id_length = *((uint32_t *) ptr);
            if (data_length - sizeof(uint32_t) - sizeof(uint8_t) - sizeof(uint16_t) != stream_id_length) {
                std::cerr << "Malformed reply packet.  Discarding..." << std::endl;
                return;
            }
            ptr += sizeof(uint32_t);

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

        std::pair<size_t, void *> get_packet_data() {
            size_t packet_size = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t) + stream_id_length;
            auto *packet = (char *) malloc(packet_size);
            char *ptr = packet;

            *((uint8_t *) ptr) = packet_type;
            ptr += sizeof(uint8_t);

            *((uint16_t *) ptr) = port_number;
            ptr += sizeof(uint16_t);

            *((uint32_t *) ptr) = stream_id_length;
            ptr += sizeof(uint32_t);

            memcpy(ptr, stream_id, stream_id_length);

            return {packet_size, packet};
        }

        PeerDiscoveryReplyPacket(uint16_t port_number, const char *stream_id) :
                port_number(port_number), stream_id(stream_id) {
            stream_id_length = strlen(stream_id) + 1; // + 1 for \0
            valid = true;
        }
    };
}

#endif //GU_EDGENT_PEERDISCOVERYANOUNCEMENTPACKET_HPP
