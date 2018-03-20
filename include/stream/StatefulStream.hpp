#ifndef GU_EDGENT_STATEFULSTREAM_HPP
#define GU_EDGENT_STATEFULSTREAM_HPP

#include "StreamTypes.hpp"
#include "Subscriber.hpp"

namespace NAMESPACE_NAME {

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class StatefulMap {
    public:
        virtual OUTPUT_TYPE apply_stateful_map(INPUT_TYPE) = 0;
    };

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class StatefulStream : public TwoTypeStream<INPUT_TYPE, OUTPUT_TYPE> {

    private:

        StatefulMap<INPUT_TYPE, OUTPUT_TYPE> *statefulMap;

    public:

        StatefulStream(StatefulMap<INPUT_TYPE, OUTPUT_TYPE> *statefulMap) : statefulMap(statefulMap) {
        }

        virtual void receive(INPUT_TYPE value) {
            this->publish(statefulMap->apply_stateful_map(value));
        }
    };

}
#endif //GU_EDGENT_STATEFULSTREAM_HPP
