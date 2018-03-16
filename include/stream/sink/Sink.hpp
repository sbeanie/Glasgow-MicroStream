#ifndef _SINK_H_
#define _SINK_H_

#include "../Subscriber.hpp"

template <typename T>
class Sink : public Subscriber<T> {
    
    void (*sink_function)(T);

public:
    Sink(void (*sink_function)(T)) : sink_function(sink_function) {};

    void notify_subscribeable_deleted(Subscribeable<T> *) override {
        delete_and_notify();
    };

    void add_subscribeable(Subscribeable<T> *) override {};

    bool delete_and_notify() override {
        delete(this);
        return true;
    }

    void receive(T value) override {
            sink_function(value);
    }

    virtual ~Sink() {}
};

#include "NetworkSink.hpp"

#ifdef COMPILE_WITH_BOOST
#include "BoostSerializedNetworkSink.hpp"
#endif

#endif