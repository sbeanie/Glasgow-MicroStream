#ifndef _MAP_STREAM_H_
#define _MAP_STREAM_H_

#include "StreamTypes.hpp"

namespace glasgow_ustream {

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class MapStream : public TwoTypeStream<INPUT_TYPE, OUTPUT_TYPE> {
        OUTPUT_TYPE (*map_function)(INPUT_TYPE);

    public:
        explicit MapStream(OUTPUT_TYPE (*map_function)(INPUT_TYPE)) : map_function(map_function) {};

        virtual void receive(INPUT_TYPE value) {
            this->publish(map_function(value));
        }
    };

}
#endif
