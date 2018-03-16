#ifndef GU_EDGENT_STATEFULSTREAM_HPP
#define GU_EDGENT_STATEFULSTREAM_HPP

#include "StreamTypes.hpp"
#include "Subscriber.hpp"

namespace NAMESPACE_NAME {

    template<typename INPUT, typename OUTPUT>
    class StatefulMap {
    public:
        virtual OUTPUT apply_stateful_map(INPUT) = 0;
    };

    template<typename INPUT, typename OUTPUT>
    class StatefulStream : public TwoTypeStream<INPUT, OUTPUT> {

    private:

        StatefulMap<INPUT, OUTPUT> *statefulMap;

    public:

        StatefulStream(StatefulMap<INPUT, OUTPUT> *statefulMap) : statefulMap(statefulMap) {
        }

        virtual void receive(INPUT value) {
            this->publish(statefulMap->apply_stateful_map(value));
        }
    };

}
#endif //GU_EDGENT_STATEFULSTREAM_HPP
