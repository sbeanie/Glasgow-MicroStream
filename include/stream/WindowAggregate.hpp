#ifndef _WINDOW_AGGREGATE_H_
#define _WINDOW_AGGREGATE_H_

#include <StreamTypes.hpp>
#include <list>

template <typename INPUT, typename OUTPUT>
class WindowAggregate: public MapStream<std::pair<int, std::list<INPUT> >, OUTPUT> {

public:
    WindowAggregate(OUTPUT (*func_vals_to_val) (std::pair<int, std::list<INPUT> >))
            : MapStream<std::pair<int, std::list<INPUT> >, OUTPUT>(func_vals_to_val) {};
};

#endif //_WINDOW_AGGREGATE_H_
