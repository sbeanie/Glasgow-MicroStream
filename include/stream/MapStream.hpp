#ifndef _MAP_STREAM_H_
#define _MAP_STREAM_H_

#include "StreamTypes.hpp"

namespace glasgow_ustream {

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class MapStream : public TwoTypeStream<INPUT_TYPE, OUTPUT_TYPE> {
        OUTPUT_TYPE (*map_function)(INPUT_TYPE);

    public:
        /**
         * Provides functionality to map a value from one type to the other.  It is not required for the type to change,
         * and can also just be used to alter a value.
         * @param map_function The mapping function that takes a value and returns another, possibly of a different type.
         */
        explicit MapStream(OUTPUT_TYPE (*map_function)(INPUT_TYPE)) : map_function(map_function) {};

        virtual void receive(INPUT_TYPE value) {
            this->publish(map_function(value));
        }
    };

}
#endif
