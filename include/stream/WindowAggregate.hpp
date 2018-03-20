#ifndef _WINDOW_AGGREGATE_H_
#define _WINDOW_AGGREGATE_H_

#include "StreamTypes.hpp"
#include <list>

namespace NAMESPACE_NAME {

    template<typename INPUT_TYPE, typename OUTPUT_TYPE>
    class WindowAggregate : public MapStream<std::pair<int, std::list<INPUT_TYPE> >, OUTPUT_TYPE> {

    public:
        WindowAggregate(OUTPUT_TYPE (*func_vals_to_val)(std::pair<int, std::list<INPUT_TYPE> >))
                : MapStream<std::pair<int, std::list<INPUT_TYPE> >, OUTPUT_TYPE>(func_vals_to_val) {};
    };

}

#endif //_WINDOW_AGGREGATE_H_
