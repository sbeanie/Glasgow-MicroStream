#ifndef _SPLIT_STREAM_H_
#define _SPLIT_STREAM_H_

#include "StreamTypes.hpp"
#include <vector>

namespace NAMESPACE_NAME {

    template<typename T>
    class SplitStream : public Stream<T> {
        int (*split_function)(T);

        int num_streams;
        std::vector<Stream<T> *> streams;
    public:
        SplitStream(int (*split_function)(T), int num_streams, std::vector<Stream<T> *> streams)
                : split_function(split_function), num_streams(num_streams), streams(streams) {

            for (auto it = streams.begin(); it != streams.end(); it++) {
                // Subscribing won't cause duplicates as the streams receive values directly via receive() instead of publish()
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