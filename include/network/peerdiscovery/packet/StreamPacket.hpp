#ifndef GU_EDGENT_STREAMPACKET_HPP
#define GU_EDGENT_STREAMPACKET_HPP


#include <cstring>
#include <utility>
#include <cstdlib>
#include <iostream>

#define STREAM_PACKET_START_DELIMITER "StreamPacketStart"
#define STREAM_PACKET_END_DELIMITER "StreamPacketEnd"
#define STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE strlen(STREAM_PACKET_START_DELIMITER) + 1 + sizeof(size_t)

class StreamPacket {

private:

    bool invalid = false;
    bool complete = false;

    char *current_data_ptr;
    size_t current_data_length;

    void *remainder = nullptr;

    void *packet_data = nullptr;
    size_t packet_length = 0;

    /** PACKET STRUCTURE **/
    const char *start_delimiter = STREAM_PACKET_START_DELIMITER;
    size_t data_length;
    void *data;
    const char *end_delimiter = STREAM_PACKET_END_DELIMITER;
    /** END PACKET STRUCTURE **/

    size_t start_delimiter_size = strlen(start_delimiter) + 1;
    size_t end_delimiter_size = strlen(end_delimiter) + 1;
    size_t delimiters_size = start_delimiter_size + end_delimiter_size;

    size_t MIN_STREAM_PACKET_SIZE = delimiters_size + sizeof(size_t) + 1; // Require data size of 1 byte


    void encapsulate_data(std::pair<size_t, void*> data) {
        this->packet_length = delimiters_size + sizeof(size_t) + data.first;
        this->packet_data = malloc(this->packet_length);

        char *ptr = (char *) this->packet_data;

        memcpy(ptr, start_delimiter, start_delimiter_size);
        ptr += start_delimiter_size;

        *((size_t *) ptr) = data.first;
        ptr += sizeof(size_t);

        memcpy(ptr, data.second, data.first);
        this->data = ptr;
        ptr += data.first;

        memcpy(ptr, end_delimiter, end_delimiter_size);
        ptr += end_delimiter_size;

        this->data_length = data.first;
        this->complete = true;
    }

    void unpack_stream_packet(std::pair<size_t, void *> data) {
        char *ptr = (char *) data.second;

        if (strncmp(start_delimiter, ptr, start_delimiter_size) != 0) {
            this->invalid = true;
            return;
        }
        ptr += start_delimiter_size;

        this->data_length = *((size_t *) ptr);
        ptr += sizeof(size_t);

        // We can now allocate the memory for the entire packet.
        this->packet_length = delimiters_size + sizeof(size_t) + data_length;
        this->packet_data = malloc(packet_length);
        this->current_data_ptr = (char *) packet_data;
        this->current_data_length = 0;
        add_data(std::pair<size_t, void *>(start_delimiter_size, (void *) start_delimiter));
        add_data(std::pair<size_t, void *>(sizeof(size_t), &this->data_length));


        this->data = (void *) ptr;
        if (data.first < this->packet_length) {
            // We have an incomplete data section
            size_t data_written_so_far = ptr - (char *) data.second;
            size_t num_bytes_to_copy = data.first - data_written_so_far;
            add_data(std::pair<size_t, void *>(num_bytes_to_copy, ptr));
            complete = false;
            return;
        } else if (data.first >= this->packet_length) {
            // We have at least one packet
            add_data(std::pair<size_t, void *>(this->data_length, ptr));
            ptr += this->data_length;
            add_data(std::pair<size_t, void *>(end_delimiter_size, (void *) end_delimiter));
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

    explicit StreamPacket (std::pair<size_t, void *> data, bool is_stream_packet) {
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

    bool add_data(std::pair<size_t, void *> data) {
        if (complete) return true;
        memcpy(current_data_ptr, data.second, data.first);
        current_data_ptr += data.first;
        current_data_length += data.first;
        return is_complete();
    }

    std::pair<size_t, void *> get_packet() {
        void *packet_data_copy = malloc(this->packet_length);
        memcpy(packet_data_copy, this->packet_data, this->packet_length);
        return {packet_length, packet_data_copy};
    }

    std::pair<size_t, void *> get_stream_data() {
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
