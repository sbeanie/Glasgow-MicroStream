#ifndef GU_EDGENT_OPTIONAL_HPP
#define GU_EDGENT_OPTIONAL_HPP

namespace glasgow_ustream {

    /**
     * This class is used as an alternative to boost::optional.
     * @tparam T the type of the value this optional might contain.
     */
    template<typename T>
    class optional {

        bool initialized = false;
        T optional_value;

    public:

        /**
         * This constructor should be used when a value is not returned.
         */
        optional() {
            initialized = false;
        }

        /**
         * This constructor should be used when a value is returned.
         * @param val the value this optional should contain.
         */
        explicit optional(T val) {
            optional_value = val;
            initialized = true;
        }

        /**
         * This should be checked before calling value()
         * @return true if this optional contains a value.
         */
        bool is_initialized() {
            return initialized;
        }

        T value() {
            return optional_value;
        }
    };
}

#endif //GU_EDGENT_OPTIONAL_HPP
