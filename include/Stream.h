#ifndef _STREAM_H_
#define _STREAM_H_

#include <iostream>
#include <type_traits>
#include <thread>         // std::thread
#include <mutex>          // std::mutex, std::lock_guard
#include <boost/date_time.hpp>
#include <list>

template <typename T>
class Subscriber {

public:
    virtual void receive(T) {};
};


template <typename T>
class Subscribeable {
    std::list< Subscriber<T>* > subscribers;

protected:
    void publish(T value) {
        for (typename std::list<Subscriber<T>*>::iterator subscribersIterator = subscribers.begin();
                subscribersIterator != subscribers.end();
                subscribersIterator++) {
            (*subscribersIterator)->receive(value);
        }
    }

public:
    void subscribe(Subscriber<T>* subscriber) {
        subscribers.push_back(subscriber);
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
class TwoTypeStream;

template <typename T>
class Stream: public TwoTypeStream<T, T> {};

template <typename T>
class FilterStream;

template <typename INPUT, typename OUTPUT>
class MapStream;

template <typename INPUT>
class SplitStream;

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


    
        ~TwoTypeStream() {
        }
};


template <typename T>
class Window: public Stream<T> {

    std::mutex mtx;

    long duration_in_millis;
    int (*func_val_to_int) (T);
    Stream<T>** streams;
    std::list<T> values;

public:
    Window(long duration_in_millis, int (*func_val_to_int)(T), Stream<T>** streams)
        : duration_in_millis(duration_in_millis), func_val_to_int(func_val_to_int), streams(streams) {};

    void receive(T value) {
        std::lock_guard<std::mutex> lck(mtx);
        std::cout << "Got value: " << value << std::endl;
    }

};

template <typename INPUT>
class SplitStream: public Stream<INPUT> {
    int (*split_function) (INPUT);
    int num_streams;
    Stream<INPUT>** streams;
public:
    SplitStream(int (*split_function)(INPUT), int num_streams, Stream<INPUT>** streams) 
        : split_function(split_function), num_streams(num_streams), streams(streams) {};

    void receive(INPUT value) {
        int split_stream_num = split_function(value);
        streams[split_stream_num % num_streams]->receive(value);
    }
};

template <typename INPUT, typename OUTPUT>
class MapStream: public TwoTypeStream<INPUT, OUTPUT> {
    OUTPUT (*map_function)(INPUT);
public:
    MapStream(OUTPUT (*map_function)(INPUT)) : map_function(map_function) {};
    
    virtual void receive(INPUT value) {
        this->publish(map_function(value));
    }
};

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
