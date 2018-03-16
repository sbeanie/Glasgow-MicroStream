#include "Window.hpp"

namespace NAMESPACE_NAME {

    template<typename T>
    TimestampedValue<T> *Window<T>::earliest_t_val() {
        TimestampedValue<T> *first_t_val = nullptr;

        std::lock_guard<std::recursive_mutex> lock(values_lock);
        for (auto listIterator = this->values.begin(); listIterator != this->values.end(); listIterator++) {
            if ((*listIterator)->empty()) continue;
            TimestampedValue<T> *t_val = (*listIterator)->front();
            if (first_t_val == nullptr) {
                first_t_val = t_val;
            } else if (t_val->timePoint < first_t_val->timePoint) {
                first_t_val = t_val;
            }
        }
        return first_t_val;
    }

    template<typename T>
    void Window<T>::remove_and_send(TimestampedValue<T> *t_val) {
        int split = this->func_val_to_int(t_val->value) % this->number_of_splits;
        std::lock_guard<std::recursive_mutex> lock(values_lock);
        std::list<TimestampedValue<T> *> *list = values.at(split);
        for (auto it = list->begin(); it != list->end(); it++) {
            if (*it == t_val) {
                list->erase(it);
                break;
            }
        }

        int split_number = t_val->split_number;
        delete (t_val);
        send(split_number);
    }

    template<typename T>
    void Window<T>::send(int split_number) {
        std::lock_guard<std::recursive_mutex> lock(values_lock);
        std::list<TimestampedValue<T> *> *t_val_list = values.at(split_number);

        std::list<T> val_list;
        for (auto valueIterator = t_val_list->begin(); valueIterator != t_val_list->end(); valueIterator++) {
            val_list.push_back((*valueIterator)->value);
        }

        this->publish(std::pair<int, std::list<T> >(split_number, val_list));
    }

    template<typename T>
    void Window<T>::publish(std::pair<int, std::list<T> > keyValuePair) {
        for (auto subscribersIterator = this->subscribers.begin();
             subscribersIterator != this->subscribers.end(); subscribersIterator++) {
            (*subscribersIterator)->receive(keyValuePair);
        }
    }

    template<typename T>
    void Window<T>::run() {
        while (should_run) {
            TimestampedValue<T> *earliest_t_val = this->earliest_t_val();
            if (earliest_t_val != nullptr) {

                auto future_wake_time = earliest_t_val->timePoint + duration;
                std::chrono::system_clock::time_point time_now = std::chrono::system_clock::now();

                if (future_wake_time < time_now) {
                    remove_and_send(earliest_t_val);
                    continue;
                }

                std::chrono::duration<double> sleep_duration = future_wake_time - time_now;

                std::this_thread::sleep_for(sleep_duration);
                remove_and_send(earliest_t_val);

            } else {
                std::this_thread::sleep_for(duration / 2);
            }
        }
    }

    template<typename T>
    void Window<T>::receive(T value) {
        std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::now();
        auto *t_val = new TimestampedValue<T>(value, timePoint, func_val_to_int(value) % number_of_splits);

        std::lock_guard<std::recursive_mutex> lock(values_lock);
        (*values.at(t_val->split_number)).push_back(t_val);
        send(t_val->split_number);
        if (!this->thread_started) {
            this->thread_started = true;
            this->thread = std::thread(&Window<T>::run, this);
        }
    }

}
