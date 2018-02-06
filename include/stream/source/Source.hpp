#ifndef GU_EDGENT_SOURCE_H
#define GU_EDGENT_SOURCE_H

#include "Stream.hpp"


class Startable {

public:
    virtual void start() = 0;
};

template <typename T>
class Source : public Stream<T>, public Startable {

public:

    virtual void start() = 0;
};


#include "stream/source/FixedDataSource.hpp"
#include "stream/source/PolledSource.hpp"
#include "stream/source/NetworkSource.hpp"

#endif //GU_EDGENT_SOURCE_H
