#include "PeerListener.hpp"

namespace glasgow_ustream {

    void PeerListener::run() {
        ssize_t recv_bytes_received;
        struct sockaddr_in serv_addr;
        char msgbuf[GU_EDGENT_NETWORK_BUFFER_SIZE] = {0};

        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            stop();
            return;
        }

        memset(&serv_addr, '0', sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port_number);
        serv_addr.sin_addr = source_addr;

        std::cout << "[" << stream_id << "] Connecting to: " << inet_ntoa(source_addr) << ":" << port_number
                  << std::endl;

        if (connect(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            printf("\nConnection Failed \n");
            stop();
            return;
        }

        // This is used to contain header data for a StreamPacket that can't yet be used to construct a partial StreamPacket
        // as there is not yet enough data for the StreamPacket to know how much memory to allocate for itself.
        char incomplete_header[STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE] = {1};
        size_t incomplete_header_used_bytes = 0;

        StreamPacket *streamPacket = nullptr;

        while (should_run) {
            if ((recv_bytes_received = read(socket_fd, msgbuf, GU_EDGENT_NETWORK_BUFFER_SIZE)) < 0) {
                stop();
                return;
            }
            if (!should_run) {
                // We might have been interrupted during our read
                stop();
                return;
            }

            // Safe cast as -1 if statement catches a failure.
            // Casting to uint32_t is safe as buffer size is smaller.
            auto num_bytes_to_process = (uint32_t) recv_bytes_received;
            if (should_process_data) {
                void *data;
                if (incomplete_header_used_bytes != 0) {
                    // Prepend existing data onto current data to meet minimum length requirements.
                    data = malloc(num_bytes_to_process + incomplete_header_used_bytes);

                    memcpy(data, incomplete_header, incomplete_header_used_bytes);
                    memcpy((char *) data + incomplete_header_used_bytes, msgbuf, num_bytes_to_process);

                    num_bytes_to_process += incomplete_header_used_bytes;
                    incomplete_header_used_bytes = 0;
                } else {
                    // There is no incomplete header data we need to prepend.
                    data = malloc(num_bytes_to_process);
                    memcpy(data, msgbuf, num_bytes_to_process);
                }

                void *remainder = data;

                while (true) {
                    uint32_t remaining_bytes = (uint32_t) (num_bytes_to_process - ((char *) remainder - (char *) data));
                    if (streamPacket == nullptr) {
                        // We need to construct a new StreamPacket as we have just completed the one before.
                        if (remaining_bytes < STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE) {
                            // We don't yet have enough bytes to construct a StreamPacket, save remaining data until we do.
                            memcpy(incomplete_header, remainder, remaining_bytes);
                            incomplete_header_used_bytes = remaining_bytes;
                            remainder = nullptr;
                        } else {
                            // We have enough data to construct a StreamPacket (not necessarily completely).
                            streamPacket = new StreamPacket({remaining_bytes, remainder}, true);
                            if (streamPacket->is_complete()) {
                                this->streamPacketDataReceiver->receive(streamPacket->get_stream_data());
                                remainder = streamPacket->get_remainder();
                                delete (streamPacket);
                                streamPacket = nullptr;
                            } else {
                                // If the data we had did not complete the stream packet, there will be no remainder or the
                                // data we used was invalid.
                                if (streamPacket->is_invalid()) {
                                    // This means we are out of sync with the sender.  We should disconnect.
                                    stop();
                                    return;
                                }
                            }
                        }
                    } else {
                        // We need to finalize a partial StreamPacket we have created.
                        streamPacket->add_data({remaining_bytes, remainder});
                        if (streamPacket->is_complete()) {
                            this->streamPacketDataReceiver->receive(streamPacket->get_stream_data());
                            remainder = streamPacket->get_remainder();
                            delete (streamPacket);
                            streamPacket = nullptr;
                        } else {
                            // If the data we had did not complete the stream packet, there will be no remainder or the
                            // data we used was invalid.
                            if (streamPacket->is_invalid()) {
                                // This means we are out of sync with the sender.  We should disconnect.
                                stop();
                                return;
                            }
                        }
                    }
                    if (remainder == nullptr) break;
                }
                free(data);
            }
        }
        if (streamPacket != nullptr) {
            delete(streamPacket);
        }
    }
}