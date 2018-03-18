#ifndef GU_EDGENT_SOURCE_H
#define GU_EDGENT_SOURCE_H

#include "../StreamTypes.hpp"

namespace NAMESPACE_NAME {

    class Startable {

    public:
        virtual void start() = 0;

        virtual void stop() = 0;

        virtual void join() = 0;
    };

    template<typename T>
    class Source : public Stream<T>, public Startable {

    public:

        virtual void start() = 0;

        virtual void stop() = 0;

        virtual void join() = 0;
    };

}

#include "FixedDataSource.hpp"
#include "PolledSource.hpp"
#include "NetworkSource.hpp"

#ifdef COMPILE_WITH_BOOST_SERIALIZATION
#include "BoostSerializedNetworkSource.hpp"
#endif

#endif //GU_EDGENT_SOURCE_H
