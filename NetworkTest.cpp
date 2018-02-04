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

    Topology* topology = new Topology();

    std::pair<unsigned int, void*> (*int_to_byte_array) (int) = [] (int val) {
        int *int_ptr = (int *) malloc(sizeof(int));
        *int_ptr = val;
        return std::pair<unsigned int, void*>(sizeof(val), int_ptr);
    };

    int (*byte_array_to_int) (unsigned int, void*) = [] (unsigned int length, void *byte_array) {
        return *((int*) byte_array);
    };

    std::list<int> values = {0,1,2,3,4};
    Source<int>* int_source = topology->addPolledSource(std::chrono::seconds(1), new NumberSource(values));
    Source<int>* int_source2 = topology->addFixedDataSource(values);

    auto *networkSink = new NetworkSink<int>("test_stream", "225.0.0.37", 12345, int_to_byte_array);
    auto *networkSource = new NetworkSource<int>("test_stream", "225.0.0.37", 12345, byte_array_to_int);

    int_source->subscribe(networkSink);
    int_source2->subscribe(networkSink);

    auto print_sink = [](int val) { std::cout << "Received val " << val << " over the network." << std::endl;};

    networkSource->sink(print_sink);

    std::cout << "Setup streams" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    topology->run();

    std::this_thread::sleep_for(std::chrono::seconds(60));

    return 0;
}
