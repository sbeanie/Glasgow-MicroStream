#ifndef GU_EDGENT_FIXEDDATASOURCE_H
#define GU_EDGENT_FIXEDDATASOURCE_H

#include <iostream>
#include "../StreamTypes.hpp"

namespace NAMESPACE_NAME {

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
