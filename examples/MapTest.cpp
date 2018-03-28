#include "Stream.hpp"

using namespace glasgow_ustream;

class IntMapStateful : public StatefulMap<int, int> {

private:
    int count = 0;
public:
    int apply_stateful_map(int val) override {
        count += 1;
        std::cout << "Received " << count << " values." << std::endl;
        return val;
    }
};

int main(int, char **) {
    auto *topology = new Topology(true);

    std::list<int> values = {0,1,2,3,4,5,6,7,8,9,10};
    Stream<int> *int_source = topology->addFixedDataSource(values);

    auto *intMapStateful = new IntMapStateful();
    int_source->map_stateful(intMapStateful);

    topology->run();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    topology->shutdown();
    delete(topology);
    delete(intMapStateful);
}