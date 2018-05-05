#ifndef GU_EDGENT_NETWORKSINK_H
#define GU_EDGENT_NETWORKSINK_H

#include "../StreamTypes.hpp"

namespace glasgow_ustream {

    /**
    * This class represents a sink that is capable of serializing a type/class and publishing it via the peer discovery
     * protocol.
    * @tparam T
    */
    template<typename T>
    class NetworkSink : public Subscriber<T> {

    private:

        Topology *topology;
        std::string stream_id;

    protected:
        std::pair<uint32_t, void *> (*val_to_bytes)(T) = nullptr;

    public:

        NetworkSink(Topology *topology, std::string stream_id) : topology(topology), stream_id(stream_id) {
            topology->addNetworkSink(stream_id);
        }

        NetworkSink(Topology *topology, std::string stream_id, std::pair<uint32_t, void *> (*val_to_bytes)(T)) :
                topology(topology), stream_id(stream_id), val_to_bytes(val_to_bytes) {
            topology->addNetworkSink(stream_id);
        }

        void receive(T val) {
            if (val_to_bytes == nullptr) {
                // This is the case when the boost serialized network sink has not yet initialized the function.
                return;
            }
            std::pair<uint32_t, void *> data = val_to_bytes(val);
            topology->send_network_data(stream_id, data);
            free(data.second);
        }

        void notify_subscribeable_deleted(Subscribeable<T> *) override {
            delete_and_notify();
        };

        void add_subscribeable(Subscribeable<T> *) override {};

        bool delete_and_notify() override {
            delete (this);
            return true;
        }

        virtual ~NetworkSink() = default;
    };

}

#endif //GU_EDGENT_NETWORKSINK_H
