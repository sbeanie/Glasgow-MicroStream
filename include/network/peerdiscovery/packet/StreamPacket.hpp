#ifndef GU_EDGENT_STREAMPACKET_HPP
#define GU_EDGENT_STREAMPACKET_HPP


#include <cstring>
#include <utility>
#include <cstdlib>
#include <iostream>

#define STREAM_PACKET_START_DELIMITER "StreamPacketStart"
#define STREAM_PACKET_END_DELIMITER "StreamPacketEnd"
#define STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE strlen(STREAM_PACKET_START_DELIMITER) + 1 + sizeof(uint32_t)

namespace glasgow_ustream {

    /**
     * This class acts as a means to encapsulate stream values.  It provides a way to delimit stream values on a TCP stream.
     */
    class StreamPacket {

    private:
        // Set to true when data received is incorrectly formatted.
        bool invalid = false;

        // Set to true when the stream packet has not fully been received yet, but it is still structured ok.
        bool complete = false;

        // These are used when an incomplete stream packet needs more data.
        char *current_data_ptr;
        uint32_t current_packet_length;

        // Points to extra data that is not a part of this Stream Packet if a stream packet was constructed with too much data
        void *remainder = nullptr;

        // Points to the allocated memory for a stream packet
        void *packet_data = nullptr;
        uint32_t complete_packet_length = 0;

        /** PACKET STRUCTURE **/
        const char *start_delimiter = STREAM_PACKET_START_DELIMITER;
        uint32_t data_length;
        void *data;
        const char *end_delimiter = STREAM_PACKET_END_DELIMITER;
        /** END PACKET STRUCTURE **/

        uint32_t start_delimiter_size = (uint32_t) strlen(start_delimiter) + 1;
        uint32_t end_delimiter_size = (uint32_t) strlen(end_delimiter) + 1;
        uint32_t delimiters_size = start_delimiter_size + end_delimiter_size;

        uint32_t MIN_STREAM_PACKET_SIZE = delimiters_size + sizeof(uint32_t) + 1; // Require data size of 1 byte


        void encapsulate_data(std::pair<uint32_t, void *> data) {
            // This stream packet will include the delimiters, the data's length and the data itself.
            this->complete_packet_length = delimiters_size + sizeof(uint32_t) + data.first;
            this->packet_data = malloc(this->complete_packet_length);

            char *ptr = (char *) this->packet_data;

            memcpy(ptr, start_delimiter, start_delimiter_size);
            ptr += start_delimiter_size;

            *((uint32_t *) ptr) = data.first;
            ptr += sizeof(uint32_t);

            memcpy(ptr, data.second, data.first);
            this->data = ptr;
            ptr += data.first;

            memcpy(ptr, end_delimiter, end_delimiter_size);
            ptr += end_delimiter_size;

            this->data_length = data.first;
            this->complete = true;
        }

        void unpack_stream_packet(std::pair<uint32_t, void *> data) {
            char *ptr = (char *) data.second;

            // Check the data starts with the stream packet start delimiter
            if (strncmp(start_delimiter, ptr, start_delimiter_size) != 0) {
                this->invalid = true;
                return;
            }
            ptr += start_delimiter_size;

            // Get the data length
            this->data_length = *((uint32_t *) ptr);
            ptr += sizeof(uint32_t);

            // We can now allocate the memory for the entire packet.
            this->complete_packet_length = delimiters_size + sizeof(uint32_t) + data_length;
            this->packet_data = malloc(complete_packet_length);

            // Get ready for adding the data to the allocated memory
            this->current_data_ptr = (char *) packet_data;
            this->current_packet_length = 0;

            add_data(std::pair<uint32_t, void *>(start_delimiter_size, (void *) start_delimiter));
            add_data(std::pair<uint32_t, void *>(sizeof(uint32_t), &this->data_length));

            // ptr currently points to the start of the data section
            this->data = (void *) ptr;

            if (data.first < this->complete_packet_length) {
                // We have less data than the Stream Packet declares
                uint32_t data_written_so_far = (uint32_t) (ptr - (char *) data.second);
                uint32_t num_bytes_to_copy = data.first - data_written_so_far;
                add_data(std::pair<uint32_t, void *>(num_bytes_to_copy, ptr));
                complete = false;
                return;
            } else if (data.first >= this->complete_packet_length) {
                // We have at least one packet
                add_data(std::pair<uint32_t, void *>(this->data_length, ptr));
                ptr += this->data_length;
                add_data(std::pair<uint32_t, void *>(end_delimiter_size, (void *) ptr));
                ptr += this->end_delimiter_size;

                // Sanity check
                if (data.first == this->complete_packet_length && ptr != (char *) data.second + data.first) {
                    std::cerr << "Invalid stream packet parsed." << std::endl;
                    invalid = true;
                    return;
                }

                current_packet_length = complete_packet_length;
                current_data_ptr = (char *) packet_data + complete_packet_length;
                check_if_done();
                if (data.first > this->complete_packet_length) {
                    remainder = ptr;
                }
            }
        }

        void check_if_done() {
            if (current_packet_length == complete_packet_length) {
                // Check end delimiter
                char *ptr = current_data_ptr - end_delimiter_size;
                if (strncmp(ptr, STREAM_PACKET_END_DELIMITER, end_delimiter_size) == 0) {
                    this->complete = true;
                    return;
                } else {
                    this->complete = false;
                    this->invalid = true;
                    return;
                }
            } else if (current_packet_length > complete_packet_length) {
                this->invalid = true;
                // Something seriously went wrong if this happens.
                std::cerr << "Too much data was added to a stream packet." << std::endl;
                exit(-5);
            } else {
                this->complete = false;
                return;
            }
        }

    public:

        explicit StreamPacket(std::pair<uint32_t, void *> data, bool is_stream_packet) {
            if (is_stream_packet) {
                if (data.first < STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE) {
                    // A stream packet must at least be the minimum construction size.
                    invalid = true;
                    return;
                }

                // Check that the data starts with the Stream Packet start delimiter
                bool data_starts_stream_packet =
                        strncmp(start_delimiter, (char *) data.second, start_delimiter_size) == 0;

                if (!data_starts_stream_packet) {
                    this->invalid = true;
                    return;
                }

                // It should now be ok to start unpacking the stream packet.
                unpack_stream_packet(data);
            } else {
                encapsulate_data(data);
            }
        }

        bool add_data(std::pair<uint32_t, void *> data) {
            if (complete) return true;
            size_t remaining_bytes = this->complete_packet_length - this->current_packet_length;
            if (data.first >= remaining_bytes) {
                memcpy(current_data_ptr, data.second, remaining_bytes);
                current_data_ptr += remaining_bytes;
                current_packet_length += remaining_bytes;
                if (data.first > remaining_bytes) {
                    remainder = (char *) data.second + remaining_bytes;
                }
            } else {
                memcpy(current_data_ptr, data.second, data.first);
                current_data_ptr += data.first;
                current_packet_length += data.first;
            }
            this->check_if_done();
            return is_complete();
        }

        std::pair<uint32_t, void *> get_packet() {
            void *packet_data_copy = malloc(this->complete_packet_length);
            memcpy(packet_data_copy, this->packet_data, this->complete_packet_length);
            return {complete_packet_length, packet_data_copy};
        }

        std::pair<uint32_t, void *> get_stream_data() {
            void *aligned_data_ptr = malloc((size_t) data_length);
            memcpy(aligned_data_ptr, data, (size_t) data_length);
            return {data_length, aligned_data_ptr};
        }

        bool is_invalid() {
            return this->invalid;
        }

        bool is_complete() {
            return this->complete;
        }

        void *get_remainder() {
            return this->remainder;
        }

        ~StreamPacket() {
            if (this->packet_data != nullptr) free(this->packet_data);
        }
    };

}

#endif //GU_EDGENT_STREAMPACKET_HPP
