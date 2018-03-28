#ifndef GU_EDGENT_FIXEDDATASOURCE_H
#define GU_EDGENT_FIXEDDATASOURCE_H

#include <iostream>
#include "../StreamTypes.hpp"

namespace glasgow_ustream {

    /**
     * This class provides a means of pushing a fixed set of values through a topology.
     * @tparam T
     */
    template<typename T>
    class FixedDataSource : public Source<T> {

        std::list<T> values;

    public:

        explicit FixedDataSource(std::list<T> values) {
            this->values = std::move(values);
        };

        void start() override {
            for (auto i = values.begin(); i != values.end(); i++) {
                this->publish(*i);
            }
        }

        void stop() override {}

        void join() override {}
    };

}

#endif //GU_EDGENT_FIXEDDATASOURCE_H
