#ifndef GU_EDGENT_PEERDISCOVERER_H
#define GU_EDGENT_PEERDISCOVERER_H

#include <list>
#include <mutex>
#include <cstring>
#include <stream/source/NetworkSource.hpp>

#include "network/peerdiscovery/packet/PeerDiscoveryPacketTypes.hpp"

#ifndef GU_EDGENT_NETWORK_BUFFER_SIZE
#define GU_EDGENT_NETWORK_BUFFER_SIZE 1024
#endif

#include "network/peerdiscovery/PeerListener.hpp"
#include "network/peerdiscovery/PeerSender.hpp"

class PeerDiscoverer {

private:

    std::recursive_mutex search_lock;
    std::unordered_map<std::string, std::pair<StreamPacketDataReceiver*, std::list<PeerListener*> > > stream_ids_to_search_for;

    std::recursive_mutex publish_lock;
    std::unordered_map<std::string, PeerSender*> stream_ids_to_publish;


    bool should_run;
    std::thread thread;

    const char *multicast_group;
    uint16_t udp_port;
    int listen_socket_fd, send_socket_fd;
    struct sockaddr_in send_addr;


    void process_packet(struct sockaddr_in sender, std::pair<size_t, void *> data) {
        uint8_t packet_type = PeerDiscoveryPacket::get_packet_type(data.second);
        if (packet_type == PEER_DISCOVERY_UNKNOWN_PACKET_TYPE) {
            std::cout << "Received unknown peer discovery packet." << std::endl;
            return;
        } else if (packet_type == PEER_DISCOVERY_QUERY_PACKET_TYPE) {
            auto peerDiscoveryQueryPacket = new PeerDiscoveryQueryPacket(data);
            if (peerDiscoveryQueryPacket->is_valid()) {
                const char *stream_id = peerDiscoveryQueryPacket->get_stream_id();
                std::lock_guard<std::recursive_mutex> lock(publish_lock);
                auto ptr = stream_ids_to_publish.find(stream_id);
                if (ptr != stream_ids_to_publish.end()) {
                    send_peer_discovery_reply(ptr->second);
                }
            } else {
                std::cout << "Failed to parse query packet." << std::endl;
            }
            delete(peerDiscoveryQueryPacket);
        } else if (packet_type == PEER_DISCOVERY_REPLY_PACKET_TYPE) {
            auto peerDiscoveryReplyPacket = new PeerDiscoveryReplyPacket(data);
            if (peerDiscoveryReplyPacket->is_valid()) {
                const char *stream_id = peerDiscoveryReplyPacket->get_stream_id();
                std::lock_guard<std::recursive_mutex> lock(search_lock);
                auto ptr = stream_ids_to_search_for.find(stream_id);
                if (ptr != stream_ids_to_search_for.end()) {
                    if (listener_already_exists(stream_id, sender.sin_addr)) {
                        delete(peerDiscoveryReplyPacket);
                        return;
                    }
                    StreamPacketDataReceiver *streamPacketDataReceiver = ptr->second.first;
                    auto *peer_listener = new PeerListener(sender.sin_addr, peerDiscoveryReplyPacket->get_port_number(), streamPacketDataReceiver);
                    ptr->second.second.push_back(peer_listener);
                    if (should_run) {
                        peer_listener->start();
                    }
                }
            } else {
                std::cout << "Failed to parse reply packet." << std::endl;
            }
            delete(peerDiscoveryReplyPacket);
        }
    }

    bool listener_already_exists(const char *stream_id, in_addr source_addr) {
        std::lock_guard<std::recursive_mutex> lock(search_lock);
        auto ptr = stream_ids_to_search_for.find(stream_id);
        if (ptr != stream_ids_to_search_for.end()) {
            std::list<PeerListener *> peerListeners = ptr->second.second;
            for (auto &peerListener : peerListeners) {
                if (peerListener->get_source_addr().s_addr == source_addr.s_addr) return true;
            }
            return false;
        } else return false;
    }


    void start_listening() {
        struct sockaddr_in addr;
        size_t num_bytes_received;
        struct ip_mreq mreq;
        char msgbuf[GU_EDGENT_NETWORK_BUFFER_SIZE];
        u_int reuse_addr = 1;

        memset(&msgbuf, 0, GU_EDGENT_NETWORK_BUFFER_SIZE);


        if ( (listen_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("socket");
            exit(1);
        }

        /* allow multiple sockets to use the same PORT number */
        if (setsockopt(listen_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) < 0) {
            perror("Reusing ADDR failed");
            exit(1);
        }

        /* set up destination address */
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
        addr.sin_port=htons(this->udp_port);

        /* bind to receive address */
        if (bind(listen_socket_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            perror("bind");
            exit(1);
        }


        /* use setsockopt() to request that the kernel join a multicast group */
        mreq.imr_multiaddr.s_addr=inet_addr(this->multicast_group);
        mreq.imr_interface.s_addr=htonl(INADDR_ANY);
        if (setsockopt(listen_socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
            perror("setsockopt");
            exit(1);
        }


        struct sockaddr_in sender;
        socklen_t sendsize = sizeof(sender);
        bzero(&sender, sizeof(sender));

        while (should_run) {
            ssize_t recv_bytes_received;
            if ( (recv_bytes_received = recvfrom(listen_socket_fd, msgbuf, GU_EDGENT_NETWORK_BUFFER_SIZE, 0,
                                                 (struct sockaddr *) &sender, &sendsize)) <= 0) {
                continue;
            }

            num_bytes_received = (size_t) recv_bytes_received; // Safe cast as -1 if statement catches a failure.
            void *data = malloc(num_bytes_received);
            memcpy(data, msgbuf, num_bytes_received);
            process_packet(sender, {num_bytes_received, data});
            bzero(&sender, sizeof(sender));
            free(data);
        }
    }

    void send_peer_discovery_reply(PeerSender *peerSender) {
        auto peerDiscoveryReplyPacket = new PeerDiscoveryReplyPacket(peerSender->get_tcp_port(), peerSender->get_stream_id());
        if ( ! peerDiscoveryReplyPacket->is_valid()) return;

        auto packet_data = peerDiscoveryReplyPacket->get_packet_data();

        if (sendto(send_socket_fd, packet_data.second, packet_data.first, 0, (struct sockaddr *) &send_addr,
                   sizeof(send_addr)) < 0) {
            perror("sendto");
        }
        free(packet_data.second);
        delete(peerDiscoveryReplyPacket);
    }

public:

    template <typename T>
    bool add_network_source(NetworkSource<T> *networkSource, const char *stream_id) {
        std::lock_guard<std::recursive_mutex> lock(search_lock);
        auto ptr = stream_ids_to_search_for.find(stream_id);
        if (ptr == stream_ids_to_search_for.end()) {
            std::pair<StreamPacketDataReceiver*, std::list<PeerListener*> > stream_connections;
            stream_connections.first = dynamic_cast<StreamPacketDataReceiver*>(networkSource);
            stream_connections.second = std::list<PeerListener*>();
            stream_ids_to_search_for.insert({stream_id, stream_connections});
            return true;
        }
        // Already exists.
        return false;
    }

    void send_network_data(const char *stream_id, std::pair<size_t, void*> data) {
        std::lock_guard<std::recursive_mutex> lock(publish_lock);
        PeerSender *peerSender = nullptr;
        auto ptr = stream_ids_to_publish.find(stream_id);
        if (ptr == stream_ids_to_publish.end()) {
            peerSender = new PeerSender(stream_id);
            stream_ids_to_publish.insert({stream_id, peerSender});
            peerSender->start();
        } else {
            peerSender = ptr->second;
        }
        send_peer_discovery_reply(peerSender);
        peerSender->send_data(data);
    }

    void start() {
        if (should_run) return;
        should_run = true;
        this->thread = std::thread(&PeerDiscoverer::start_listening, this);
        std::lock_guard<std::recursive_mutex> lock(search_lock);
        for (auto &kv : stream_ids_to_search_for) {
            std::list<PeerListener *> peerListeners = kv.second.second;
            for (auto &peerListener : peerListeners) {
                peerListener->start();
            }
        }
    }

    void stop() {
        if ( ! should_run) return;
        should_run = false;
        shutdown(listen_socket_fd, SHUT_RDWR);
        close(listen_socket_fd);
        if (thread.joinable()) thread.join();
        for (auto &kv : stream_ids_to_search_for) {
            std::pair<StreamPacketDataReceiver*, std::list<PeerListener *> > data = kv.second;
            for (auto ptr = data.second.begin(); ptr != data.second.end(); ptr++) {
                PeerListener *peerListener = *ptr;
                peerListener->stop();
                delete(peerListener);
            }
            // We know the pointer points to a NetworkSource and a NetworkSource implements CascadeDeleteable
            CascadeDeleteable *cascadeDeleteable = dynamic_cast<CascadeDeleteable*>(data.first);
            cascadeDeleteable->delete_and_notify();
        }
        for (auto &kv : stream_ids_to_publish) {
            PeerSender *peerSender = kv.second;
            peerSender->stop();
            delete(peerSender);
        }
    }


    PeerDiscoverer(const char *multicast_group, uint16_t udp_port) : multicast_group(multicast_group), udp_port(udp_port) {
        should_run = false;

        // Setup sending.
        if ( (send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("socket");
        }

        /* set up destination address */
        memset(&send_addr,0,sizeof(send_addr));
        send_addr.sin_family=AF_INET;
        send_addr.sin_addr.s_addr=inet_addr(this->multicast_group);
        send_addr.sin_port=htons(this->udp_port);
    }


};


#endif //GU_EDGENT_PEERDISCOVERER_H
