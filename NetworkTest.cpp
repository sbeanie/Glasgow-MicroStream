#include "Stream.hpp"


class NumberSource : public Pollable<int> {

    std::list<int> values;
    int pos = 0;

public:
    explicit NumberSource(std::list<int> values) {
        this->values = std::move(values);
    }

    int getData (PolledSource<int>* caller) {

        std::list<int>::iterator ptr;
        int i = 0;

        for (i = 0 , ptr = values.begin() ; i < pos && ptr != values.end(); i++ , ptr++ );

        if( ptr == values.end() ) {
            caller->stop();
            return 0;
        } else {
            pos++;
            return *ptr;
        }
    }
};


int main (int argc, char **argv) {


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //  Lambda Functions
    std::pair<size_t, void*> (*int_to_byte_array) (int) = [] (int val) {
        int *int_ptr = (int *) malloc(sizeof(int));
        *int_ptr = val;
        return std::pair<size_t, void*>(sizeof(val), int_ptr);
    };

    std::optional<int> (*byte_array_to_int) (std::pair<size_t, void*>) = [] (std::pair<size_t, void*> data) {
        return std::optional<int>(*((int*) data.second));
    };

    auto print_sink = [](int val) { std::cout << "Received val " << val << " over the network." << std::endl;};
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Topology* topology = new Topology();

    // Create two data sources that will feed the topology.
    std::list<int> values = {0,1,2,3,4};
    Source<int>* int_source = topology->addPolledSource(std::chrono::seconds(1), new NumberSource(values));
    Source<int>* int_source2 = topology->addFixedDataSource(values);

    // Union the two data sources and sink them into the network stream "numbers"
    auto union_stream = int_source->union_streams(1, &int_source2);
    union_stream->networkSink(topology, "numbers", int_to_byte_array);

    // Create a new topology source that will read data from the network (potentially from a different sensor)
    // This call fails if another source exists with the same stream_id.
    std::optional<NetworkSource<int>*> opt_network_int_source = topology->addNetworkSource("numbers", byte_array_to_int);
    if (! opt_network_int_source.has_value()) {
        std::cout << "Failed to create network source" << std::endl;
        exit(1);
    }

    // Sink the network stream into std.out.
    NetworkSource<int>* networkSource = opt_network_int_source.value();
    networkSource->sink(print_sink);

    std::cout << "Topology built.\nRunning..." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    topology->run();

    std::this_thread::sleep_for(std::chrono::seconds(60));

    return 0;
}
