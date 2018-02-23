#ifndef GU_EDGENT_TOPOLOGY_H
#define GU_EDGENT_TOPOLOGY_H

#include <unordered_map>
#include <boost/optional.hpp>
#include "StreamTypes.hpp"

#include "../network/peerdiscovery/PeerDiscoverer.hpp"

#define DEFAULT_MULTICAST_GROUP "225.0.0.37"
#define DEFAULT_UDP_PORT 12345


class Topology {

private:
    std::list<Startable*> startables;

    PeerDiscoverer* peerDiscoverer;

    std::unordered_map<std::string, StreamPacketDataReceiver*> network_source_map;

public:


    Topology() {
        peerDiscoverer = new PeerDiscoverer(DEFAULT_MULTICAST_GROUP, DEFAULT_UDP_PORT, std::chrono::seconds(5));
    }

    Topology(std::chrono::duration<double> peer_discovery_broadcast_period) {
        peerDiscoverer = new PeerDiscoverer(DEFAULT_MULTICAST_GROUP, DEFAULT_UDP_PORT, peer_discovery_broadcast_period);
    }

    template <typename T>
    FixedDataSource<T>* addFixedDataSource(std::list<T> values) {
        auto *fixedDataSource = new FixedDataSource<T>(values);
        startables.push_back((Startable*) fixedDataSource);
        return fixedDataSource;
    }

    template <typename T>
    PolledSource<T>* addPolledSource(std::chrono::duration<double> interval, Pollable<T>* pollable) {
        auto *polledSource = new PolledSource<T>(interval, pollable);
        startables.push_back((Startable*) polledSource);
        return polledSource;
    }

    void set_broadcast_period(std::chrono::duration<double> period) {
        peerDiscoverer->set_broadcast_period(period);
    }

    void send_network_data(const char *stream_id, std::pair<uint32_t, void*> data) {
        peerDiscoverer->send_network_data(stream_id, data);
    }

    void addNetworkSink(const char *stream_id) {
        peerDiscoverer->register_network_sink(stream_id);
    }

    template <typename T>
    boost::optional<NetworkSource<T>* > addNetworkSource(const char *stream_id, boost::optional<T> (*deserialize_func) (std::pair<uint32_t, void *>)) {
        auto *networkSource = new NetworkSource<T>(deserialize_func);
        bool added = peerDiscoverer->add_network_source(networkSource, stream_id);

        if (added) {
            return networkSource;
        } else {
            networkSource->delete_and_notify();
            return boost::optional<NetworkSource<T>* >();
        }
    }

    void run() {
        peerDiscoverer->start();
        for (auto &startable : startables) {
            (*startable).start();
        }
    }

    void shutdown() {

    }

    ~Topology() {
        for (auto &startable : startables) {
            CascadeDeleteable* cascadeDeleteable = dynamic_cast<CascadeDeleteable*>(startable);
            cascadeDeleteable->delete_and_notify();
        }
        for (auto &key_value_pair : network_source_map) {
            CascadeDeleteable* cascadeDeleteable = dynamic_cast<CascadeDeleteable*>(key_value_pair.second);
            cascadeDeleteable->delete_and_notify();
        }
        peerDiscoverer->stop();
        delete(peerDiscoverer);
    }
};

#endif //GU_EDGENT_TOPOLOGY_H
