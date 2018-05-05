#ifndef GU_EDGENT_PEERDISCOVERYANOUNCEMENTPACKET_HPP
#define GU_EDGENT_PEERDISCOVERYANOUNCEMENTPACKET_HPP

#include <cstdint>
#include <cstring>
#include <utility>
#include <malloc.h>

#include "PeerDiscoveryPacketTypes.hpp"

namespace glasgow_ustream {

    class PeerDiscoveryReplyPacket {
        // Minimum packet size should be size of packet type, size of port number, sizeof stream_id_length, + 1 for minimum stream_id_size
        static const size_t min_packet_size = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint16_t) + 1;

        // Format of a reply packet
        uint8_t packet_type = PEER_DISCOVERY_REPLY_PACKET_TYPE;
        uint16_t port_number;
        uint32_t stream_id_length;
        std::string stream_id;
        // End Format

        bool valid = false;

        char *data_ptr = nullptr;

    public:

        /**
         * Constructs a PeerDiscoveryReplyPacket for a stream_id
         * @param stream_id the stream identifier to announce.
         */
        explicit PeerDiscoveryReplyPacket(uint16_t port_number, std::string stream_id) :
                port_number(port_number), stream_id(stream_id) {
            stream_id_length = (uint32_t) stream_id.length() + 1; // + 1 for \0
            valid = true;
        }

        /**
         * Deconstructs a supposed PeerDiscoveryReplyPacket from bytes.
         * @param data the raw bytes that were received.
         */
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

        std::string get_stream_id() {
            return stream_id;
        }

        /**
         * Should be called before calling get_stream_id() and get_port_number() in order to make sure a valid reply
         * packet was deserialized.
         * @return true if deserialization was successful.
         */
        bool is_valid() {
            return valid;
        }

        uint16_t get_port_number() {
            return port_number;
        }

        /**
         * Constructs a reploy packet in memory.
         * @return the raw bytes representing a reply packet that can be sent over the network.
         */
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

            memcpy(ptr, stream_id.c_str(), stream_id_length);

            return {packet_size, packet};
        }
    };
}

#endif //GU_EDGENT_PEERDISCOVERYANOUNCEMENTPACKET_HPP
