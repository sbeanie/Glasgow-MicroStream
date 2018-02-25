#ifndef _SUBSCRIBEABLE_H_
#define _SUBSCRIBEABLE_H_

#include <list>
#include "Subscriber.hpp"
#include <mutex>

template <typename T>
class Subscribeable {

public:
    virtual void publish(T) {}

    virtual void unsubscribe(Subscriber<T>*) = 0;

    virtual void subscribe(Subscriber<T>*) = 0;
};
#endif
