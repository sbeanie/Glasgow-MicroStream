#ifndef GU_EDGENT_ITERABLESOURCE_HPP
#define GU_EDGENT_ITERABLESOURCE_HPP

#include "../StreamTypes.hpp"

namespace glasgow_ustream {

    /**
     * Extend this class and implement its methods in order to create an IterableSource node.
     * @tparam T The type of the class.
     */
    template <typename T>
    class Iterable {

    public:
        /**
         * This should return true if next() will return data.
         * @return
         */
        virtual bool has_next() = 0;

        /**
         * This should return the next value in the iterable.
         * @return the next value of type T.
         */
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
