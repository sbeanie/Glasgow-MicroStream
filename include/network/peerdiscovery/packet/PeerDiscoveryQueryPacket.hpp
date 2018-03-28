#ifndef GU_EDGENT_PEERDISCOVERYQUERYPACKET_HPP
#define GU_EDGENT_PEERDISCOVERYQUERYPACKET_HPP

#include <cstdint>
#include <cstring>
#include <utility>
#include <malloc.h>

#include "PeerDiscoveryPacketTypes.hpp"

namespace glasgow_ustream {

    class PeerDiscoveryQueryPacket {
        static const uint32_t min_packet_size = sizeof(uint8_t) + sizeof(uint32_t) + 1;

        // Format of a query packet
        uint8_t packet_type = PEER_DISCOVERY_QUERY_PACKET_TYPE;
        uint32_t stream_id_length;
        const char *stream_id;
        // End format

        bool valid = false;

        char *data_ptr = nullptr;


    public:
        /**
         * Constructs a PeerDiscoveryQueryPacket for a stream_id
         * @param stream_id the stream identifier to query for.
         */
        explicit PeerDiscoveryQueryPacket(const char *stream_id) : stream_id(stream_id) {
            stream_id_length = (uint32_t) strlen(stream_id) + 1; // + 1 for \0
            valid = true;
        }

        /**
         * Deconstructs a supposed PeerDiscoveryQueryPacket from bytes.
         * @param data the raw bytes that were received.
         */
        explicit PeerDiscoveryQueryPacket(std::pair<uint32_t, void *> data) {
            uint32_t data_length = data.first;
            if (data_length < min_packet_size) {
                std::cerr << "Minimum query packet length not met.  Discarding..." << std::endl;
                return;
            }

            this->data_ptr = (char *) data.second;
            auto ptr = this->data_ptr;

            packet_type = *((uint8_t *) ptr);
            if (packet_type != PEER_DISCOVERY_QUERY_PACKET_TYPE) {
                std::cerr << "Data provided to parse is not a query packet. Discarding..." << std::endl;
                return;
            }
            ptr += sizeof(uint8_t);

            stream_id_length = *((uint32_t *) ptr);
            if (data_length - sizeof(uint8_t) - sizeof(uint32_t) != stream_id_length) {
                std::cerr << "Malformed query packet.  Discarding..." << std::endl;
                return;
            }
            ptr += sizeof(uint32_t);

            stream_id = ptr;
            valid = true;
        }

        const char *get_stream_id() {
            return stream_id;
        }

        /**
         * Should be called before calling get_stream_id() in order to make sure a valid query packet was deserialized.
         * @return true if deserialization was successful.
         */
        bool is_valid() {
            return valid;
        }

        /**
         * Constructs a query packet in memory.
         * @return the raw bytes representing a query packet that can be sent over the network
         */
        std::pair<size_t, void *> get_packet_data() {
            uint32_t packet_size = sizeof(uint8_t) + sizeof(uint32_t) + stream_id_length;
            auto *packet = (char *) malloc(packet_size);
            char *ptr = packet;

            *((uint8_t *) ptr) = packet_type;
            ptr += sizeof(uint8_t);

            *((uint32_t *) ptr) = stream_id_length;
            ptr += sizeof(uint32_t);

            memcpy(ptr, stream_id, stream_id_length);

            return {packet_size, packet};
        }
    };
}
#endif //GU_EDGENT_PEERDISCOVERYQUERYPACKET_HPP
