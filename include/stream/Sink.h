#ifndef _SINK_H_
#define _SINK_H_

#include "Subscriber.h"

template <typename T>
class Sink : public Subscriber<T> {
    
        void (*sink_function)(T);
    
    public:
    
        Sink(void (*sink_function)(T)) : sink_function(sink_function){};
    
        void receive(T value) {
            sink_function(value);
        }
};
#endif