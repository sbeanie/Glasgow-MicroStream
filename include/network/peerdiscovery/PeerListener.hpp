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

#include "../../stream/source/NetworkSource.hpp"
#include "PeerDiscoverer.hpp"
#include "packet/StreamPacket.hpp"

namespace glasgow_ustream {

    class PeerListener {

    private:

        PeerDiscoverer *peerDiscoverer;
        char *stream_id;

        StreamPacketDataReceiver *streamPacketDataReceiver;

        std::thread thread;
        bool should_run, should_process_data;

        uint16_t port_number;
        in_addr source_addr;
        int socket_fd = 0;

        void run();

        void unregister_with_peer_discoverer() {
            peerDiscoverer->unregister_peer_listener(stream_id, this);
        }

    public:

        /**
         * Constructs a peer listener that is responsible for receiving stream values over a tcp socket.
         * @param peerDiscoverer the peerdiscover object.
         * @param stream_id the stream identifier this listener is responsible for.
         * @param source_addr the ip address of the source.
         * @param port_number the port the stream is hosted on the source's device.
         * @param streamPacketDataReceiver the deserializer that the data received should be forwarded to.
         */
        PeerListener(PeerDiscoverer *peerDiscoverer, const char *stream_id, in_addr source_addr, uint16_t port_number,
                     StreamPacketDataReceiver *streamPacketDataReceiver) :
                peerDiscoverer(peerDiscoverer), streamPacketDataReceiver(streamPacketDataReceiver),
                port_number(port_number), source_addr(source_addr) {
            this->should_run = true;
            this->should_process_data = false;

            this->stream_id = (char *) malloc(strlen(stream_id) + 1);
            strcpy(this->stream_id, stream_id);

            this->thread = std::thread(&PeerListener::run, this);
        }

        /**
         * Called when the listener should start to push data it receives through the topology.
         */
        void start() {
            should_process_data = true;
        }

        /**
         * Shutdown the peer listener and free its resources.
         */
        void stop() {
            if (should_run) {
                this->should_run = false;
                if (socket_fd != 0) {
                    shutdown(socket_fd, SHUT_RDWR);
                    close(socket_fd);
                }
                if (std::this_thread::get_id() == thread.get_id()) {
                    unregister_with_peer_discoverer();
                    peerDiscoverer->add_peer_listener_to_cleanup(this);
                }
            }
            if (std::this_thread::get_id() != thread.get_id()) {
                if (thread.joinable()) thread.join();
            }
        }

        uint16_t get_port_number() {
            return port_number;
        }

        in_addr get_source_addr() {
            return source_addr;
        }

        ~PeerListener() {
            free(stream_id);
        }

    };
}

#include "PeerListener.cpp"

#endif //GU_EDGENT_PEERLISTENER_HPP
