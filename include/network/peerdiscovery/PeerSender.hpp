#ifndef GU_EDGENT_PEERSENDER_HPP
#define GU_EDGENT_PEERSENDER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include <cstring>
#include <iostream>
#include <unistd.h>

class PeerSender {

private:

    const char *stream_id;

    uint16_t tcp_port = 0; // Default 0 binds to any available port.
    int listen_socket_fd = 0;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    std::recursive_mutex subscriber_sockets_lock;
    std::list<int> subscriber_sockets;

    std::thread thread;
    bool should_run = false;

    void start_listening() {

        int new_socket;

        while (should_run) {
            if ((new_socket = accept(listen_socket_fd, (struct sockaddr *) &address,
                                     (socklen_t *) &addrlen)) < 0) {
//                perror("accept");
            }
            if ( ! should_run) return;
            std::lock_guard<std::recursive_mutex> lock(subscriber_sockets_lock);
            subscriber_sockets.push_back(new_socket);
        }
    }

public:

    uint16_t get_tcp_port() {
        return tcp_port;
    }

    const char *get_stream_id() {
        return stream_id;
    }

    void send_data(std::pair<size_t, void *> data) {
        std::lock_guard<std::recursive_mutex> lock(subscriber_sockets_lock);
        if (subscriber_sockets.size() == 0) {
            std::cout << "No subscribers to send data to." << std::endl;
        }
        subscriber_sockets.remove_if([data](int socket) {
            ssize_t bytes_sent = send(socket, data.second, data.first, 0 );
            return bytes_sent == -1;
        });
    }

    void start() {
        int opt = 1;

        // Creating socket file descriptor
        if ((listen_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
        }

        // Forcefully attaching socket to the port 8080
        if (setsockopt(listen_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                       &opt, sizeof(opt))) {
            perror("setsockopt");
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(tcp_port);

        // Forcefully attaching socket to the port 8080
        if (bind(listen_socket_fd, (struct sockaddr *)&address,
                 sizeof(address))<0) {
            perror("bind failed");
        }

        if (listen(listen_socket_fd, 3) < 0) {
            perror("listen");
        }

        struct sockaddr_in server_address;
        int len = sizeof(struct sockaddr);
        if (getsockname(listen_socket_fd, (struct sockaddr *) &server_address, (socklen_t *) &len ) < 0 )
            perror ( "getsockname" );
        tcp_port = ntohs(server_address.sin_port);
        std::cout << "[" << stream_id << "] Providing on port: " << tcp_port << std::endl;

        should_run = true;
        this->thread = std::thread(&PeerSender::start_listening, this);
    }

    void stop() {
        if (listen_socket_fd == 0) return;
        if ( ! should_run) return;
        this->should_run = false;
        shutdown(listen_socket_fd, SHUT_RDWR);
        close(listen_socket_fd);
        this->thread.join();
        for (auto ptr = subscriber_sockets.begin(); ptr != subscriber_sockets.end(); ptr++ ) {
            shutdown(*ptr, SHUT_RDWR);
            close(*ptr);
        }
    }

    PeerSender(const char *stream_id) : stream_id(stream_id) {

    }

    PeerSender(const char *stream_id, uint16_t tcp_port) : stream_id(stream_id), tcp_port(tcp_port) {

    }
};


#endif //GU_EDGENT_PEERSENDER_HPP
