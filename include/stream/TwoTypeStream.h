#ifndef _TWO_TYPE_STREAM_H_
#define _TWO_TYPE_STREAM_H_

#include "StreamTypes.h"
#include "Subscribeable.h"
#include "Subscriber.h"
#include "boost/chrono.hpp"

template <typename INPUT, typename OUTPUT>
class TwoTypeStream : public Subscriber<INPUT>, public Subscribeable<OUTPUT> {
        
    public:
    
        virtual void receive(INPUT value) {
            this->publish(value);
        }
    
        Sink<OUTPUT>* sink(void (*sink_function)(OUTPUT)) {
            Sink<OUTPUT>* sink = new Sink<OUTPUT>(sink_function);
            this->subscribe(sink);
            return sink;
        }

        FilterStream<OUTPUT>* filter(bool (*filter_function)(OUTPUT)) {
            FilterStream<OUTPUT>* filter_stream = new FilterStream<OUTPUT>(filter_function);
            this->subscribe(filter_stream);
            return filter_stream;
        }

        template <typename X>
        MapStream<OUTPUT, X>* map(X (*map_function)(OUTPUT)) {
            MapStream<OUTPUT, X>* map_stream = new MapStream<OUTPUT, X>(map_function);
            this->subscribe(map_stream);
            return map_stream;
        }

        Stream<OUTPUT>** split(int num_streams, int (*split_function)(OUTPUT)) {
            Stream<OUTPUT>** streams = new Stream<OUTPUT>*[num_streams];

            for (int i = 0; i < num_streams; i++) {
                streams[i] = new Stream<OUTPUT>();
            }

            SplitStream<OUTPUT>* split_stream = new SplitStream<OUTPUT>(split_function, num_streams, streams);

            this->subscribe(split_stream);
            return streams;
        }

        template<class T>
        Stream<OUTPUT>* union_streams(int num_streams, T** streams) {
            static_assert(std::is_base_of<Subscribeable<OUTPUT>, T>::value, "Passed streams should have matching output type.");

            Stream<OUTPUT>* union_stream = new Stream<OUTPUT>();
            this->subscribe(union_stream);
            for (int i = 0; i < num_streams; i++) {
                streams[i]->subscribe(union_stream);
            }
            return union_stream;
        }

        Window<OUTPUT>* last(boost::chrono::duration<double> duration, int number_of_splits, int (*func_val_to_int)(OUTPUT)) {
            Window<OUTPUT>* window = new Window<OUTPUT>(duration, number_of_splits, func_val_to_int);
            this->subscribe(window);
            return window;
        }
    
        ~TwoTypeStream() {
        }
};

#endif