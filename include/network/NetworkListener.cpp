#include "network/NetworkListener.hpp"

void NetworkListener::start_listening() {
    struct sockaddr_in addr;
    size_t num_bytes_received;
    struct ip_mreq mreq;
    char msgbuf[MSGBUFSIZE];
    u_int reuse_addr = 1;

    memset(&msgbuf, 0, MSGBUFSIZE);


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
        ssize_t recv_bytes_received;
        if ( (recv_bytes_received = recvfrom(socket_fd, msgbuf, MSGBUFSIZE,0,
                                             (struct sockaddr *) &sender, &sendsize)) <= 0) {
            perror("recvfrom");
            continue;
        }

        num_bytes_received = (size_t) recv_bytes_received; // Safe cast as -1 if statement catches a failure.
        void *data = malloc((size_t) num_bytes_received);
        memcpy(data, msgbuf, num_bytes_received);
        this->topology->receive(std::pair<size_t, void*> ( num_bytes_received, data));
        free(data);
    }
}