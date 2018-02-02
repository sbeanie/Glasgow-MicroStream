#include <network/NetworkSource.h>
#include "network/NetworkSink.h"


int main (int argc, char **argv) {

    std::pair<unsigned int, void*> (*int_to_byte_array) (int) = [] (int val) {
        int *int_ptr = (int *) malloc(sizeof(int));
        *int_ptr = val;
        return std::pair<unsigned int, void*>(sizeof(val), int_ptr);
    };

    int (*byte_array_to_int) (unsigned int, void*) = [] (unsigned int length, void *byte_array) {
        return *((int*) byte_array);
    };

    auto* int_stream = new Stream<int>();

    auto *networkSink = new NetworkSink<int>("test_stream", "225.0.0.37", 12345, int_to_byte_array);
    auto *networkSource = new NetworkSource<int>("test_stream", "225.0.0.37", 12345, byte_array_to_int);

    int_stream->subscribe(networkSink);


    auto print_sink = [](int val) { std::cout << "Received val " << val << " over the network." << std::endl;};

    networkSource->sink(print_sink);

    std::cout << "Setup streams" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    int_stream->receive(5);
    int_stream->receive(10);
    int_stream->receive(15);
    int_stream->receive(20);
    int_stream->receive(25);

    std::this_thread::sleep_for(std::chrono::seconds(60));

    return 0;
}
