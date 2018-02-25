#ifndef _WINDOW_BATCH_H_
#define _WINDOW_BATCH_H_

#include "StreamTypes.hpp"
#include <ctime>
#include <vector>
#include <chrono>
#include <thread>
#include <list>

template <typename T, typename OUTPUT>
class WindowBatch: public TwoTypeStream<std::pair<int, std::list<T> >,  OUTPUT > {

    std::thread thread;
    bool should_run, thread_started;

    std::chrono::duration<double> duration;

    int number_of_splits;
    OUTPUT (*func_vals_to_val) (std::pair<int, std::list<T> >);
    std::recursive_mutex values_lock;
    std::vector<std::list<T> > values;

private:

    void run() {
        while (should_run) {
            std::this_thread::sleep_for(this->duration);

            std::lock_guard<std::recursive_mutex> lock(values_lock);
            for (int i = 0; i < number_of_splits; i++) {
                std::list<T> value_list = values.at(i);
                auto key_value_pair = std::pair<int, std::list<T> >(i, value_list);
                this->publish(func_vals_to_val(key_value_pair));
            }
        }
    }

public:
    WindowBatch(std::chrono::duration<double> duration, int number_of_splits, OUTPUT (*func_vals_to_val) (std::pair<int, std::list<T> >))
            : duration(duration), number_of_splits(number_of_splits), func_vals_to_val(func_vals_to_val) {
        this->should_run = true;
        this->thread_started = false;

        std::lock_guard<std::recursive_mutex> lock(values_lock);
        for (int i = 0; i < number_of_splits; i++) {
            std::list<T> new_list;
            values.push_back(std::move(new_list));
        }
    };

    void stop() {
        this->should_run = false;
        this->thread.join();
    }

    void receive(std::pair<int, std::list<T> > key_value_pair) {
        std::lock_guard<std::recursive_mutex> lock(values_lock);
        this->values[key_value_pair.first] = key_value_pair.second;
        if ( ! this->thread_started) {
            this->thread_started = true;
            this->thread = std::thread(&WindowBatch<T, OUTPUT>::run, this);
        }
    }

    bool delete_and_notify() override {
        if (this->subscribeables.size() != 0) return false;
        auto subscribeable = (Subscribeable<OUTPUT> *) this;
        for (auto subscribersIterator = this->subscribers.begin(); subscribersIterator != this->subscribers.end(); subscribersIterator++) {
            (*subscribersIterator)->notify_subscribeable_deleted(subscribeable);
        }
        this->stop();
        delete(this);
        return true;
    }
};

#endif //_WINDOW_BATCH_H_
