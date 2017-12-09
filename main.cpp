#include <iostream>
#include "Stream.h"

int main(int, char**) {
    
    Stream<int>* stream_1 = new Stream<int>();
    Stream<int>* stream_2 = new Stream<int>();

    Stream<int>* union_stream = stream_1->union_streams(1, &stream_2);

    auto print_sink_0 = [](char chars) { std::cout << "0: " << chars << std::endl;};
    auto print_sink_1 = [](char chars) { std::cout << "1: " << chars << std::endl;};

    char (*map_func) (int) = [](int a) { return a == 5 ? 'a' : 'b';};

    MapStream<int, char>* map_stream =  union_stream->map(map_func);

    Stream<char>** split_streams = map_stream->split(2, [](char c) {return c == 'a' ? 0 : 1;});

    split_streams[0]->sink(print_sink_0);
    split_streams[1]->sink(print_sink_1);

    auto window = union_stream->last(boost::chrono::seconds(5), 1, [](int a) {return 0;});

    std::pair<int, std::list<int>*> (*int_values_printer) (std::pair<int, std::list<int>*>) = [] (std::pair<int, std::list<int>* > nums) {
        std::cout << "Key " << nums.first << ": Received " << nums.second->size() << " value(s)." << std::endl;
        for (auto &i : *(nums.second)) {
            std::cout << "Received number: " << i << std::endl;
        }
        return nums;
    };

    int (*aggr_func) (std::pair<int, std::list<int>*>) = [] (std::pair<int, std::list<int>*> keyValuePair) {
        int sum = 0;
        for (int &i : *keyValuePair.second) {
            sum += i;
        }
        return sum;
    };
    auto window_aggregate = window->aggregate(aggr_func);

    window_aggregate->sink([] (int num) {
        std::cout << "Aggregate value: " << num << std::endl;
    });

    window->batch(boost::chrono::seconds(1), int_values_printer);

    stream_1->receive(5);
    stream_2->receive(10);

    // If window->stop() is called before the window thread is done processing, the thread may die before processing data.
    boost::this_thread::sleep_for(boost::chrono::seconds(10));
    window->stop();

    return 0;
}
