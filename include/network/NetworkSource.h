#ifndef GU_EDGENT_NETWORKSOURCE_H
#define GU_EDGENT_NETWORKSOURCE_H

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

template <typename T>
class NetworkSource : public Stream<T> {

    T (*byte_array_to_val) (unsigned int length, void* byte_array);

    std::thread thread;
    bool should_run;

    const char *stream_name, *multicast_group;
    uint16_t udp_port;

    struct sockaddr_in addr;
    int socket_fd, num_bytes_received;
    struct ip_mreq mreq;
    char msgbuf[MSGBUFSIZE];

    u_int reuse_addr = 1;


private:

    void process(unsigned int length, void* byte_array) {

        T val = byte_array_to_val(length, byte_array);

        free(byte_array);

        this->publish(val);
    }


    void start_listening() {
        /* create what looks like an ordinary UDP socket */

        if ( (socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("socket");
            exit(1);
        }

        /* allow multiple sockets to use the same PORT number */
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) < 0) {
            perror("Reusing ADDR failed");
            exit(1);
        }

        /* set up destination address */
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
        addr.sin_port=htons(this->udp_port);

        /* bind to receive address */
        if (bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            perror("bind");
            exit(1);
        }


        /* use setsockopt() to request that the kernel join a multicast group */
        mreq.imr_multiaddr.s_addr=inet_addr(this->multicast_group);
        mreq.imr_interface.s_addr=htonl(INADDR_ANY);
        if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
            perror("setsockopt");
            exit(1);
        }


        struct sockaddr_storage sender;
        socklen_t sendsize = sizeof(sender);
        bzero(&sender, sizeof(sender));

        /* now just enter a read-print loop */
        while (should_run) {
            socklen_t addrlen=sizeof(addr);
            if ( (num_bytes_received = recvfrom(socket_fd, msgbuf, MSGBUFSIZE,0,
                                 (struct sockaddr *) &sender, &sendsize)) < 0) {
                perror("recvfrom");
                exit(1);
            }
            std::cout << "Received message" << std::endl;
            void *data = malloc(num_bytes_received);
            memcpy(data, msgbuf, num_bytes_received);
            this->process((unsigned int) num_bytes_received, data);
        }
    }

public:

    NetworkSource<T> (const char *stream_name, const char *multicast_group, uint16_t udp_port, T (*byte_array_to_val) (unsigned int length, void* byte_array) ) :
            stream_name(stream_name), multicast_group(multicast_group), udp_port(udp_port), byte_array_to_val(byte_array_to_val) {
        this->should_run = true;
        memset(&msgbuf, 0, MSGBUFSIZE);

        this->thread = std::thread(&NetworkSource<T>::start_listening, this);
    }

};


#endif //GU_EDGENT_NETWORKSOURCE_H
