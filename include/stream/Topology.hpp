#ifndef GU_EDGENT_TOPOLOGY_H
#define GU_EDGENT_TOPOLOGY_H

#include <unordered_map>
#include "StreamTypes.hpp"

#include "../network/peerdiscovery/PeerDiscoverer.hpp"

#define DEFAULT_MULTICAST_GROUP "225.0.0.37"
#define DEFAULT_UDP_PORT 12345

namespace NAMESPACE_NAME {

    class Topology {

    private:
        std::list<Startable *> startables;

        PeerDiscoverer *peerDiscoverer = nullptr;

        std::unordered_map<std::string, StreamPacketDataReceiver *> network_source_map;

        void init_peer_discovery(const char *multicast_group, uint16_t udp_port,
                                 std::chrono::duration<double> broadcast_period) {
            peerDiscoverer = new PeerDiscoverer(multicast_group, udp_port, broadcast_period);
        }

    public:


        Topology() {
            init_peer_discovery(DEFAULT_MULTICAST_GROUP, DEFAULT_UDP_PORT, std::chrono::seconds(5));
        }

        Topology(std::chrono::duration<double> peer_discovery_broadcast_period) {
            init_peer_discovery(DEFAULT_MULTICAST_GROUP, DEFAULT_UDP_PORT, peer_discovery_broadcast_period);
        }

        Topology(bool without_networking) {
            if (!without_networking) {
                init_peer_discovery(DEFAULT_MULTICAST_GROUP, DEFAULT_UDP_PORT, std::chrono::seconds(5));
            }
        }

        template<typename T>
        FixedDataSource<T> *addFixedDataSource(std::list<T> values) {
            auto *fixedDataSource = new FixedDataSource<T>(values);
            startables.push_back((Startable *) fixedDataSource);
            return fixedDataSource;
        }

        template<typename T>
        PolledSource<T> *addPolledSource(std::chrono::duration<double> interval, Pollable<T> *pollable) {
            auto *polledSource = new PolledSource<T>(interval, pollable);
            startables.push_back((Startable *) polledSource);
            return polledSource;
        }

        void set_broadcast_period(std::chrono::duration<double> period) {
            peerDiscoverer->set_broadcast_period(period);
        }

        void send_network_data(const char *stream_id, std::pair<uint32_t, void *> data) {
            peerDiscoverer->send_network_data(stream_id, data);
        }

        void addNetworkSink(const char *stream_id) {
            if (peerDiscoverer == nullptr) {
                std::cerr << "Cannot add a network sink with peer discovery disabled." << std::endl;
                exit(1);
            }
            peerDiscoverer->register_network_sink(stream_id);
        }

        template<typename T>
        optional<NetworkSource<T> *>
        addNetworkSource(const char *stream_id, optional<T> (*deserialize_func)(std::pair<uint32_t, void *>)) {
            if (peerDiscoverer == nullptr) {
                std::cerr << "Cannot add a network source with peer discovery disabled." << std::endl;
                exit(1);
            }
            auto *networkSource = new NetworkSource<T>(deserialize_func);
            bool added = peerDiscoverer->add_network_source(networkSource, stream_id);

            if (added) {
                return optional<NetworkSource<T> *>(networkSource);
            } else {
                networkSource->delete_and_notify();
                return optional<NetworkSource<T> *>();
            }
        }

#ifdef COMPILE_WITH_BOOST_SERIALIZATION

        template<typename T>
        optional<BoostSerializedNetworkSource<T> *> addBoostSerializedNetworkSource(const char *stream_id) {
            if (peerDiscoverer == nullptr) {
                std::cerr << "Cannot add a network source with peer discovery disabled." << std::endl;
                exit(1);
            }
            auto *networkSource = new BoostSerializedNetworkSource<T>();
            bool added = peerDiscoverer->add_network_source(networkSource, stream_id);

            if (added) {
                return optional<BoostSerializedNetworkSource<T> *>(networkSource);
            } else {
                networkSource->delete_and_notify();
                return optional<BoostSerializedNetworkSource<T> *>();
            }
        }

#endif

        bool peers_connected() {
            if (this->peerDiscoverer == nullptr) return false;
            return this->peerDiscoverer->check_connected();
        }

        void start() {
            if (peerDiscoverer != nullptr) peerDiscoverer->start();
            for (auto &startable : startables) {
                startable->start();
            }
        }

        void run() {
            this->start();
            for (auto &startable : startables) {
                startable->join();
            }
        }

        void shutdown() {
            for (auto &startable : startables) {
                startable->stop();
            }
            for (auto &startable : startables) {
                startable->join();
            }
        }

        ~Topology() {
            for (auto &startable : startables) {
                CascadeDeleteable *cascadeDeleteable = dynamic_cast<CascadeDeleteable *>(startable);
                cascadeDeleteable->delete_and_notify();
            }
            for (auto &key_value_pair : network_source_map) {
                CascadeDeleteable *cascadeDeleteable = dynamic_cast<CascadeDeleteable *>(key_value_pair.second);
                cascadeDeleteable->delete_and_notify();
            }
            if (peerDiscoverer != nullptr) {
                peerDiscoverer->stop();
                delete (peerDiscoverer);
            }
        }
    };
}

#endif //GU_EDGENT_TOPOLOGY_H
