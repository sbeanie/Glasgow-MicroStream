#ifndef _STREAM_H_
#define _STREAM_H_

#include <type_traits>
#include <boost/date_time.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <mutex>
#include <condition_variable>
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

template <typename T>
class Window;

template <typename INPUT, typename OUTPUT>
class MapStream;

template <typename T>
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

        Window<OUTPUT>* last(boost::chrono::duration<double> duration, int number_of_splits, int (*func_val_to_int)(OUTPUT)) {
            Window<OUTPUT>* window = new Window<OUTPUT>(duration, number_of_splits, func_val_to_int);
            this->subscribe(window);
            return window;
        }
    
        ~TwoTypeStream() {
        }
};


template <class T>
struct TimestampedValue {
    T value;
    boost::chrono::system_clock::time_point timePoint;

    TimestampedValue(T value, boost::chrono::system_clock::time_point timePoint) : value(value), timePoint(timePoint) {};
};

template <typename T>
class Window: public Stream<T> {

    boost::mutex m;
    boost::condition_variable cv;
    boost::thread thread;
    bool should_run;
    

    boost::chrono::duration<double> duration;

    int number_of_splits;
    int (*func_val_to_int) (T);
    std::vector<std::list<TimestampedValue<T>* > > values;

private:

    TimestampedValue<T>* earliest_t_val() {
        TimestampedValue<T>* first_t_val = nullptr;
        std::cout << "Iterating" << std::endl;

        for (typename std::vector<std::list<TimestampedValue<T>* > >::iterator listIterator = this->values.begin();
                listIterator != this->values.end();
                listIterator++) {
            TimestampedValue<T>* t_val = listIterator->front();
            if (first_t_val == nullptr) {
                first_t_val = t_val;
            } else if (t_val->timePoint < first_t_val->timePoint) {
                first_t_val = t_val;
            }
        }
        return first_t_val;
    }

    void remove_and_send(TimestampedValue<T>* t_val) {
        int split = this->func_val_to_int(t_val->value) % this->number_of_splits;
        std::list<TimestampedValue<T>* > list = values.at(split);
        for (auto it = list.begin(); it != list.end(); it++) {
            if (*it == t_val) {
                it = list.erase(it);
                break;
            }
        }
        T val = t_val->value;
        std::cout << "Removed " << val << " from window." << std::endl;
        delete(t_val);
        send();
    }

    void send() {

    }

    void run() {
        while (should_run) {
            TimestampedValue<T>* earliest_t_val = this->earliest_t_val();
            if (earliest_t_val != nullptr) {
                std::cout << earliest_t_val->value << std::endl;

                auto  future_wake_time = earliest_t_val->timePoint + duration;
                boost::chrono::system_clock::time_point  time_now         = boost::chrono::system_clock::now();

                if (future_wake_time < time_now) continue;

                boost::chrono::duration<double> sleep_duration = future_wake_time - time_now;
                
                boost::this_thread::sleep_for(sleep_duration);
                remove_and_send(earliest_t_val);

            } else {
                boost::this_thread::sleep_for(duration/2);
            }
            this->should_run = false;
        }
    }

public:
    Window(boost::chrono::duration<double> duration, int number_of_splits, int (*func_val_to_int)(T))
        : duration(duration), number_of_splits(number_of_splits), func_val_to_int(func_val_to_int) {
            this->should_run = false;
            this->values.resize(number_of_splits);
        };

    void stop() {
        this->should_run = false;
        this->thread.join();
    }

    void receive(T value) { 

        boost::chrono::system_clock::time_point timePoint = boost::chrono::system_clock::now();
        TimestampedValue<T>* t_val = new TimestampedValue<T>(value, timePoint);
        std::cout << "Timestamped value: " << t_val->value << " - " << t_val->timePoint << std::endl;
        int split = func_val_to_int(value) % number_of_splits;

        values.at(split).push_back(t_val);
        if (this->should_run == false) {
            this->should_run = true;
            this->thread = boost::thread(&Window<T>::run, this);
        }
        this->thread.join();
    }

};

template <typename T>
class SplitStream: public Stream<T> {
    int (*split_function) (T);
    int num_streams;
    Stream<T>** streams;
public:
    SplitStream(int (*split_function)(T), int num_streams, Stream<T>** streams) 
        : split_function(split_function), num_streams(num_streams), streams(streams) {};

    void receive(T value) {
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
