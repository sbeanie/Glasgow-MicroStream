#ifndef GU_EDGENT_SOURCE_H
#define GU_EDGENT_SOURCE_H

#include "../StreamTypes.hpp"

namespace glasgow_ustream {

    /**
     * This class is used by a topology in its list of things to start when it begins execution.
     */
    class Startable {

    public:
        /**
         * This method is called when the topology starts
         */
        virtual void start() = 0;

        /**
         * This is called when the topology is requested to stop execution.
         */
        virtual void stop() = 0;

        /**
         * This method is called to join any threads if any when a Topology is shutdown/deleted.
         */
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
#include "IterableSource.hpp"

#ifdef COMPILE_WITH_BOOST_SERIALIZATION
#include "BoostSerializedNetworkSource.hpp"
#endif

#endif //GU_EDGENT_SOURCE_H
