#ifndef GU_EDGENT_NETWORKEDSTREAMHANDLER_H
#define GU_EDGENT_NETWORKEDSTREAMHANDLER_H


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include <cstring>
#include <iostream>

#include "Stream.hpp"

template <typename T>
class NetworkSink : public Subscriber<T> {

    std::pair<unsigned int, void*> (*val_to_byte_array) (T val);

    const char *stream_name, *multicast_group;
    uint16_t udp_port;

    struct sockaddr_in addr;
    int socket_fd;

private:

    void receive(T value) override {

        std::pair<unsigned int, void*> bytes = val_to_byte_array(value);

        if (sendto(socket_fd, bytes.second, bytes.first, 0, (struct sockaddr *) &addr,
                   sizeof(addr)) < 0) {
            perror("sendto");
        }
        free(bytes.second);
    }

public:

    NetworkSink<T> (const char *stream_name, const char *multicast_group, uint16_t udp_port, std::pair<unsigned int, void*> (*val_to_byte_array) (T val) ) :
            stream_name(stream_name), multicast_group(multicast_group), udp_port(udp_port), val_to_byte_array(val_to_byte_array) {

        /* create what looks like an ordinary UDP socket */
        if ( (socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("socket");
        }

        /* set up destination address */
        memset(&addr,0,sizeof(addr));
        addr.sin_family=AF_INET;
        addr.sin_addr.s_addr=inet_addr(this->multicast_group);
        addr.sin_port=htons(this->udp_port);
    }


    void notify_subscribeable_deleted(Subscribeable<T> *) override {
        delete_and_notify();
    };

    void add_subscribeable(Subscribeable<T> *) override {};

    bool delete_and_notify() override {
        delete(this);
        return true;
    }

};

#endif //GU_EDGENT_NETWORKEDSTREAMHANDLER_H
