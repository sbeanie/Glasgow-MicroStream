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

class PeerListener;
class PeerSender;

class PeerDiscoverer {

private:

    std::recursive_mutex search_lock;
    std::unordered_map<std::string, std::pair<StreamPacketDataReceiver*, std::list<PeerListener *> > > stream_ids_to_search_for;

    std::recursive_mutex premature_shutdown_listener_lock;
    std::list<PeerListener*> premature_shutdown_listener_list;

    std::recursive_mutex publish_lock;
    std::unordered_map<std::string, PeerSender*> stream_ids_to_publish;


    bool should_run;
    std::thread multicast_listening_thread;
    std::thread peer_discovery_thread;
    std::chrono::duration<double> broadcast_period;

    const char *multicast_group;
    uint16_t udp_port;
    int listen_socket_fd, send_socket_fd;
    struct sockaddr_in send_addr;


    void process_packet(struct sockaddr_in sender, std::pair<uint32_t, void *> data);

    bool listener_already_exists(const char *stream_id, in_addr source_addr, uint16_t source_port);

    void start_listening();

    void periodically_discover();

    void send_peer_discovery_reply(PeerSender *peerSender);
    void send_peer_discovery_query(const char *stream_id);

    void cleanup_peer_listeners();


public:

    template <typename T>
    bool add_network_source(NetworkSource<T> *networkSource, const char *stream_id);

    void register_network_sink(const char *stream_id);

    void send_network_data(const char *stream_id, std::pair<uint32_t, void*> data);

    void start();

    void stop();

    void add_peer_listener_to_cleanup(PeerListener *peerListener);
    void unregister_peer_listener(const char *stream_id, PeerListener *peerListener);

    void set_broadcast_period(std::chrono::duration<double> broadcast_period) {
        this->broadcast_period = broadcast_period;
    }

    PeerDiscoverer(const char *multicast_group, uint16_t udp_port, std::chrono::duration<double> broadcast_period)
            : broadcast_period(broadcast_period), multicast_group(multicast_group), udp_port(udp_port) {
        should_run = true;

        // Setup sending.
        if ( (send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
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

#include "PeerListener.hpp"
#include "PeerSender.hpp"

#include "PeerDiscoverer.cpp"

#endif //GU_EDGENT_PEERDISCOVERER_H
