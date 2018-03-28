#ifndef _SUBSCRIBEABLE_H_
#define _SUBSCRIBEABLE_H_

#include <list>
#include "Subscriber.hpp"
#include <mutex>

namespace glasgow_ustream {

    /**
     * This interface allows classes that implement it to be subscribed to.
     * Subscribers must implement the Subscriber interface and have the same type as this one.
     * @tparam T
     */
    template<typename T>
    class Subscribeable {

    public:
        /**
         * This function should publish the value to all current subsribers.
         * @param T the value to be published.
         */
        virtual void publish(T) {}

        virtual void unsubscribe(Subscriber<T> *) = 0;

        virtual void subscribe(Subscriber<T> *) = 0;
    };
}
#endif
