#ifndef _STREAM_TYPES_H_
#define _STREAM_TYPES_H_

namespace glasgow_ustream {

    class Topology;

    template<typename T>
    class Sink;

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class TwoTypeStream;

    template<typename T>
    class Stream : public TwoTypeStream<T, T> {
    public:
        virtual void receive(T value) {
            this->publish(value);
        }
    };

    template<typename T>
    class FilterStream;

    template<typename T>
    class Window;

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class MapStream;

    template <typename T>
    class TailStream;

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class WindowAggregate;

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class WindowBatch;

    template<typename T>
    class SplitStream;

    template<typename T>
    class NetworkSink;

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class StatefulStream;

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class StatefulMap;

    template<typename T>
    class NetworkSource;

#ifdef COMPILE_WITH_BOOST_SERIALIZATION
    template <typename T>
    class BoostSerializedNetworkSink;
#endif
}

#endif