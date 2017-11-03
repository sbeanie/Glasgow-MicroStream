#include "Stream.h"

int main(int, char**) {
    
    Stream<int>* stream = new Stream<int>();

    Stream<int>* sub_stream = new Stream<int>();

    stream->subscribe(sub_stream);

    auto f = [](int a) { std::cout << a << std::endl;};

    auto filter_func = [](int a) { return a % 2 == 0;};

    Stream<int>* filtered_stream = stream->filter(filter_func);

    filtered_stream->sink(f);

    stream->receive(5);
    stream->receive(10);

    delete(stream);
    return 0;
}