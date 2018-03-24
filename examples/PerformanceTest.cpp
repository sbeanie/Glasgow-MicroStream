#include "Stream.hpp"

#include <chrono>

using namespace glasgow_ustream;

class NumberGenerator : public Iterable<int> {

    int start, end;
    int current;

public:

    NumberGenerator(int start, int end) : start(start), end(end), current(start) {}

    bool has_next() override {
        return current <= end;
    }

    int next() override {
        return current++;
    }
};

class CountMap : public StatefulMap<int, int> {

    int count = 0;
public:

    explicit CountMap(){};

    int get_count() {
        return count;
    }

    int apply_stateful_map(int val) {
        count++;
        return val;
    }
};

int main(int, char**) {

    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    auto filter_op  = [] (int val) {
        return val % 2 == 0;
    };

    auto print_sink  = [] (int val) {
        std::cout << val << std::endl;
    };

    Topology topology(true);

    NumberGenerator numberGenerator(0, 100000000);
    CountMap countMap = CountMap();

    IterableSource<int> *iterableSource = topology.addIterableSource((Iterable<int> *) &numberGenerator);
    FilterStream<int> *filterStream = iterableSource->filter(filter_op);
    StatefulStream<int,int> *statefulStream = filterStream->map_stateful((StatefulMap<int, int> *) &countMap);

    topology.run_with_threads();
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::cout << countMap.get_count() << std::endl;
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    std::cout << "Elapsed time (ms): " << duration << std::endl;

    topology.shutdown();
}