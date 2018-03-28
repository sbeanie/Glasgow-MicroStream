#ifndef _SINK_H_
#define _SINK_H_

#include "../Subscriber.hpp"

namespace glasgow_ustream {

    /**
     * This class consumes values from a stream as it does not publish any values and cannot be subscribed to.
     * @tparam T
     */
    template<typename T>
    class Sink : public Subscriber<T> {

        // This function is user-defined and allows a user to process a value in some arbitrary way.
        void (*sink_function)(T);

    public:
        Sink(void (*sink_function)(T)) : sink_function(sink_function) {};

        void notify_subscribeable_deleted(Subscribeable<T> *) override {
            delete_and_notify();
        };

        void add_subscribeable(Subscribeable<T> *) override {};

        bool delete_and_notify() override {
            delete (this);
            return true;
        }

        void receive(T value) override {
            sink_function(value);
        }

        virtual void stop_and_join() {}

        virtual ~Sink() {}
    };

}

#include "NetworkSink.hpp"

#ifdef COMPILE_WITH_BOOST_SERIALIZATION
#include "BoostSerializedNetworkSink.hpp"
#endif

#endif