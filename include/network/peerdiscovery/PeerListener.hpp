#ifndef GU_EDGENT_PEERLISTENER_HPP
#define GU_EDGENT_PEERLISTENER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include <stream/source/NetworkSource.hpp>

class PeerListener {

private:
    StreamPacketDataReceiver *streamPacketDataReceiver;

    std::thread thread;
    bool should_run;

    uint16_t port_number;
    in_addr source_addr;
    int socket_fd = 0;

    void run() {
        struct sockaddr_in address;
        ssize_t recv_bytes_received;
        struct sockaddr_in serv_addr;
        char msgbuf[GU_EDGENT_NETWORK_BUFFER_SIZE] = {0}; //todo

        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            stop();
            return;
        }

        memset(&serv_addr, '0', sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port_number);
        serv_addr.sin_addr = source_addr;

        std::cout << "Connecting to port: " << port_number << std::endl;

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
            auto num_bytes_received = (size_t) recv_bytes_received; // Safe cast as -1 if statement catches a failure.
            void *data = malloc(num_bytes_received);
            memcpy(data, msgbuf, num_bytes_received);
            this->streamPacketDataReceiver->receive({num_bytes_received, data});
        }
    }

public:

    void stop() {
        this->should_run = false;
        std::cout << "Stopping PeerListener" << std::endl;
        if (socket_fd != 0) {
            close(socket_fd);
        }
        if (std::this_thread::get_id() != thread.get_id()) {
            thread.join();
        }
    }

    uint16_t get_port_number() {
        return port_number;
    }

    in_addr get_source_addr() {
        return source_addr;
    }

    void start() {
        if (this->should_run) return; // todo stop()
        this->should_run = true;
        this->thread = std::thread(&PeerListener::run, this);
    }

    PeerListener(in_addr source_addr, uint16_t port_number, StreamPacketDataReceiver* streamPacketDataReceiver) :
            streamPacketDataReceiver(streamPacketDataReceiver), source_addr(source_addr), port_number(port_number) {
        this->should_run = false;
    }

};

#endif //GU_EDGENT_PEERLISTENER_HPP
