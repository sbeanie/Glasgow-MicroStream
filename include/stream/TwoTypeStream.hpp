#ifndef _TWO_TYPE_STREAM_H_
#define _TWO_TYPE_STREAM_H_

#include "StreamTypes.hpp"
#include "Subscribeable.hpp"
#include "Subscriber.hpp"
#include <chrono>
#include <vector>

template <typename INPUT, typename OUTPUT>
class TwoTypeStream : public Subscriber<INPUT>, public Subscribeable<OUTPUT> {

protected:
    std::mutex subscribers_lock;
    std::list<Subscriber<OUTPUT>* > subscribers;
    std::list<Subscribeable<INPUT>* > subscribeables;

public:

    virtual void publish(OUTPUT value) override {
        std::lock_guard<std::mutex> lock(subscribers_lock);
        for (auto subscribersIterator = subscribers.begin(); subscribersIterator != subscribers.end(); subscribersIterator++) {
            (*subscribersIterator)->receive(value);
        }
    }

    void unsubscribe(Subscriber<OUTPUT>* subscriber) override {
        std::lock_guard<std::mutex> lock(subscribers_lock);
        for (auto subscribersIterator = subscribers.begin(); subscribersIterator != subscribers.end(); subscribersIterator++) {
            if (*subscribersIterator == subscriber) {
                subscribers.remove(*subscribersIterator);
                return;
            }
        }
    }

    void subscribe(Subscriber<OUTPUT>* subscriber) override {
        std::lock_guard<std::mutex> lock(subscribers_lock);
        subscribers.push_back(subscriber);
        subscriber->add_subscribeable((Subscribeable<OUTPUT>*) this);
    }

    void notify_subscribeable_deleted(Subscribeable<INPUT>* subscribeable) override {
        for (auto subscribeableIterator = subscribeables.begin(); subscribeableIterator != subscribeables.end(); subscribeableIterator++) {
            if (*subscribeableIterator == subscribeable) {
                subscribeables.remove(*subscribeableIterator);
                break;
            }
        }
        if (subscribeables.size() == 0) {
            delete_and_notify();
        }
    }

    bool delete_and_notify() override {
        if (subscribeables.size() != 0) return false;
        for (auto subscribersIterator = subscribers.begin(); subscribersIterator != subscribers.end(); subscribersIterator++) {
            Subscribeable<OUTPUT>* subscribeable = this;
            (*subscribersIterator)->notify_subscribeable_deleted(subscribeable);
        }
        delete(this);
        return true;
    }

    void add_subscribeable(Subscribeable<INPUT>* subscribeable) override {
        std::lock_guard<std::mutex> lock(subscribers_lock);
        subscribeables.push_back(subscribeable);
    }

    Sink<OUTPUT>* sink(void (*sink_function)(OUTPUT)) {
        Sink<OUTPUT>* sink = new Sink<OUTPUT>(sink_function);
        this->subscribe(sink);
        return sink;
    }

    FilterStream<OUTPUT>* filter(bool (*filter_function)(OUTPUT)) {
        FilterStream<OUTPUT>* filter_stream = new FilterStream<OUTPUT>(filter_function);
        this->subscribe(filter_stream);
        return filter_stream;
    }

    template <typename X>
    MapStream<OUTPUT, X>* map(X (*map_function)(OUTPUT)) {
        MapStream<OUTPUT, X>* map_stream = new MapStream<OUTPUT, X>(map_function);
        this->subscribe(map_stream);
        return map_stream;
    }

    std::vector<Stream<OUTPUT>*> split(int num_streams, int (*split_function)(OUTPUT)) {

        std::vector<Stream<OUTPUT>* > streams;

        for (int i = 0; i < num_streams; i++) {
            streams.push_back(new Stream<OUTPUT>());
        }

        SplitStream<OUTPUT>* split_stream = new SplitStream<OUTPUT>(split_function, num_streams, streams);

        this->subscribe(split_stream);
        return streams;
    }

    Stream<OUTPUT>* union_streams(std::list<Subscribeable<OUTPUT>*> streams) {
        auto *union_stream = new Stream<OUTPUT>();
        this->subscribe(union_stream);
        for (auto ptr = streams.begin(); ptr != streams.end(); ptr++) {
            (*ptr)->subscribe(union_stream);
        }
        return union_stream;
    }

    Window<OUTPUT>* last(std::chrono::duration<double> duration, int number_of_splits, int (*func_val_to_int)(OUTPUT)) {
        Window<OUTPUT>* window = new Window<OUTPUT>(duration, number_of_splits, func_val_to_int);
        this->subscribe(window);
        return window;
    }


    NetworkSink<OUTPUT>* networkSink(Topology *topology, char *stream_id, std::pair<size_t, void*> (*val_to_bytes) (OUTPUT)) {
        NetworkSink<OUTPUT>* networkSink = new NetworkSink<OUTPUT>(topology, stream_id, val_to_bytes);
        this->subscribe(networkSink);
        return networkSink;
    }

    virtual ~TwoTypeStream() {}
};

#endif