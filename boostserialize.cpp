#include "Stream.hpp"

class gps_location {

private:

    int degrees;
    int minutes;
    float seconds;

    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive & archive, const unsigned int)
    {
        archive & degrees;
        archive & minutes;
        archive & seconds;
    }

public:
    gps_location() = default;
    gps_location(int d, int m, float s) :
            degrees(d), minutes(m), seconds(s)
    {}

    int get_degrees() {
        return degrees;
    }

    int get_minutes() {
        return minutes;
    }

    float get_seconds() {
        return seconds;
    }
};


int main(int, char **) {
    Topology *topology = new Topology();

    const gps_location a(35, 59, 24.567f);
    const gps_location b(36, 60, 24.567f);
    const gps_location c(37, 61, 24.567f);

    std::list<gps_location> vals = {a,b,c};
    Stream<gps_location> *vals_stream = topology->addFixedDataSource(vals);

    auto print_sink = [](gps_location val) {
        std::cout << "Received gps_location: " << val.get_degrees() << ", " << val.get_minutes() << ", " << val.get_seconds()
                  << " over the network." << std::endl;
    };

    vals_stream->boostSerializedNetworkSink(topology, "test");

    auto optNetworkSource = topology->addBoostNetworkSource<gps_location>("test");
    if (optNetworkSource.is_initialized()) {
        NetworkSource<gps_location> *gps_stream = optNetworkSource.get();
        gps_stream->sink(print_sink);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    topology->run();
    topology->shutdown();
    delete(topology);
}