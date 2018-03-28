#ifndef _FILTER_STREAM_H_
#define _FILTER_STREAM_H_

#include "StreamTypes.hpp"

namespace glasgow_ustream {

    template<typename T>
    class FilterStream : public Stream<T> {

        bool (*filter_function)(T);

    public:
        /**
         * Provides functionality to filter values from a stream based on a user-defined lambda function.
         * @param filter_function this function should return false if the value should be removed, true otherwise.
         */
        FilterStream(bool (*filter_function)(T)) : filter_function(filter_function) {};

        void receive(T value) {
            if (filter_function(value))
                this->publish(value);
        }
    };

}

#endif