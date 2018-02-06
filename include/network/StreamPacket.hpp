#ifndef GU_EDGENT_STREAMPACKET_H
#define GU_EDGENT_STREAMPACKET_H

#include <cstring>


class StreamPacket {

private:
    size_t stream_id_length;
    size_t packet_length, data_length;
    char *stream_id;
    void *stream_data;

    bool valid;

public:
    StreamPacket (std::pair<size_t, void*> stream_packet_data) {
        this->packet_length = stream_packet_data.first;
        char *packet_data = (char *) stream_packet_data.second; // Treat it as bytes

        size_t stream_id_size = *((size_t*) packet_data);

        if (stream_id_size > packet_length - sizeof(size_t) - 1) { // Allow for a minimum data size of 1.
            valid = false;
            return;
        }

        char *stream_id_start = packet_data + sizeof(size_t);

        if (stream_id_start[stream_id_size - 1] != '\0') {
            valid = false;
            return;
        }

        char *data_start = stream_id_start + stream_id_size;

        this->data_length = stream_packet_data.first - sizeof(size_t) - stream_id_size;

        stream_id = (char *) malloc(stream_id_size);
        memcpy(stream_id, stream_id_start, stream_id_size);

        stream_data = malloc(data_length);
        memcpy(stream_data, data_start, data_length);

        valid = true;
    }

    StreamPacket(const char *stream_id, std::pair<size_t, void*> stream_data) {
        this->stream_id_length = strlen(stream_id) + 1; // + 1 for \0
        this->data_length = stream_data.first;
        this->packet_length = stream_id_length + data_length + sizeof(size_t);

        this->stream_id = strdup(stream_id);

        this->stream_data = malloc(stream_data.first);
        memcpy(this->stream_data, stream_data.second, stream_data.first);

        valid = true; // This constructor will always be valid.
    }

    ~StreamPacket () {
        free(stream_id);
        free(stream_data);
    }

    char* get_stream_id() {
        return stream_id;
    }

    void* get_stream_data() {
        return stream_data;
    }

    size_t get_data_size() {
        return data_length;
    }

    bool is_valid() {
        return valid;
    }

    std::pair<size_t, void *> buildPacket() {
        auto *ptr = (char *) malloc(packet_length);
        auto *start_ptr = ptr;

        *((size_t *) ptr) = stream_id_length;
        ptr += sizeof(size_t);

        memcpy(ptr, stream_id, stream_id_length);
        ptr += stream_id_length;

        memcpy(ptr, stream_data, data_length);
        ptr += data_length;

        return std::pair<size_t, void*>(packet_length, start_ptr);
    };
};


#endif //GU_EDGENT_STREAMPACKET_H
