#ifndef _WINDOW_H_
#define _WINDOW_H_


#include "StreamTypes.hpp"
#include "WindowBatch.hpp"
#include <ctime>
#include <chrono>
#include <thread>
#include <list>
#include <mutex>

template <class T>
struct TimestampedValue {
    int split_number;
    T value;
    std::chrono::system_clock::time_point timePoint;

    TimestampedValue(T value, std::chrono::system_clock::time_point timePoint, int split_number) : split_number(split_number), value(value), timePoint(timePoint) {};
};

template <typename T>
class Window: public TwoTypeStream<T, std::pair<int, std::shared_ptr<std::list<T> > > > {

    std::thread thread;
    bool should_run, thread_started;
    
    std::chrono::duration<double> duration;

    int number_of_splits;
    int (*func_val_to_int) (T);

    std::recursive_mutex values_lock;
    std::vector<std::list<TimestampedValue<T>* >* > values;
private:

    TimestampedValue<T>* earliest_t_val();

    void remove_and_send(TimestampedValue<T>* t_val);

    void send(int split_number);

    void run();

public:
    Window(std::chrono::duration<double> duration, int number_of_splits, int (*func_val_to_int)(T))
        : duration(duration), number_of_splits(number_of_splits), func_val_to_int(func_val_to_int) {
            this->should_run = true;
            this->thread_started = false;
            for (int i = 0; i < number_of_splits; i++) {
                values.push_back(new std::list<TimestampedValue<T> *>());
            }
        };

    void stop() {
        this->should_run = false;
        this->thread.join();
    }

    template <typename OUTPUT>
    WindowAggregate<T, OUTPUT>* aggregate(OUTPUT (*func_vals_to_val) (std::pair<int, std::shared_ptr<std::list<T> > >)) {
        WindowAggregate<T, OUTPUT>* window_aggregate = new WindowAggregate<T, OUTPUT>(func_vals_to_val);
        this->subscribe(window_aggregate);
        return window_aggregate;
    }

    template <typename OUTPUT>
    WindowBatch<T, OUTPUT>* batch(std::chrono::duration<double> period, OUTPUT (*func_vals_to_val) (std::pair<int, std::shared_ptr<std::list<T> > >)) {
        WindowBatch<T, OUTPUT>* window_batch = new WindowBatch<T, OUTPUT>(period, number_of_splits, func_vals_to_val);
        this->subscribe(window_batch);
        return window_batch;
    }

    void publish(std::pair<int, std::shared_ptr<std::list<T> > > keyValuePair);
    void receive(T value);

    ~Window() {
        for (auto it = values.begin(); it != values.end(); it++) {
            delete(*it);
        }
    }
};

#include "stream/Window.cpp"

#endif
