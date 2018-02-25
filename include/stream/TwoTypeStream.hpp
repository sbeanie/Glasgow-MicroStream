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

    /**
     * Publishes the provided value to all subscribers of this stream.
     * @param value
     */
    virtual void publish(OUTPUT value) override {
        std::lock_guard<std::mutex> lock(subscribers_lock);
        for (auto subscribersIterator = subscribers.begin(); subscribersIterator != subscribers.end(); subscribersIterator++) {
            (*subscribersIterator)->receive(value);
        }
    }

    /**
     * Sinks any output that is published by this stream into the function provided.
     * @param sink_function A function that takes a value of the type of the stream and returns nothing.
     * @return A reference to the sink stream.  Note sink streams do not publish data.
     */
    Sink<OUTPUT>* sink(void (*sink_function)(OUTPUT)) {
        Sink<OUTPUT>* sink = new Sink<OUTPUT>(sink_function);
        this->subscribe(sink);
        return sink;
    }

    /**
     * Filters values from the stream.
     * @param filter_function The filter function.  Returning true means the value is kept.
     * @return A reference to the filtered stream.
     */
    FilterStream<OUTPUT>* filter(bool (*filter_function)(OUTPUT)) {
        FilterStream<OUTPUT>* filter_stream = new FilterStream<OUTPUT>(filter_function);
        this->subscribe(filter_stream);
        return filter_stream;
    }

    /**
     * Maps the current streams output to a stream of type X.
     * @tparam X The type of the new stream.
     * @param map_function A function mapping a value from the current stream to a value of the new type.
     * @return A reference to the mapped stream.
     */
    template <typename X>
    MapStream<OUTPUT, X>* map(X (*map_function)(OUTPUT)) {
        MapStream<OUTPUT, X>* map_stream = new MapStream<OUTPUT, X>(map_function);
        this->subscribe(map_stream);
        return map_stream;
    }

    template <typename X>
    StatefulStream<OUTPUT, X>* map_stateful(StatefulMap<OUTPUT, X> *statefulMap) {
        StatefulStream<OUTPUT, X> *statefulStream = new StatefulStream<OUTPUT, X>(statefulMap);
        this->subscribe(statefulStream);
        return statefulStream;
    }

    /**
     * Splits the current stream into several streams based on the output of a splitting function.
     * @param num_streams The number of streams the output should be split into.
     * @param split_function The function mapping a value to a stream.  The output value is modulo num_streams.
     * @return The list of streams the split function will push data into.
     */
    std::vector<Stream<OUTPUT>*> split(int num_streams, int (*split_function)(OUTPUT)) {
        std::vector<Stream<OUTPUT>* > streams;

        for (int i = 0; i < num_streams; i++) {
            streams.push_back(new Stream<OUTPUT>());
        }

        SplitStream<OUTPUT>* split_stream = new SplitStream<OUTPUT>(split_function, num_streams, streams);
        this->subscribe(split_stream);
        return streams;
    }

    /**
     * Combines two streams into a single stream.
     * @param streams A list of streams all of the same output type.
     * @return A reference to a stream that publishes the combined output of all unioned streams.
     */
    Stream<OUTPUT>* union_streams(std::list<Subscribeable<OUTPUT>*> streams) {
        auto *union_stream = new Stream<OUTPUT>();
        this->subscribe(union_stream);
        for (auto ptr = streams.begin(); ptr != streams.end(); ptr++) {
            (*ptr)->subscribe(union_stream);
        }
        return union_stream;
    }

    /**
     * Creates a window of the current stream where values are preserved for the time specified in the duration.
     * @param duration The amount of times values should remain in the window.
     * @param number_of_splits The number of windows to have.
     * @param func_val_to_int A function mapping a value to a window.  The output of this is modulo number_of_splits.
     * @return A reference to the window encapsulating all window output streams.
     */
    Window<OUTPUT>* last(std::chrono::duration<double> duration, int number_of_splits, int (*func_val_to_int)(OUTPUT)) {
        Window<OUTPUT>* window = new Window<OUTPUT>(duration, number_of_splits, func_val_to_int);
        this->subscribe(window);
        return window;
    }


    /**
     * Sinks data into the network.
     * @param topology
     * @param stream_id The name of the stream where data should be published.
     * @param val_to_bytes This function should return a pointer to a region of memory, and its size.  The region of memory
     * is freed upon data transmission.
     * @return
     */
    NetworkSink<OUTPUT>* networkSink(Topology *topology, const char *stream_id, std::pair<uint32_t, void*> (*val_to_bytes) (OUTPUT)) {
        auto *networkSink = new NetworkSink<OUTPUT>(topology, stream_id, val_to_bytes);
        this->subscribe(networkSink);
        return networkSink;
    }

    virtual ~TwoTypeStream() {}
};

#endif