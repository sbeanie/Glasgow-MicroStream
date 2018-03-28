#ifndef _SUBSCRIBER_H_
#define _SUBSCRIBER_H_

#include "Subscribeable.hpp"

namespace glasgow_ustream {

    template<typename T>
    class Subscribeable;

    /**
     * This class is used to simplify the deletion of a topology.  It allows a single deletion call on the topology
     * to delete all nodes within the topology.
     */
    class CascadeDeleteable {

    public:
        /**
         * Delete this class instance and notify subscribers that their publisher has disappeared.
         * This method should only delete the class if it is no longer subscribed to any publishers.
         * @return true if the class was deleted, false if not.
         */
        virtual bool delete_and_notify() = 0;
    };

    template<typename T>
    class Subscriber : public CascadeDeleteable {

    public:

        /**
         * This function is called by a publisher in order to push values to subscribers.
         * @param T the value the publisher is publishing.
         */
        virtual void receive(T) = 0;

        /**
         * This function is called when a publisher has been requested to be deleted.  The publisher notifies
         * subscribers that they have been deleted.  This method should call delete_and_notify if the class is no longer
         * subscribed to any publishers.
         * @param Subscribeable<T> * the publisher that has been deleted.
         */
        virtual void notify_subscribeable_deleted(Subscribeable<T> *) = 0;

        /**
         * This function is called when a class is subcribed to.  Upon successful subscription to a publisher,
         * the publisher notifies the subscriber that it has been subscribed to it.
         * @param Subscribeable<T> * the publisher that this subscriber has been subscribed to.
         */
        virtual void add_subscribeable(Subscribeable<T> *) = 0;

    };
}
#endif
