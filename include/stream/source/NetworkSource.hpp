#ifndef GU_EDGENT_NETWORKSOURCE_H
#define GU_EDGENT_NETWORKSOURCE_H

#include "../StreamTypes.hpp"

namespace glasgow_ustream {

    class StreamPacketDataReceiver {

    public:
        virtual void receive(std::pair<uint32_t, void *> data) = 0;
    };

    template<typename T>
    class NetworkSource : public TwoTypeStream<std::pair<uint32_t, void *>, T>, public StreamPacketDataReceiver {


    protected:

        optional<T> (*deserialize_func)(std::pair<uint32_t, void *>) = nullptr;

    public:

        NetworkSource() = default;

        explicit NetworkSource(optional<T> (*deserialize_func)(std::pair<uint32_t, void *>)) : deserialize_func(
                deserialize_func) {};

        void receive(std::pair<uint32_t, void *> data) override {
            if (deserialize_func == nullptr) return;
            optional<T> optionalValue = deserialize_func(data);
            if (optionalValue.is_initialized()) {
                this->publish(optionalValue.value());
            }
        }
    };
}

#endif //GU_EDGENT_NETWORKSOURCE_H
