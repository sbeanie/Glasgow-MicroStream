#include "Stream.hpp"
#include <chrono>

using namespace glasgow_ustream;

class TimedNumberGenerator : public Iterable<std::pair<std::chrono::high_resolution_clock::time_point, int>> {

    int start, end;
    int current;

public:

    TimedNumberGenerator(int start, int end) : start(start), end(end), current(start) {}

    bool has_next() override {
        return current <= end;
    }

    std::pair<std::chrono::high_resolution_clock::time_point, int> next() override {
        return std::pair<std::chrono::high_resolution_clock::time_point, int>
                (std::chrono::high_resolution_clock::now(), current++);
    }
};


int main(int, char**) {

    Topology *topology = new Topology();

    TimedNumberGenerator timedNumberGenerator = TimedNumberGenerator(0, 1000);

    IterableSource<std::pair<std::chrono::high_resolution_clock::time_point, int>> *iterableSource =
            topology->addIterableSource(&timedNumberGenerator);

    FilterStream<std::pair<std::chrono::high_resolution_clock::time_point, int>> *filterStream =
            iterableSource->filter([] (std::pair<std::chrono::high_resolution_clock::time_point, int> pair) {
       return pair.second % 2 == 0;
    });

    long (*difference_map) (std::pair<std::chrono::high_resolution_clock::time_point, int>) = [] (std::pair<std::chrono::high_resolution_clock::time_point, int> pair) {
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now() - pair.first).count();
        return duration;
    };

    MapStream<std::pair<std::chrono::high_resolution_clock::time_point, int>, long> *mapStream =
    filterStream->map(difference_map);

    unsigned long (*average_map) (std::pair<int, std::list<long>>) = [] (std::pair<int, std::list<long>> key_differences) {
        long sum = 0L;
        unsigned long count = key_differences.second.size();
        std::cout << "Size: " << count << std::endl;
        for (auto val : key_differences.second) {
            sum += val;
        }
        return sum / count;
    };

    WindowBatch<long, unsigned long> *average = mapStream->last(std::chrono::seconds(5))->batch(std::chrono::seconds(5), average_map);

    average->boostSerializedNetworkSink(topology, "average");
    NetworkSource<unsigned long> *averages = topology->addBoostSerializedNetworkSource<unsigned long>("average").value();


    void (*print_sink) (unsigned long) = [] (unsigned long val) {
        std::cout << "Average time: " << val << std::endl;
    };

    average->sink(print_sink);

    std::cout << "Starting" << std::endl;
    topology->run_with_threads();
    std::cout << "Finished" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    topology->shutdown();
    delete(topology);
}