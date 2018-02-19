#ifndef GU_EDGENT_NETWORKSINK_H
#define GU_EDGENT_NETWORKSINK_H

#include "Stream.hpp"

template <typename T>
class NetworkSink : public Subscriber<T> {

private:

    Topology *topology;
    const char *stream_id;
    std::pair<size_t, void*> (*val_to_bytes) (T);

public:

    NetworkSink(Topology *topology, const char *stream_id, std::pair<size_t, void*> (*val_to_bytes) (T)) :
            topology(topology), stream_id(stream_id), val_to_bytes(val_to_bytes) {
        topology->addNetworkSink(stream_id);
    }

    void receive(T val) {
        std::pair<size_t, void*> data = val_to_bytes(val);
        topology->send_network_data(stream_id, data);
        free(data.second);
    }

    void notify_subscribeable_deleted(Subscribeable<T> *) override {
        delete_and_notify();
    };

    void add_subscribeable(Subscribeable<T> *) override {};

    bool delete_and_notify() override {
        delete(this);
        return true;
    }

    virtual ~NetworkSink() = default;
};

#endif //GU_EDGENT_NETWORKSINK_H
