#ifndef _STREAM_TYPES_H_
#define _STREAM_TYPES_H_

#include <list>

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

template <typename T>
class SplitStream;

#endif