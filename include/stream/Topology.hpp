#ifndef GU_EDGENT_TOPOLOGY_H
#define GU_EDGENT_TOPOLOGY_H

#include <unordered_map>
#include "StreamTypes.hpp"

#include "../network/peerdiscovery/PeerDiscoverer.hpp"

#define DEFAULT_MULTICAST_GROUP "225.0.0.37"
#define DEFAULT_UDP_PORT 12345

namespace glasgow_ustream {

    /**
     * This class represents a computational graph that can be executed.
     */
    class Topology {

    private:
        std::list<Startable *> startables; // A list of objects that should be started when the topology is executed.

        PeerDiscoverer *peerDiscoverer = nullptr; // This class manages the peer discovery protocol.

        std::unordered_map<std::string, StreamPacketDataReceiver *> network_source_map;

        void init_peer_discovery(const char *multicast_group, uint16_t udp_port,
                                 std::chrono::duration<double> broadcast_period) {
            peerDiscoverer = new PeerDiscoverer(multicast_group, udp_port, broadcast_period);
        }

    public:


        /**
         * Constructs a topology object with the default multicast group, udp port, and broadcast period.
         */
        Topology() {
            init_peer_discovery(DEFAULT_MULTICAST_GROUP, DEFAULT_UDP_PORT, std::chrono::seconds(5));
        }

        /**
         * Constructs a topology object with the default multicast group and port, but with a specified broadcast period.
         * @param peer_discovery_broadcast_period the period to wait before broadcasting/querying for stream identifiers.
         */
        Topology(std::chrono::duration<double> peer_discovery_broadcast_period) {
            init_peer_discovery(DEFAULT_MULTICAST_GROUP, DEFAULT_UDP_PORT, peer_discovery_broadcast_period);
        }

        /**
         * Constructs a topology object with the default multicast group and port, but with a specified broadcast period.
         * @param peer_discovery_broadcast_period the period to wait before broadcasting/querying for stream identifiers.
         */
        Topology(const char *multicast_group, uint16_t udp_port, std::chrono::duration<double> peer_discovery_broadcast_period) {
            init_peer_discovery(multicast_group, udp_port, peer_discovery_broadcast_period);
        }

        /**
         * Constructs a topology without networking if true is passed.  Otherwise constructs a default topology.
         * @param without_networking if true disables the peer discovery protocol.
         */
        Topology(bool without_networking) {
            if (!without_networking) {
                init_peer_discovery(DEFAULT_MULTICAST_GROUP, DEFAULT_UDP_PORT, std::chrono::seconds(5));
            }
        }

        /**
         * Creates a source node in the topology that provides a predefined list of values.
         * @tparam T the type of the values provided.
         * @param values the values.
         * @return A reference to the FixedDataSource topology node.
         */
        template<typename T>
        FixedDataSource<T> *addFixedDataSource(std::list<T> values) {
            auto *fixedDataSource = new FixedDataSource<T>(values);
            startables.push_back((Startable *) fixedDataSource);
            return fixedDataSource;
        }

        /**
         * Creates a source node in the topology that provides values at a periodic interval.
         * @tparam T the type of values produced.
         * @param interval the interval at which values are produced.
         * @param pollable A pollable object.
         * @return A reference to the PolledSource topology node.
         */
        template<typename T>
        PolledSource<T> *addPolledSource(std::chrono::duration<double> interval, Pollable<T> *pollable) {
            auto *polledSource = new PolledSource<T>(interval, pollable);
            startables.push_back((Startable *) polledSource);
            return polledSource;
        }

        /**
         * Creates a source node in the topology that provides values from an iterable.
         * @tparam T the type of values produced.
         * @param iterable An object implementing the iterable interface.
         * @return A reference to the IterableSource topology node.
         */
        template <typename T>
        IterableSource<T> *addIterableSource(Iterable<T> *iterable) {
            auto *iterableSource = new IterableSource<T>(iterable);
            startables.push_back((Startable *) iterableSource);
            return iterableSource;
        }

        /**
         * Redefines the broadcast period for the peer discovery protocol.
         * @param period The new period.
         */
        void set_broadcast_period(std::chrono::duration<double> period) {
            if (peerDiscoverer == nullptr) return;
            peerDiscoverer->set_broadcast_period(period);
        }

        /**
         * This method is used by NetworkSinks to request data to be sent via the peer discovery protocol.
         * @param stream_id the stream identifier to which data should be published.
         * @param data the raw data in bytes.
         */
        void send_network_data(const char *stream_id, std::pair<uint32_t, void *> data) {
            peerDiscoverer->send_network_data(stream_id, data);
        }

        /**
         * Registers a network sink for a stream identifier.  This is called during the NetworkSink's constructor when
         * it requests the peer discovery protocol creates a PeerSender for it.  This method binds the peer sender
         * to a default port.
         * @param stream_id the stream identifier.
         */
        void addNetworkSink(const char *stream_id) {
            if (peerDiscoverer == nullptr) {
                std::cerr << "Cannot add a network sink with peer discovery disabled." << std::endl;
                exit(1);
            }
            peerDiscoverer->register_network_sink(stream_id);
        }

        /**
         * Registers a network sink for a stream identifier.  This is called during the NetworkSink's constructor when
         * it requests the peer discovery protocol creates a PeerSender for it.  This method binds the peer sender to
         * the specified port.
         * @param stream_id the stream identifier.
         * @param tcp_port the tcp port the peer sender should bind to.
         */
        void addNetworkSink(const char *stream_id, uint16_t tcp_port) {
            if (peerDiscoverer == nullptr) {
                std::cerr << "Cannot add a network sink with peer discovery disabled." << std::endl;
                exit(1);
            }
            peerDiscoverer->register_network_sink(stream_id, tcp_port);
        }

        /**
         * Creates a network source topology node that fetches a stream from the peer discovery protocol and deserializes
         * the data, before publishing a value of its type.
         * @tparam T the source's type.
         * @param stream_id the stream identifier to fetch.
         * @param deserialize_func A deserialization function that can convert bytes into a value of this source's type.
         * @return A reference to the NetworkSource topology node.
         */
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

        /**
         * Creates a networked source utilizing the boost serialization library.  The type must have provided a serialize
         * function that the boost serialization can use. (See boostserialize.cpp)
         * @tparam T the type of the network source.
         * @param stream_id the stream identifier the stream is provided on.
         * @return A reference to the BoostSerializedNetworkSource topology node.
         */
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

        /**
         * Checks that all peer discovery protocol streams have at least one connection.
         * @return true if there is at least one connection per source/sink.
         */
        bool peers_connected() {
            if (this->peerDiscoverer == nullptr) return false;
            return this->peerDiscoverer->check_connected();
        }

        /**
         * Starts all startables within the topology.  These are effectively all of the sources within the topology.
         */
        void start() {
            if (peerDiscoverer != nullptr) peerDiscoverer->start();
            for (auto &startable : startables) {
                startable->start();
            }
        }

        /**
         * Runs the topology by calling start(), and then joining on all of the started sources.
         */
        void run() {
            this->start();
            for (auto &startable : startables) {
                startable->join();
            }
        }

        /**
         * Runs the topology, starting a thread per startable.  This method blocks until the startables have all completed.
         */
        void run_with_threads() {
            std::list<std::thread> threads;
            for (auto &startable : startables) {
                std::thread thread(&Startable::start, startable);
                threads.push_back(std::move(thread));
            }
            for (auto &thread : threads) {
                thread.join();
            }
            for (auto &startable : startables) {
                startable->join();
            }
        }

        /**
         * This function requests the topology to stop executing, and joins any threads that may have finished.
         */
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
