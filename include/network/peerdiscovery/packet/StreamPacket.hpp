#ifndef GU_EDGENT_STREAMPACKET_HPP
#define GU_EDGENT_STREAMPACKET_HPP


#include <cstring>
#include <utility>
#include <cstdlib>
#include <iostream>

#define STREAM_PACKET_START_DELIMITER "StreamPacketStart"
#define STREAM_PACKET_END_DELIMITER "StreamPacketEnd"
#define STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE strlen(STREAM_PACKET_START_DELIMITER) + 1 + sizeof(uint32_t)

class StreamPacket {

private:

    bool invalid = false;
    bool complete = false;

    char *current_data_ptr;
    uint32_t current_data_length;

    void *remainder = nullptr;

    void *packet_data = nullptr;
    uint32_t packet_length = 0;

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


    void encapsulate_data(std::pair<uint32_t, void*> data) {
        this->packet_length = delimiters_size + sizeof(uint32_t) + data.first;
        this->packet_data = malloc(this->packet_length);

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

        if (strncmp(start_delimiter, ptr, start_delimiter_size) != 0) {
            this->invalid = true;
            return;
        }
        ptr += start_delimiter_size;

        this->data_length = *((uint32_t *) ptr);
        ptr += sizeof(uint32_t);

        // We can now allocate the memory for the entire packet.
        this->packet_length = delimiters_size + sizeof(uint32_t) + data_length;
        this->packet_data = malloc(packet_length);
        this->current_data_ptr = (char *) packet_data;
        this->current_data_length = 0;
        add_data(std::pair<uint32_t, void *>(start_delimiter_size, (void *) start_delimiter));
        add_data(std::pair<uint32_t, void *>(sizeof(uint32_t), &this->data_length));


        this->data = (void *) ptr;
        if (data.first < this->packet_length) {
            // We have an incomplete data section
            uint32_t data_written_so_far = (uint32_t) (ptr - (char *) data.second);
            uint32_t num_bytes_to_copy = data.first - data_written_so_far;
            add_data(std::pair<uint32_t, void *>(num_bytes_to_copy, ptr));
            complete = false;
            return;
        } else if (data.first >= this->packet_length) {
            // We have at least one packet
            add_data(std::pair<uint32_t, void *>(this->data_length, ptr));
            ptr += this->data_length;
            add_data(std::pair<uint32_t, void *>(end_delimiter_size, (void *) end_delimiter));
            ptr += this->end_delimiter_size;

            // Sanity check
            if (data.first == this->packet_length && ptr != (char *) data.second + data.first) {
                std::cout << "Error: Invalid stream packet parsed." << std::endl;
                invalid = true;
                return;
            }
            complete = true;
            if (data.first > this->packet_length) {
                remainder = ptr;
            }
        }

    }

public:

    explicit StreamPacket (std::pair<uint32_t, void *> data, bool is_stream_packet) {
        if (is_stream_packet) {
            if (data.first < STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE ) {
                invalid = true;
                return;
            }
            char *data_ptr = (char *) data.second;
            bool data_starts_stream_packet = strncmp(start_delimiter, data_ptr, start_delimiter_size) == 0;

            if (data_starts_stream_packet) {
                unpack_stream_packet(data);
            } else {
                this->invalid = true;
            }
        } else {
            encapsulate_data(data);
        }
    }

    bool add_data(std::pair<uint32_t, void *> data) {
        if (complete) return true;
        memcpy(current_data_ptr, data.second, data.first);
        current_data_ptr += data.first;
        current_data_length += data.first;
        return is_complete();
    }

    std::pair<uint32_t, void *> get_packet() {
        void *packet_data_copy = malloc(this->packet_length);
        memcpy(packet_data_copy, this->packet_data, this->packet_length);
        return {packet_length, packet_data_copy};
    }

    std::pair<uint32_t, void *> get_stream_data() {
        return {data_length, data};
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


#endif //GU_EDGENT_STREAMPACKET_HPP
