#ifndef _SUBSCRIBEABLE_H_
#define _SUBSCRIBEABLE_H_

#include <list>
#include "Subscriber.h"

template <typename T>
class Subscribeable {
    std::list< Subscriber<T>* > subscribers;

protected:
    void publish(T value);

public:
    void subscribe(Subscriber<T>* subscriber);
};

#include "Subscribeable.cpp"
#endif
