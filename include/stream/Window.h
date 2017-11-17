#ifndef _WINDOW_H_
#define _WINDOW_H_


#include "StreamTypes.h"
#include <boost/date_time.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <list>

template <class T>
struct TimestampedValue {
    T value;
    boost::chrono::system_clock::time_point timePoint;

    TimestampedValue(T value, boost::chrono::system_clock::time_point timePoint) : value(value), timePoint(timePoint) {};
};

template <typename T>
class Window: public Stream<T> {

    boost::thread thread;
    bool should_run;
    
    boost::chrono::duration<double> duration;

    int number_of_splits;
    int (*func_val_to_int) (T);
    std::vector<std::list<TimestampedValue<T>* > > values;

private:

    TimestampedValue<T>* earliest_t_val();

    void remove_and_send(TimestampedValue<T>* t_val);

    void send();

    void run();

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

    void receive(T value);
};

#include "stream/Window.cpp"

#endif
