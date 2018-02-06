#ifndef GU_EDGENT_NETWORKLISTENER_H
#define GU_EDGENT_NETWORKLISTENER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include <cstring>
#include <iostream>

#include "Stream.hpp"

#define MSGBUFSIZE 256

class Topology;

class NetworkListener {

    std::thread thread;
    bool should_run, stopped = false;

    Topology* topology;

    const char *multicast_group;
    int socket_fd;
    uint16_t udp_port;

private:

    void start_listening();

public:

    NetworkListener (Topology* topology, const char *multicast_group, uint16_t udp_port) :
            topology(topology), multicast_group(multicast_group), udp_port(udp_port) {
        this->should_run = true;
        this->thread = std::thread(&NetworkListener::start_listening, this);
    }

    void stop() {
        if (stopped) return;
        this->should_run = false;
        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);
        this->thread.join();
        stopped = true;
    }

    ~NetworkListener() {
    }

};


#endif //GU_EDGENT_NETWORKLISTENER_H
