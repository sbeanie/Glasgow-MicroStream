#include "Stream.hpp"

using namespace glasgow_ustream;

int main (int, char**) {

    void (*print_int_list) (std::list<int>) = [] (std::list<int> values) {
        std::cout << "Tail stream outputted: [";
        for (auto &val : values) {
            std::cout << val << ",";
        }
        std::cout << "]" << std::endl;
    };

    // Create a topology with networking disabled.
    Topology topology = Topology(true);

    FixedDataSource<int>* source = topology.addFixedDataSource(std::list<int>{1,2,3,4,5,6,7});
    TailStream<int>* tail_stream = source->tail(3);
    tail_stream->sink(print_int_list);

    topology.run_with_threads();
    topology.shutdown();
}