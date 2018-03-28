#ifndef _SPLIT_STREAM_H_
#define _SPLIT_STREAM_H_

#include "StreamTypes.hpp"
#include <vector>

namespace glasgow_ustream {

    template<typename T>
    class SplitStream : public Stream<T> {
        int (*split_function)(T);

        int num_streams;
        std::vector<Stream<T> *> streams;
    public:

        /**
         * A SplitStream is a node in a topology that maintains a list of output streams, to which it forwards values
         * to specific streams based on the returned value by the split_function.
         * @param split_function A function that given a value, should return the stream number to forward it to.  Note
         * the return value is modulo'd the number of streams.
         * @param num_streams The number of streams involved in processing.
         */
        SplitStream(int (*split_function)(T), int num_streams, std::vector<Stream<T> *> streams)
                : split_function(split_function), num_streams(num_streams), streams(streams) {

            for (auto it = streams.begin(); it != streams.end(); it++) {
                // Subscribing won't cause duplicates as the streams receive values directly via receive() instead of publish().
                this->subscribe(*it);
            }
        };

        void receive(T value) {
            int split_stream_num = split_function(value);
            streams[split_stream_num % num_streams]->receive(value);
        }
    };
}
#endif