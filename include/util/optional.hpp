#ifndef GU_EDGENT_OPTIONAL_HPP
#define GU_EDGENT_OPTIONAL_HPP

namespace glasgow_ustream {

    template<typename T>
    class optional {

        bool initialized = false;
        T optional_value;

    public:
        optional() {
            initialized = false;
        }

        explicit optional(T val) {
            optional_value = val;
            initialized = true;
        }

        bool is_initialized() {
            return initialized;
        }

        T value() {
            return optional_value;
        }
    };
}

#endif //GU_EDGENT_OPTIONAL_HPP
