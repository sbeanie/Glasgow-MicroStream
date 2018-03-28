#ifndef _WINDOW_AGGREGATE_H_
#define _WINDOW_AGGREGATE_H_

#include "StreamTypes.hpp"
#include <list>

namespace glasgow_ustream {

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class WindowAggregate : public MapStream<std::pair<int, std::list<INPUT_TYPE> >, OUTPUT_TYPE> {

    public:
        /**
         * Constructs a window aggregate stream node.  Underneath it is just a MapStream.
         * @param func_vals_to_val a function mapping a list of values of one type to an output type.
         */
        WindowAggregate(OUTPUT_TYPE (*func_vals_to_val)(std::pair<int, std::list<INPUT_TYPE> >))
                : MapStream<std::pair<int, std::list<INPUT_TYPE> >, OUTPUT_TYPE>(func_vals_to_val) {};
    };

}

#endif //_WINDOW_AGGREGATE_H_
