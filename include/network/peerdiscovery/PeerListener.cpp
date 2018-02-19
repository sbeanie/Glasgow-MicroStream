#include "network/peerdiscovery/PeerListener.hpp"

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

    std::cout << "[" << stream_id << "] Connecting to: " << inet_ntoa(source_addr) << ":" << port_number << std::endl;

    if (connect(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        stop();
        return;
    }

    while (should_run) {
        if ((recv_bytes_received = read(socket_fd, msgbuf, 1024)) < 0) {
            stop();
            return;
        }
        if ( ! should_run) {
            // We might have been interrupted during our read
            stop();
            return;
        }
        auto num_bytes_received = (size_t) recv_bytes_received; // Safe cast as -1 if statement catches a failure.
        if (should_process_data) {
            void *data = malloc(num_bytes_received);
            memcpy(data, msgbuf, num_bytes_received);
            std::cout << "Received num bytes: " << num_bytes_received << std::endl;
            this->streamPacketDataReceiver->receive({num_bytes_received, data});
        }
    }
}