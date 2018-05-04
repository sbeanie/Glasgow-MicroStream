#ifndef _TWO_TYPE_STREAM_H_
#define _TWO_TYPE_STREAM_H_

#include "StreamTypes.hpp"
#include "Subscribeable.hpp"
#include "Subscriber.hpp"
#include <chrono>
#include <vector>

namespace glasgow_ustream {

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class TwoTypeStream : public Subscriber<INPUT_TYPE>, public Subscribeable<OUTPUT_TYPE> {

    protected:

#ifndef UNSAFE_TOPOLOGY_MODIFICATION
        std::recursive_mutex subscribers_lock;
        std::recursive_mutex subscribeables_lock;
#endif
        std::list<Subscriber<OUTPUT_TYPE> *> subscribers; // A list of nodes subscribed to this stream.
        std::list<Subscribeable<INPUT_TYPE> *> subscribeables;  // A list of publishers that this stream is subscribed to.

    public:

        /**
         * Sinks any output that is published by this stream into the function provided.
         * @param sink_function A function that takes a value of the type of the stream and returns nothing.
         * @return A reference to the sink stream.  Note sink streams do not publish data.
         */
        Sink<OUTPUT_TYPE> *sink(void (*sink_function)(OUTPUT_TYPE)) {
            Sink<OUTPUT_TYPE> *sink = new Sink<OUTPUT_TYPE>(sink_function);
            this->subscribe(sink);
            return sink;
        }

        /**
         * Filters values from the stream.
         * @param filter_function The filter function.  Returning true means the value is kept.
         * @return A reference to the filtered stream.
         */
        FilterStream<OUTPUT_TYPE> *filter(bool (*filter_function)(OUTPUT_TYPE)) {
            FilterStream<OUTPUT_TYPE> *filter_stream = new FilterStream<OUTPUT_TYPE>(filter_function);
            this->subscribe(filter_stream);
            return filter_stream;
        }

        /**
         * Maps the current stream's output to a stream of type X.
         * @tparam X The type of the new stream.
         * @param map_function A function mapping a value from the current stream to a value of the new type.
         * @return A reference to the mapped stream.
         */
        template<typename X>
        MapStream<OUTPUT_TYPE, X> *map(X (*map_function)(OUTPUT_TYPE)) {
            MapStream<OUTPUT_TYPE, X> *map_stream = new MapStream<OUTPUT_TYPE, X>(map_function);
            this->subscribe(map_stream);
            return map_stream;
        }

        /**
         * Maps the current stream's output to a stream of type X, with the option of maintaining state.
         * @tparam X The type of the output.
         * @param statefulMap A pointer to an object that implements the StatefulMap interface.
         * @return A reference to the StatefulStream.
         */
        template<typename X>
        StatefulStream<OUTPUT_TYPE, X> *map_stateful(StatefulMap<OUTPUT_TYPE, X> *statefulMap) {
            StatefulStream<OUTPUT_TYPE, X> *statefulStream = new StatefulStream<OUTPUT_TYPE, X>(statefulMap);
            this->subscribe(statefulStream);
            return statefulStream;
        }

        /**
         * Splits the current stream into several streams based on the output of a splitting function.
         * @param num_streams The number of streams the output should be split into.
         * @param split_function The function mapping a value to a stream.  The output value is modulo num_streams.
         * @return The list of streams the split function will push data into.
         */
        std::vector<Stream<OUTPUT_TYPE> *> split(int num_streams, int (*split_function)(OUTPUT_TYPE)) {
            std::vector<Stream<OUTPUT_TYPE> *> streams;

            for (int i = 0; i < num_streams; i++) {
                streams.push_back(new Stream<OUTPUT_TYPE>());
            }

            SplitStream<OUTPUT_TYPE> *split_stream = new SplitStream<OUTPUT_TYPE>(split_function, num_streams, streams);
            this->subscribe(split_stream);
            return streams;
        }

        /**
         * Combines two streams into a single stream.
         * @param streams A list of streams all of the same output type.
         * @return A reference to a stream that publishes the combined output of all unioned streams.
         */
        Stream<OUTPUT_TYPE> *union_streams(std::list<Subscribeable<OUTPUT_TYPE> *> streams) {
            auto *union_stream = new Stream<OUTPUT_TYPE>();
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
        Window<OUTPUT_TYPE> *
        last(std::chrono::duration<double> duration, int number_of_splits, int (*func_val_to_int)(OUTPUT_TYPE)) {
            Window<OUTPUT_TYPE> *window = new Window<OUTPUT_TYPE>(duration, number_of_splits, func_val_to_int);
            this->subscribe(window);
            return window;
        }

        /**
         * Creates a window of the current stream where values are preserved for the time specified in the duration.
         * @param duration The amount of times values should remain in the window.
         * @return A reference to the window.  This method does not allow you to split at the same time.
         */
        Window<OUTPUT_TYPE> *last(std::chrono::duration<double> duration) {
            Window<OUTPUT_TYPE> *window = new Window<OUTPUT_TYPE>(duration);
            this->subscribe(window);
            return window;
        }

        /**
         * Creates a tail of the current stream for which the last "tail_size" values to pass through the stream are
         * published as an ordered list whenever the tail of the stream changes.
         * @param tail_size The number of elements to include in the tail.
         * @return A reference to the tail stream.
         */
        TailStream<OUTPUT_TYPE> *tail(unsigned int tail_size) {
            if (tail_size < 1) return nullptr;
            TailStream<OUTPUT_TYPE>* tail_stream = new TailStream<OUTPUT_TYPE>(tail_size);
            this->subscribe(tail_stream);
            return tail_stream;
        }


        /**
         * Sinks data into the network via the peer discovery protocol.
         * @param topology
         * @param stream_id The name of the stream where data should be published.
         * @param val_to_bytes This function should return a pointer to a region of memory, and its size.  The region of memory
         * is freed upon data transmission.
         * @return
         */
        NetworkSink<OUTPUT_TYPE> *
        networkSink(Topology *topology, const char *stream_id, std::pair<uint32_t, void *> (*val_to_bytes)(OUTPUT_TYPE)) {
            auto *networkSink = new NetworkSink<OUTPUT_TYPE>(topology, stream_id, val_to_bytes);
            this->subscribe(networkSink);
            return networkSink;
        }

#ifdef COMPILE_WITH_BOOST_SERIALIZATION

        /**
         * If boost serialization is enabled via the compilation flag COMPILE_WITH_BOOST_SERIALIZATION, this method can
         * be used to make serialization/deserialization of types/classes more simple.
         * @param topology The topology reference.
         * @param stream_id The stream identifier stream values should be published to via the peer discovery protocol.
         * @return a reference to a NetworkSink object.
         */
        NetworkSink<OUTPUT_TYPE> *boostSerializedNetworkSink(Topology *topology, const char *stream_id) {
            auto *networkSink = new BoostSerializedNetworkSink<OUTPUT_TYPE>(topology, stream_id);
            this->subscribe(networkSink);
            return networkSink;
        }

#endif

        /**
         * Publishes the provided value to all subscribers of this stream.
         * @param value the value to be published.
         */
        virtual void publish(OUTPUT_TYPE value) override {
#ifndef UNSAFE_TOPOLOGY_MODIFICATION
            std::lock_guard<std::recursive_mutex> lock(subscribers_lock);
#endif
            for (auto subscribersIterator = subscribers.begin();
                 subscribersIterator != subscribers.end(); subscribersIterator++) {
                (*subscribersIterator)->receive(value);
            }
        }

        void unsubscribe(Subscriber<OUTPUT_TYPE> *) override;
        void subscribe(Subscriber<OUTPUT_TYPE> *) override;
        void notify_subscribeable_deleted(Subscribeable<INPUT_TYPE> *) override;
        bool delete_and_notify() override;
        void add_subscribeable(Subscribeable<INPUT_TYPE> *) override;

        virtual ~TwoTypeStream() {}
    };

}

#include "TwoTypeStream.cpp"
#endif