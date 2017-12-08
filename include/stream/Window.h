#ifndef _WINDOW_H_
#define _WINDOW_H_


#include "StreamTypes.h"
#include <boost/date_time.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <list>

template <class T>
struct TimestampedValue {
    int split_number;
    T value;
    boost::chrono::system_clock::time_point timePoint;

    TimestampedValue(T value, boost::chrono::system_clock::time_point timePoint, int split_number) : value(value), timePoint(timePoint), split_number(split_number) {};
};

template <typename T>
class Window: public TwoTypeStream<T, std::pair<int, std::list<T>* > > {

    boost::thread thread;
    bool should_run, thread_started;
    
    boost::chrono::duration<double> duration;

    int number_of_splits;
    int (*func_val_to_int) (T);
    std::vector<std::list<TimestampedValue<T>* >* > values; // TODO Access to this needs to be made synchronized

private:

    TimestampedValue<T>* earliest_t_val();

    void remove_and_send(TimestampedValue<T>* t_val);

    void send(int split_number);

    void run();

public:
    Window(boost::chrono::duration<double> duration, int number_of_splits, int (*func_val_to_int)(T))
        : duration(duration), number_of_splits(number_of_splits), func_val_to_int(func_val_to_int) {
            this->should_run = true;
            this->thread_started = false;
            for (int i = 0; i < number_of_splits; i++) {
                values.push_back(new std::list<TimestampedValue<T> *>());
            }
            std::cout << "Constructed window" << std::endl;
        };

    void stop() {
        this->should_run = false;
        this->thread.join();
    }

    template <typename OUTPUT>
    WindowAggregate<T, OUTPUT>* aggregate(OUTPUT (*func_vals_to_val) (std::pair<int, std::list<T>* >)) {
        WindowAggregate<T, OUTPUT>* window_aggregate = new WindowAggregate<T, OUTPUT>(func_vals_to_val);
        this->subscribe(window_aggregate);
        return window_aggregate;
    };

    void receive(T value);
};

#include "stream/Window.cpp"

#endif
