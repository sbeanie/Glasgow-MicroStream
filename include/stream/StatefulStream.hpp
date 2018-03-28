#ifndef GU_EDGENT_STATEFULSTREAM_HPP
#define GU_EDGENT_STATEFULSTREAM_HPP

#include "StreamTypes.hpp"
#include "Subscriber.hpp"

namespace glasgow_ustream {

    /**
     * An interface that must be implemented by an object to create a StatefulStream operation.
     * @tparam INPUT_TYPE
     * @tparam OUTPUT_TYPE
     */
    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class StatefulMap {
    public:
        
        /**
         * This method should accept a value of one type, and return a value of another if required.
         * It may make use of any class methods/variables belonging to the implementing class.
         * @return
         */
        virtual OUTPUT_TYPE apply_stateful_map(INPUT_TYPE) = 0;
    };

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class StatefulStream : public TwoTypeStream<INPUT_TYPE, OUTPUT_TYPE> {

    private:

        StatefulMap<INPUT_TYPE, OUTPUT_TYPE> *statefulMap;

    public:

        /**
         * A stateful stream operation allows an external class object implementing StatefulMap to process a value
         * and return a value, potentially of a different type, to be forwarded to subscribers of this node.
         * @param statefulMap A pointer to a class object implementing the StatefulMap interface.
         */
        explicit StatefulStream(StatefulMap<INPUT_TYPE, OUTPUT_TYPE> *statefulMap) : statefulMap(statefulMap) {}

        virtual void receive(INPUT_TYPE value) {
            this->publish(statefulMap->apply_stateful_map(value));
        }
    };

}
#endif //GU_EDGENT_STATEFULSTREAM_HPP
