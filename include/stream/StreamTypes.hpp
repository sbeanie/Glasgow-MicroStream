#ifndef _STREAM_TYPES_H_
#define _STREAM_TYPES_H_

class Topology;

template <typename T>
class Sink;

template <typename INPUT, typename OUTPUT>
class TwoTypeStream;

template <typename T>
class Stream: public TwoTypeStream<T, T> {
public:
    virtual void receive(T value) {
        this->publish(value);
    }
};

template <typename T>
class FilterStream;

template <typename T>
class Window;

template <typename INPUT, typename OUTPUT>
class MapStream;

template <typename INPUT, typename OUTPUT>
class WindowAggregate;

template <typename INPUT, typename OUTPUT>
class WindowBatch;

template <typename T>
class SplitStream;

template <typename T>
class NetworkSink;

template <typename INPUT, typename OUTPUT>
class StatefulStream;

template <typename INPUT, typename OUTPUT>
class StatefulMap;

template <typename T>
class NetworkSource;

template <typename T>
class BoostSerializedNetworkSink;

#endif