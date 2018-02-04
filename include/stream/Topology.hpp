#ifndef GU_EDGENT_TOPOLOGY_H
#define GU_EDGENT_TOPOLOGY_H

#include "Stream.hpp"

class Topology {

private:
    std::list<Startable*> sources;

public:


    template <typename T>
    FixedDataSource<T>* addFixedDataSource(std::list<T> values) {
        auto *fixedDataSource = new FixedDataSource<T>(values);
        sources.push_back((Startable*) fixedDataSource);
        return fixedDataSource;
    }

    template <typename T>
    PolledSource<T>* addPolledSource(std::chrono::duration<double> interval, Pollable<T>* pollable) {
        auto *polledSource = new PolledSource<T>(interval, pollable);
        sources.push_back((Startable*) polledSource);
        return polledSource;
    }

    void run() {
        for (auto &source : sources) {
            (*source).start();
        }
    }
};


#endif //GU_EDGENT_TOPOLOGY_H
