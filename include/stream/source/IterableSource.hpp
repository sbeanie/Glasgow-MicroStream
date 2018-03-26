#ifndef GU_EDGENT_ITERABLESOURCE_HPP
#define GU_EDGENT_ITERABLESOURCE_HPP

#include "../StreamTypes.hpp"

namespace glasgow_ustream {

    template <typename T>
    class Iterable {

    public:
        virtual bool has_next() = 0;
        virtual T next() = 0;
    };

    template<typename T>
    class IterableSource : public Source<T> {

        bool should_stop = false;
        Iterable<T> *iterable;

    public:

        explicit IterableSource(Iterable<T> *iterable) : iterable(iterable) {};

        void start() override {
            while (iterable->has_next() &! should_stop) {
                this->publish(iterable->next());
            }
        }

        void stop() override {
            this->should_stop = true;
        }

        void join() override {}
    };
}

#endif //GU_EDGENT_ITERABLESOURCE_HPP
