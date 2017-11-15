#ifndef _STREAM_H_
#define _STREAM_H_

#include <iostream>
#include "LinkedList.h"

template <typename T>
class Subscriber {

public:
    virtual void receive(T) {};
};


template <typename T>
class Subscribeable {
    LinkedList< Subscriber<T>* > subscribers;

protected:
    void publish(T value) {
        subscribers.for_each([=](Subscriber<T>* subscriber){subscriber->receive(value);});
    }

public:
    void subscribe(Subscriber<T>* subscriber) {
        subscribers.add(subscriber);
    };
};

template <typename T>
class Sink : public Subscriber<T> {
    
        void (*sink_function)(T);
    
    public:
    
        Sink(void (*sink_function)(T)) : sink_function(sink_function){};
    
        void receive(T value) {
            sink_function(value);
        }
};


template <typename INPUT, typename OUTPUT>
class Stream;

template <typename INPUT, typename OUTPUT>
class FilterStream;

template <typename INPUT, typename OUTPUT>
class MapStream;

template <typename INPUT, typename OUTPUT>
class SplitStream;

template <typename INPUT, typename OUTPUT>
class Stream : public Subscriber<INPUT>, public Subscribeable<OUTPUT> {
        
    public:
    
        virtual void receive(INPUT value) {
            this->publish(value);
        }
    
        Sink<OUTPUT>* sink(void (*sink_function)(OUTPUT)) {
            Sink<OUTPUT>* sink = new Sink<OUTPUT>(sink_function);
            this->subscribe(sink);
            return sink;
        }

        FilterStream<OUTPUT, OUTPUT>* filter(bool (*filter_function)(OUTPUT)) {
            FilterStream<OUTPUT, OUTPUT>* filter_stream = new FilterStream<OUTPUT, OUTPUT>(filter_function);
            this->subscribe(filter_stream);
            return filter_stream;
        }

        template <typename X>
        MapStream<OUTPUT, X>* map(X (*map_function)(OUTPUT)) {
            MapStream<OUTPUT, X>* map_stream = new MapStream<OUTPUT, X>(map_function);
            this->subscribe(map_stream);
            return map_stream;
        }

        Stream<OUTPUT, OUTPUT>** split(int num_streams, int (*split_function)(OUTPUT)) {
            Stream<OUTPUT, OUTPUT>** streams = new Stream<OUTPUT, OUTPUT>*[num_streams];

            for (int i = 0; i < num_streams; i++) {
                streams[i] = new Stream<OUTPUT, OUTPUT>();
            }

            SplitStream<OUTPUT, OUTPUT>* split_stream = new SplitStream<OUTPUT, OUTPUT>(split_function, num_streams, streams);

            this->subscribe(split_stream);
            return streams;
        }

        template <typename X>
        Stream<OUTPUT, OUTPUT>* union_streams(int num_streams, Stream<X, OUTPUT>** streams) {
            Stream* union_stream = new Stream();
            this->subscribe(union_stream);
            for (int i = 0; i < num_streams; i++) {
                streams[i]->subscribe(union_stream);
            }
            return union_stream;
        }
    
        ~Stream() {
        }
};

template <typename INPUT, typename OUTPUT>
class SplitStream: public Stream<INPUT, OUTPUT> {
    int (*split_function) (INPUT);
    int num_streams;
    Stream<OUTPUT, OUTPUT>** streams;
public:
    SplitStream(int (*split_function)(INPUT), int num_streams, Stream<OUTPUT, OUTPUT>** streams) 
        : split_function(split_function), streams(streams), num_streams(num_streams) {};

    void receive(INPUT value) {
        int split_stream_num = split_function(value);
        streams[split_stream_num % num_streams]->receive(value);
    }
};

template <typename INPUT, typename OUTPUT>
class MapStream: public Stream<INPUT, OUTPUT> {
    OUTPUT (*map_function)(INPUT);
public:
    MapStream(OUTPUT (*map_function)(INPUT)) : map_function(map_function) {};
    
    virtual void receive(INPUT value) {
        this->publish(map_function(value));
    }
};

template <typename INPUT, typename OUTPUT>
class FilterStream: public Stream<INPUT, OUTPUT> {
    bool (*filter_function)(INPUT);
public:
    FilterStream(bool (*filter_function)(INPUT)) : filter_function(filter_function) {};
    void receive(INPUT value) {
        if (filter_function(value)) 
            this->publish(value);
    }
};


#endif
