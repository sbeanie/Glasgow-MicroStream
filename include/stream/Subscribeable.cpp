#include "Subscribeable.hpp"

template <typename T>
void Subscribeable<T>::publish(T value) {
    for (auto subscribersIterator = subscribers.begin(); subscribersIterator != subscribers.end(); subscribersIterator++) {
        (*subscribersIterator)->receive(value);
    }
}

template <typename T>
void Subscribeable<T>::subscribe(Subscriber<T>* subscriber) {
    subscribers.push_back(subscriber);
}
