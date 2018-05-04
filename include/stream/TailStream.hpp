#ifndef _TAIL_STREAM_HPP_
#define _TAIL_STREAM_HPP_

#include "StreamTypes.hpp"
#include <list>
#include <vector>
#include <cstdlib>

namespace glasgow_ustream {

    template<typename INPUT_TYPE>
    class TailStream : public TwoTypeStream<INPUT_TYPE, std::list<INPUT_TYPE>> {

        std::vector<INPUT_TYPE> values;
        int current_start = 0;
        int current_end = 0;
        unsigned int number_of_elements = 0;
        unsigned int tail_size = 0;

        void build_and_push() {
            // Build the ordered list to publish.
            int index = current_start;
            std::list<INPUT_TYPE> publish_list;
            unsigned int number_added = 0;
            while (number_added++ != number_of_elements) {
                publish_list.push_back(values.at(index));
                index = (index + 1) % tail_size;
            }
            this->publish(publish_list);
        }

    public:

        /**
         * A TailStream is a stream operation that always maintains the last "tail_size" values that pass through it.
         * When the tail becomes full (that is it has received "tail_size" values), it pushes the last "tail_size" values
         * as an ordered list to any subscribers in the order they passed through the stream.
         * When a new value arrives, the oldest value is removed and the new one is placed at the end of the tail.
         * @param tail_size The number of elements to include in the tail list for each published list.
         */
        explicit TailStream(unsigned int tail_size) : tail_size(tail_size){
            values = std::vector<INPUT_TYPE>(tail_size);
        }

        virtual void receive(INPUT_TYPE value) {
            if (number_of_elements == tail_size) {
                // Tail is ready to push

                // Put new value into current start and make it the new end.
                values[current_start] = value;
                current_start = (current_start + 1) % tail_size;
                current_end = (current_end + 1) % tail_size;

                this->build_and_push();
            } else {
                // Tail is not ready to push
                values[current_end] = value;
                number_of_elements++;
                current_end = (current_end + 1) % tail_size;
                if (number_of_elements == tail_size) this->build_and_push();
            }
        }


    };
}

#endif