#ifndef GU_EDGENT_OPTIONAL_HPP
#define GU_EDGENT_OPTIONAL_HPP

template <typename T>
class Optional {

    bool initialized = false;
    T optional_value;

public:
    Optional() {
        initialized = false;
    }

    Optional(T val) {
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

#endif //GU_EDGENT_OPTIONAL_HPP
