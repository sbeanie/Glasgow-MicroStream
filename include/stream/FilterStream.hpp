#ifndef _FILTER_STREAM_H_
#define _FILTER_STREAM_H_

#include "StreamTypes.hpp"

template <typename T>
class FilterStream: public Stream<T> {
    bool (*filter_function)(T);
public:
    FilterStream(bool (*filter_function)(T)) : filter_function(filter_function) {};
    void receive(T value) {
        if (filter_function(value)) 
            this->publish(value);
    }
};

#endif