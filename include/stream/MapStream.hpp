#ifndef _MAP_STREAM_H_
#define _MAP_STREAM_H_

#include "StreamTypes.hpp"

template <typename INPUT, typename OUTPUT>
class MapStream: public TwoTypeStream<INPUT, OUTPUT> {
    OUTPUT (*map_function)(INPUT);
public:
    MapStream(OUTPUT (*map_function)(INPUT)) : map_function(map_function) {};
    
    virtual void receive(INPUT value) {
        this->publish(map_function(value));
    }
};

#endif
