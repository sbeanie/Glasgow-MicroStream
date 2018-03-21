#include <iostream>
#include "Stream.hpp"

using namespace glasgow_ustream;

int main(int, char**) {

    Topology *topology = new Topology();

    Stream<int>* odd_nums = topology->addFixedDataSource(std::list<int>{1,3,5,7,9,11,13,15});
    Stream<int>* even_nums = topology->addFixedDataSource(std::list<int>{0,2,4,6,8,10,12,14});

    Stream<int>* number_stream = odd_nums->union_streams(std::list<Subscribeable<int>*>{even_nums});

    auto even_print_sink = [](char chars) { std::cout << "Even: " << chars << std::endl;};
    auto odd_print_sink = [](char chars) { std::cout << "Odd: " << chars << std::endl;};

    char (*map_func) (int) = [](int a) { return a % 2 == 0 ? 'a' : 'b';};

    MapStream<int, char>* map_stream =  number_stream->map(map_func);

    std::vector<Stream<char>*> split_streams = map_stream->split(2, [](char c) {return c == 'a' ? 0 : 1;});

    split_streams[0]->sink(even_print_sink);
    split_streams[1]->sink(odd_print_sink);

    auto window = number_stream->last(std::chrono::seconds(5), 2, [](int v) {return (v % 2 == 0 ? 0 : 1);});

    std::pair<int, std::list<int> > (*int_values_printer) (std::pair<int, std::list<int> >) = [] (std::pair<int, std::list<int> > nums) {
        std::cout << "Key " << nums.first << ": Received " << nums.second.size() << " value(s)." << std::endl;
        std::cout << "Contents: [";

        unsigned long n = 1L;
        size_t max_n = nums.second.size();
        for (auto &i : nums.second) {
            if (n == max_n) {
                std::cout << i;
            } else {
                std::cout << i << ", ";
            }
            n++;
        }
        std::cout << "]" << std::endl;
        return nums;
    };

    int (*aggr_func) (std::pair<int, std::list<int> >) = [] (std::pair<int, std::list<int> > keyValuePair) {
        int sum = 0;
        for (int &i : keyValuePair.second) {
            sum += i;
        }
        return sum;
    };
    auto window_aggregate = window->aggregate(aggr_func);

    window_aggregate->sink([] (int num) {
        std::cout << "Aggregate value: " << num << std::endl;
    });

    auto window_batch = window->batch(std::chrono::seconds(1), int_values_printer);

    topology->run();

    // If window->stop() is called before the window thread is done processing, the thread may die before processing data.
    std::this_thread::sleep_for(std::chrono::seconds(10));
    topology->shutdown();
    delete(topology);

    return 0;
}
