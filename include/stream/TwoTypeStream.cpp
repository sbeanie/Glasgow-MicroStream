#include "TwoTypeStream.hpp"

namespace glasgow_ustream {

    template <typename INPUT_TYPE, typename OUTPUT_TYPE>
    void TwoTypeStream<INPUT_TYPE, OUTPUT_TYPE>::unsubscribe(Subscriber<OUTPUT_TYPE> *subscriber) {
#ifndef UNSAFE_TOPOLOGY_MODIFICATION
        std::lock_guard<std::recursive_mutex> lock(subscribers_lock);
#endif
        for (auto subscribersIterator = subscribers.begin();
             subscribersIterator != subscribers.end(); subscribersIterator++) {
            if (*subscribersIterator == subscriber) {
                subscribers.remove(*subscribersIterator);
                return;
            }
        }
    }

    template <typename INPUT_TYPE, typename OUTPUT_TYPE>
    void TwoTypeStream<INPUT_TYPE,OUTPUT_TYPE>::subscribe(Subscriber<OUTPUT_TYPE> *subscriber) {
#ifndef UNSAFE_TOPOLOGY_MODIFICATION
        std::lock_guard<std::recursive_mutex> lock(subscribers_lock);
#endif
        subscribers.push_back(subscriber);
        subscriber->add_subscribeable((Subscribeable<OUTPUT_TYPE> *) this);
    }

    template <typename INPUT_TYPE, typename OUTPUT_TYPE>
    void TwoTypeStream<INPUT_TYPE,OUTPUT_TYPE>::notify_subscribeable_deleted(Subscribeable<INPUT_TYPE> *subscribeable) {
        {
#ifndef UNSAFE_TOPOLOGY_MODIFICATION
            std::lock_guard<std::recursive_mutex> lock(subscribeables_lock);
#endif
            for (auto subscribeableIterator = subscribeables.begin();
                 subscribeableIterator != subscribeables.end(); subscribeableIterator++) {
                if (*subscribeableIterator == subscribeable) {
                    subscribeables.remove(*subscribeableIterator);
                    break;
                }
            }
            if (subscribeables.size() != 0) return;
        }
        delete_and_notify();
    }

    template <typename INPUT_TYPE, typename OUTPUT_TYPE>
    bool TwoTypeStream<INPUT_TYPE,OUTPUT_TYPE>::delete_and_notify() {
        {
#ifndef UNSAFE_TOPOLOGY_MODIFICATION
            std::lock_guard<std::recursive_mutex> lock_subscribeables(subscribeables_lock);
#endif
            if (subscribeables.size() != 0) return false;

#ifndef UNSAFE_TOPOLOGY_MODIFICATION
            std::lock_guard<std::recursive_mutex> lock_subscribers(subscribers_lock);
#endif
            for (auto subscribersIterator = subscribers.begin();
                 subscribersIterator != subscribers.end(); subscribersIterator++) {
                Subscribeable<OUTPUT_TYPE> *subscribeable = this;
                (*subscribersIterator)->notify_subscribeable_deleted(subscribeable);
            }
        }
        delete (this);
        return true;
    }

    template <typename INPUT_TYPE, typename OUTPUT_TYPE>
    void TwoTypeStream<INPUT_TYPE,OUTPUT_TYPE>::add_subscribeable(Subscribeable<INPUT_TYPE> *subscribeable) {
#ifndef UNSAFE_TOPOLOGY_MODIFICATION
        std::lock_guard<std::recursive_mutex> lock(subscribeables_lock);
#endif
        subscribeables.push_back(subscribeable);
    }
}