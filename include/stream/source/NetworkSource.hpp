#ifndef GU_EDGENT_NETWORKSOURCE_H
#define GU_EDGENT_NETWORKSOURCE_H

#include "../StreamTypes.hpp"

namespace glasgow_ustream {

    class StreamPacketDataReceiver {

    public:
        virtual void receive(std::pair<uint32_t, void *> data) = 0;
    };

    /**
     * This class represents a source that is capable of deserializing bytes from a networked stream.
     * @tparam T
     */
    template<typename T>
    class NetworkSource : public TwoTypeStream<std::pair<uint32_t, void *>, T>, public StreamPacketDataReceiver {


    protected:

        /**
         * This should be a deserialization function that is capable of optionally returning a typed value if valid
         * data is supplied.
         * @return
         */
        optional<T> (*deserialize_func)(std::pair<uint32_t, void *>) = nullptr;

    public:

        /**
         * This constructor is used by the boost serialized network source and should not be used externally.
         */
        NetworkSource() = default;

        explicit NetworkSource(optional<T> (*deserialize_func)(std::pair<uint32_t, void *>)) : deserialize_func(
                deserialize_func) {};

        void receive(std::pair<uint32_t, void *> data) override {
            if (deserialize_func == nullptr) {
                // This is the case for a boost serialized network source before it initiates the deserialization function.
                return;
            }
            optional<T> optionalValue = deserialize_func(data);
            if (optionalValue.is_initialized()) {
                this->publish(optionalValue.value());
            }
            free(data.second);
        }
    };
}

#endif //GU_EDGENT_NETWORKSOURCE_H
