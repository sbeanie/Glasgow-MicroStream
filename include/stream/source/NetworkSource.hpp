#ifndef GU_EDGENT_NETWORKSOURCE_H
#define GU_EDGENT_NETWORKSOURCE_H

#include "../StreamTypes.hpp"
#include <optional>

class StreamPacketDataReceiver {

public:
    virtual void receive(std::pair<size_t, void*> data) = 0;
};

template <typename T>
class NetworkSource : public TwoTypeStream<std::pair<size_t, void*>, T>, public StreamPacketDataReceiver {

    std::optional<T> (*deserialize_func) (std::pair<size_t, void *>);

public:

    NetworkSource(std::optional<T> (*deserialize_func) (std::pair<size_t, void *>)) : deserialize_func(deserialize_func) {};

    void receive(std::pair<size_t, void*> data) override {
        std::optional<T> optionalValue = deserialize_func(data);
        if (optionalValue.has_value()) {
            this->publish(optionalValue.value());
        }
    }
};

#endif //GU_EDGENT_NETWORKSOURCE_H
