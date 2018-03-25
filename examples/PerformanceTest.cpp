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

int main(int argc, char** argv) {

    auto filter_op  = [] (int val) {
        return val % 2 == 0;
    };

    auto print_sink  = [] (int val) {
        std::cout << val << std::endl;
    };

    if (argc < 2) {
        exit(1);
    }
    int num_cores = atoi(argv[1]);

    Topology topology(true);

    std::vector<NumberGenerator *> numGens;
    std::vector<CountMap *> countMaps;
    for (int i = 0; i < num_cores; i++) {
        auto *numberGenerator = new NumberGenerator(0,1000000000);
        auto *countMap = new CountMap();
        numGens.push_back(numberGenerator);
        countMaps.push_back(countMap);
        IterableSource<int> *iterableSource = topology.addIterableSource((Iterable<int> *) numGens[i]);
        FilterStream<int> *filterStream = iterableSource->filter(filter_op);
        StatefulStream<int,int> *statefulStream = filterStream->map_stateful((StatefulMap<int, int> *) countMaps[i]);
    }

//    IterableSource<int> *iterableSource = topology.addIterableSource((Iterable<int> *) &numberGenerator);
//    FilterStream<int> *filterStream = iterableSource->filter(filter_op);
//    StatefulStream<int,int> *statefulStream = filterStream->map_stateful((StatefulMap<int, int> *) &countMap);
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    topology.run_with_threads();
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    std::cout << countMaps[0]->get_count() << std::endl;
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    std::cout << "Elapsed time (ms): " << duration << std::endl;
    topology.shutdown();
    for (int i = 0; i < num_cores; i++) {
        delete(numGens[i]);
        delete(countMaps[i]);
    }
}