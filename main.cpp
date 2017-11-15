#include "Stream.h"

int main(int, char**) {
    
    Stream<int, int>* stream_1 = new Stream<int, int>();
    Stream<int, int>* stream_2 = new Stream<int, int>();

    Stream<int, int>* union_stream = stream_1->union_streams(1, &stream_2);

    auto print_sink = [](char chars) { std::cout << chars << std::endl;};
    char (*map_func) (int) = [](int a) { return a == 5 ? 'a' : 'b';};

    Stream<int, char>* map_stream =  union_stream->map(map_func);

    map_stream->sink(print_sink);

    stream_1->receive(5);
    stream_2->receive(10);

    return 0;
}
