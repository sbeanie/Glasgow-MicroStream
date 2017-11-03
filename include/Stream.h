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


template <typename T>
class Stream : public Subscriber<T>, public Subscribeable<T> {
        
    public:
    
        virtual void receive(T value) {
            this->publish(value);
        }
    
        Sink<T>* sink(void (*sink_function)(T)) {
            Sink<T>* sink = new Sink<T>(sink_function);
            this->subscribe(sink);
            return sink;
        }

        class FilterStream;

        FilterStream* filter(bool (*filter_function)(T)) {
            FilterStream* filter_stream = new FilterStream(filter_function);
            this->subscribe(filter_stream);
            return filter_stream;
        }
    
        ~Stream() {
        }
};

template <typename T>
class Stream<T>::FilterStream: public Stream<T> {
    bool (*filter_function)(T);
public:
    FilterStream(bool (*filter_function)(T)) : filter_function(filter_function) {};
    void receive(T value) {
        if (filter_function(value)) {
            this->publish(value);
        }
    }
};


#endif