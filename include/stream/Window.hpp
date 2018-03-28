#ifndef _WINDOW_H_
#define _WINDOW_H_


#include "StreamTypes.hpp"
#include "WindowBatch.hpp"
#include <ctime>
#include <chrono>
#include <thread>
#include <list>
#include <mutex>

namespace glasgow_ustream {

    /**
     * This class is a container used by time-based operations in order to manage values.
     * @tparam T
     */
    template<class T>
    struct TimestampedValue {
        int split_number;
        T value;
        std::chrono::system_clock::time_point timePoint;

        TimestampedValue(T value, std::chrono::system_clock::time_point timePoint, int split_number) : split_number(
                split_number), value(value), timePoint(timePoint) {};
    };

    template<typename T>
    class Window : public TwoTypeStream<T, std::pair<int, std::list<T> > > {

        std::thread thread;
        bool should_run, thread_started;

        std::chrono::duration<double> duration;

        int number_of_splits;

        int (*func_val_to_int)(T);

        std::recursive_mutex values_lock;
        std::vector<std::list<TimestampedValue<T> *> *> values;
    private:

        TimestampedValue<T> *earliest_t_val();

        void remove_and_send(TimestampedValue<T> *t_val);

        void send(int split_number);

        void run();

        int (*one_split_func) (T) = [] (T) {
            return 0;
        };

    public:

        /**
         * Constructs a window with the specified duration, but with only one output stream.
         * @param duration the amount of time a value remains in the window.
         */
        Window(std::chrono::duration<double> duration)
                : duration(duration), number_of_splits(1) {
            this->func_val_to_int = one_split_func;
            this->should_run = true;
            this->thread_started = false;
            for (int i = 0; i < number_of_splits; i++) {
                values.push_back(new std::list<TimestampedValue<T> *>());
            }
        };


        /**
         * Constructs a window with the specified duration.  The window will map values to an output stream based on the
         * returned value by the func_val_to_int.
         * @param duration the amount of time a value remains in the window.
         * @param number_of_splits the number of output streams.
         * @param func_val_to_int a function that maps a value to an output stream.  The returned value is modulo'd the
         * number of splits.
         */
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

        /**
         * Aggregates the contents of a window using an aggregation function.
         * @tparam OUTPUT_TYPE The output of the aggregation function.
         * @param func_vals_to_val A function accepting a list of values of the type of the current stream that returns a value
         * of type OUTPUT.
         * @return A reference to the WindowAggregate stream.
         */
        template<typename OUTPUT_TYPE>
        WindowAggregate<T, OUTPUT_TYPE> *aggregate(OUTPUT_TYPE (*func_vals_to_val)(std::pair<int, std::list<T> >)) {
            WindowAggregate<T, OUTPUT_TYPE> *window_aggregate = new WindowAggregate<T, OUTPUT_TYPE>(func_vals_to_val);
            this->subscribe(window_aggregate);
            return window_aggregate;
        }

        /**
         * Batches the output of a window.
         * @tparam OUTPUT_TYPE
         * @param period The amount of time before publishing the window contents again.
         * @param func_vals_to_val An aggregation function to apply to the batched window contents that returns type OUTPUT.
         * @return
         */
        template<typename OUTPUT_TYPE>
        WindowBatch<T, OUTPUT_TYPE> *
        batch(std::chrono::duration<double> period, OUTPUT_TYPE (*func_vals_to_val)(std::pair<int, std::list<T> >)) {
            WindowBatch<T, OUTPUT_TYPE> *window_batch = new WindowBatch<T, OUTPUT_TYPE>(period, number_of_splits,
                                                                              func_vals_to_val);
            this->subscribe(window_batch);
            return window_batch;
        }

        void publish(std::pair<int, std::list<T> > keyValuePair);

        void receive(T value);

        bool delete_and_notify() override {
            if (this->subscribeables.size() != 0) return false;
            for (auto subscribersIterator = this->subscribers.begin();
                 subscribersIterator != this->subscribers.end(); subscribersIterator++) {
                Subscribeable<std::pair<int, std::list<T> > > *subscribeable = this;
                (*subscribersIterator)->notify_subscribeable_deleted(subscribeable);
            }
            this->stop();
            delete (this);
            return true;
        }

        ~Window() {
            for (auto it = values.begin(); it != values.end(); it++) {
                delete (*it);
            }
        }
    };
}

#include "Window.cpp"

#endif
