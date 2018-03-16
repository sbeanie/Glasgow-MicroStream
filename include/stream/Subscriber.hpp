#ifndef _SUBSCRIBER_H_
#define _SUBSCRIBER_H_

#include "Subscribeable.hpp"

namespace NAMESPACE_NAME {

    template<typename T>
    class Subscribeable;

    class CascadeDeleteable {

    public:
        virtual bool delete_and_notify() = 0;
    };

    template<typename T>
    class Subscriber : public CascadeDeleteable {

    public:
        virtual void receive(T) = 0;

//    virtual void notify_subscribeable_deleted(Subscribeable<T>*) {std::cout << "Default called" << std::endl;}
        virtual void notify_subscribeable_deleted(Subscribeable<T> *) = 0;

        virtual void add_subscribeable(Subscribeable<T> *) = 0;
    };
}
#endif
