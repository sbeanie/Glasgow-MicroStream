#ifndef GU_EDGENT_TOPOLOGY_H
#define GU_EDGENT_TOPOLOGY_H

#include <unordered_map>
#include "Stream.hpp"

#include "network/NetworkSender.hpp"
#include "network/NetworkListener.hpp"
#include "network/StreamPacket.hpp"

#define DEFAULT_MULTICAST_GROUP "225.0.0.37"
#define DEFAULT_UDP_PORT 12345


class Topology {

private:
    std::list<Startable*> startables;

    NetworkListener* networkListener;
    NetworkSender* networkSender;
    std::unordered_map<std::string, StreamPacketDataReceiver*> network_source_map;

public:


    Topology() {
        networkListener = new NetworkListener(this, DEFAULT_MULTICAST_GROUP, DEFAULT_UDP_PORT);
        networkSender = new NetworkSender(DEFAULT_MULTICAST_GROUP, DEFAULT_UDP_PORT);
    }

    // Responsible for unpacking a received packet and directing it to the correct source stream.
    void receive(std::pair<size_t, void *> data) {
        // Try and construct a stream packet from the data.
        StreamPacket* streamPacket = new StreamPacket(data);
        if ( ! streamPacket->is_valid()) {
            delete(streamPacket);
            std::cout << "Received invalid packet." << std::endl;
            return;
        }
        const char *stream_id = streamPacket->get_stream_id();
        // Perform lookup and direct data to corresponding receiver stream for deserialization.
        auto ptr = network_source_map.find(stream_id);
        if (ptr == network_source_map.end()) {
            delete(streamPacket);
            std::cout << "Could not find network source object to send packet to." << std::endl;
            return;
        }
        ptr->second->receive(std::pair<size_t, void*>(streamPacket->get_data_size(), streamPacket->get_stream_data()));
        delete(streamPacket);
    }

    // Responsible for packaging stream_id info into a fixed format packet
    void send(const char *stream_id, std::pair<size_t, void *> data) {
        StreamPacket* streamPacket = new StreamPacket(stream_id, data);
        networkSender->send(streamPacket);
        delete(streamPacket);
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

    template <typename T>
    std::optional<NetworkSource<T>*> addNetworkSource(const char *stream_id, std::optional<T> (*deserialize_func) (std::pair<size_t, void *>)) {
        auto itr = network_source_map.find(stream_id);
        if (itr != network_source_map.end()) { // Stream id already exists
            return std::nullopt;
        }

        auto *networkSource = new NetworkSource<T>(deserialize_func);
        auto *streamPacketDataReceiver = (StreamPacketDataReceiver *) networkSource;
        network_source_map.insert(std::pair<const char*, StreamPacketDataReceiver*>(stream_id, streamPacketDataReceiver));
        return networkSource;
    }

    void run() {
        for (auto &startable : startables) {
            (*startable).start();
        }
    }

    void shutdown() {
        networkListener->stop();
    }

    ~Topology() {
        networkListener->stop();
        delete(networkListener);
        delete(networkSender);
        for (auto &startable : startables) {
            CascadeDeleteable* cascadeDeleteable = dynamic_cast<CascadeDeleteable*>(startable);
            cascadeDeleteable->delete_and_notify();
        }
        for (auto &kv : network_source_map) {
            CascadeDeleteable* cascadeDeleteable = dynamic_cast<CascadeDeleteable*>(kv.second);
            cascadeDeleteable->delete_and_notify();
        }
    }
};


#include "network/NetworkListener.cpp"

#endif //GU_EDGENT_TOPOLOGY_H
