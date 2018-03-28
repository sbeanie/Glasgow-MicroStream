#ifndef GU_EDGENT_PEERDISCOVERER_H
#define GU_EDGENT_PEERDISCOVERER_H

#include <list>
#include <mutex>
#include <cstring>
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
#include "packet/PeerDiscoveryPacketTypes.hpp"

#ifndef GU_EDGENT_NETWORK_BUFFER_SIZE
#define GU_EDGENT_NETWORK_BUFFER_SIZE 1024
#endif

namespace glasgow_ustream {

    class PeerListener;

    class PeerSender;

    class PeerDiscoverer {

    private:

        // This map represents streams that are provided externally and need to be located by the peer discovery protocol.
        std::recursive_mutex search_lock;
        std::unordered_map<std::string, std::pair<StreamPacketDataReceiver *, std::list<PeerListener *> > > stream_ids_to_search_for;

        std::recursive_mutex premature_shutdown_listener_lock;
        // Used when a listener fails and needs to be cleaned up.  It is cleaned up once per broadcast.
        std::list<PeerListener *> premature_shutdown_listener_list;

        // This map represents streams that are provided locally and should be made available via the peer discovery protocol.
        std::recursive_mutex publish_lock;
        std::unordered_map<std::string, PeerSender *> stream_ids_to_publish;


        bool should_run;
        std::thread multicast_listening_thread;
        std::thread peer_discovery_thread;
        std::chrono::duration<double> broadcast_period;

        const char *multicast_group;
        uint16_t udp_port;
        int listen_socket_fd, send_socket_fd;
        struct sockaddr_in send_addr;


        /**
         * Used to process packets received via the multicast group.
         * @param sender the sender's IP address.
         * @param data the message (query/reply) the sender sent in bytes.
         */
        void process_packet(struct sockaddr_in sender, std::pair<uint32_t, void *> data);

        /**
         * Checks a listener for a source address and port already exists for a stream identifier.
         * @param stream_id
         * @param source_addr
         * @param source_port
         * @return Returns true if it does, false otherwise.
         */
        bool listener_already_exists(const char *stream_id, in_addr source_addr, uint16_t source_port);

        /**
         * Starts the peer discoverer protocol's listener thread.  This thread listens for multicast messages and
         * processes them.
         */
        void start_listening();

        /**
         * Starts a thread that periodically broadcasts queries/replies in order to allow other devices to discover
         * streams this topology can provide, and for this topology to potentially discover streams it needs.
         */
        void periodically_discover();

        /**
         * Sends a reply message to the multicast group, using the stream id and port from the peer sender.
         * @param peerSender the peer sender instance responsible for the stream id the reply message is for.
         */
        void send_peer_discovery_reply(PeerSender *peerSender);

        /**
         * Sends a query to the multicast group, looking for the stream identifier specified.
         * @param stream_id the stream identifier that should be externally sourced.
         */
        void send_peer_discovery_query(const char *stream_id);

        /**
         * Cleans up resources left behind by failed peer listeners.
         */
        void cleanup_peer_listeners();


    public:

        /**
         * Can be used to check if networked sources/sinks have at least one connection.
         * @return true if all published/subscribed streams at least one connection
         */
        bool check_connected();

        template<typename T>
        bool add_network_source(NetworkSource<T> *networkSource, const char *stream_id);

        /**
         * Registers a stream id to make public using the peer discovery protocol.  This method binds to any port available.
         * @param stream_id the stream identifier.
         */
        void register_network_sink(const char *stream_id);

        /**
         * Registers a stream id to make public using the peer discovery protocol.
         * @param stream_id the stream idendtifier.
         * @param tcp_port the port to bind the publisher to.
         */
        void register_network_sink(const char *stream_id, uint16_t tcp_port);

        /**
         * Publishes data via the PeerDiscoverer's PeerSender.
         * @param stream_id the stream identifier to publish the data for.
         * @param data the data in raw bytes.
         */
        void send_network_data(const char *stream_id, std::pair<uint32_t, void *> data);

        /**
         * Tells the peer listeners to start processing data they receive.
         */
        void start();

        /**
         * Shutdown the peer discoverer and its resources.
         */
        void stop();

        /**
         * Adds a peer listener to the cleanup list.  If a peer listener fails its thread must be joined.  This cannot be
         * done by the peer listener's thread itself, and must be done by an external thread.
         */
        void add_peer_listener_to_cleanup(PeerListener *peerListener);

        /**
         * Removes a peer listener from the peer discoverer.  This occurs when a peer listener fails.
         * @param stream_id the stream identifier of the peer listener.
         * @param peerListener a reference to the peer listener object.
         */
        void unregister_peer_listener(const char *stream_id, PeerListener *peerListener);

        void set_broadcast_period(std::chrono::duration<double> broadcast_period) {
            this->broadcast_period = broadcast_period;
        }

        PeerDiscoverer(const char *multicast_group, uint16_t udp_port, std::chrono::duration<double> broadcast_period)
                : broadcast_period(broadcast_period), multicast_group(multicast_group), udp_port(udp_port) {
            should_run = true;

            // Setup sending.
            if ((send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket");
            }

            /* set up destination address */
            memset(&send_addr, 0, sizeof(send_addr));
            send_addr.sin_family = AF_INET;
            send_addr.sin_addr.s_addr = inet_addr(this->multicast_group);
            send_addr.sin_port = htons(this->udp_port);

            this->multicast_listening_thread = std::thread(&PeerDiscoverer::start_listening, this);
            this->peer_discovery_thread = std::thread(&PeerDiscoverer::periodically_discover, this);
        }
    };
}

#include "PeerListener.hpp"
#include "PeerSender.hpp"

#include "PeerDiscoverer.cpp"

#endif //GU_EDGENT_PEERDISCOVERER_H
