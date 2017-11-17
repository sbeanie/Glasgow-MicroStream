#include "Window.h"

template <typename T>
TimestampedValue<T>* Window<T>::earliest_t_val() {
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

template <typename T>
void Window<T>::remove_and_send(TimestampedValue<T>* t_val) {
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

template <typename T>
void Window<T>::send() {
}

template <typename T>
void Window<T>::run() {
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

template <typename T>
void Window<T>::receive(T value) { 
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


