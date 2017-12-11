#ifndef _SUBSCRIBER_H_
#define _SUBSCRIBER_H_

#include "Subscribeable.hpp"

template <typename T>
class Subscribeable;

template <typename T>
class Subscriber {

public:
    virtual void receive(T) = 0;
//    virtual void notify_subscribeable_deleted(Subscribeable<T>*) {std::cout << "Default called" << std::endl;}
    virtual void notify_subscribeable_deleted(Subscribeable<T>*) = 0;
    virtual void add_subscribeable(Subscribeable<T>*) = 0;
    virtual bool delete_and_notify() = 0;
};
#endif
