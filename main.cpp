#include "Stream.h"

int main(int, char**) {
    
    Stream<int>* stream_1 = new Stream<int>();
    Stream<int>* stream_2 = new Stream<int>();

    Stream<int>* union_stream = stream_1->union_streams(1, &stream_2);

    auto filter_func = [](int a) { return a % 2 == 0;};

    Stream<int>* filter_stream = union_stream->filter(filter_func);
    
    auto f = [](int a) { std::cout << a << std::endl;};
    filter_stream->sink(f);


    stream_1->receive(5);
    stream_2->receive(10);

    return 0;
}
