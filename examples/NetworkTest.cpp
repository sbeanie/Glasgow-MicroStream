#include "Stream.hpp"

using namespace glasgow_ustream;

class NumberSource : public Pollable<uint32_t> {

    std::list<uint32_t> *values;
    int pos = 0;

public:
    explicit NumberSource(std::list<uint32_t> *values) {
        this->values = values;
    }

    virtual ~NumberSource() {}

    uint32_t getData(PolledSource<uint32_t> *caller) {

        std::list<uint32_t>::iterator ptr;
        int i = 0;

        for (i = 0, ptr = values->begin(); i < pos && ptr != values->end(); i++, ptr++);

        if (ptr == values->end()) {
            caller->stop();
            return 0;
        } else {
            pos++;
            return *ptr;
        }
    }
};


int main(int, char **) {

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //  Lambda Functions
    auto print_temperature_sink = [](double val) {
        std::cout << "Received temperature " << val << "°C" << std::endl;
    };
    auto print_humidity_sink = [](double val) {
        std::cout << "Received humidity " << val << "%" << std::endl;
    };
    auto print_pressure_sink = [](double val) {
        std::cout << "Received pressure " << val << " hPa" << std::endl;
    };

    std::pair<uint32_t, void*> (*double_to_byte_array) (double) = [] (double val) {
        double *double_ptr = (double *) malloc(sizeof(double));
        *double_ptr = val;
        return std::pair<uint32_t, void*>(sizeof(val), double_ptr);
    };

    optional<double> (*byte_array_to_double) (std::pair<uint32_t, void*>) = [] (std::pair<uint32_t, void*> data) {
        return optional<double>(*((double*) data.second));
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Topology *topology = new Topology(std::chrono::seconds(1));

    // Create two data sources that will feed the topology.
    std::list<uint32_t> values = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    NumberSource *numberSource = new NumberSource(&values);
    Source<uint32_t> *int_source = topology->addFixedDataSource(values);
    Source<uint32_t> *int_source2 = topology->addPolledSource(std::chrono::seconds(1), numberSource);

    // Union the two data sources and sink them into the network stream "numbers"
    std::list<Subscribeable<uint32_t> *> subscribers = {(Subscribeable<uint32_t> *) int_source2};
    auto *union_stream = int_source->union_streams(subscribers);
//    union_stream->networkSink(topology, "numbers", int_to_byte_array);

    // Create a new topology source that will read data from the network (potentially from a different sensor)
    // This call fails if another source exists with the same stream_id.
    optional<NetworkSource<double> *> opt_network_temperature_source = topology->addNetworkSource("temperature_a", byte_array_to_double);
    optional<NetworkSource<double> *> opt_network_pressure_source = topology->addNetworkSource("pressure_a", byte_array_to_double);
    optional<NetworkSource<double> *> opt_network_humidity_source = topology->addNetworkSource("humidity_a", byte_array_to_double);

    // Sink the network stream into std.out.
    NetworkSource<double> *networkTemperatureSource = opt_network_temperature_source.value();
    NetworkSource<double> *networkPressureSource = opt_network_pressure_source.value();
    NetworkSource<double> *networkHumiditySource = opt_network_humidity_source.value();

    networkTemperatureSource->sink(print_temperature_sink);
    networkPressureSource->sink(print_pressure_sink);
    networkHumiditySource->sink(print_humidity_sink);

    std::cout << "Topology built." << std::endl;

    while (!topology->peers_connected()) {
        std::cout << "Waiting for connections..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "Running..." << std::endl;
    topology->run();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    topology->shutdown();
    delete (topology);
    delete (numberSource);

    return 0;
}
