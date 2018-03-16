#ifndef GU_EDGENT_BOOSTDESERIALIZEDNETWORKSOURCE_HPP
#define GU_EDGENT_BOOSTDESERIALIZEDNETWORKSOURCE_HPP

#include "../StreamTypes.hpp"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/stream.hpp>

namespace NAMESPACE_NAME {

    template<typename T>
    class BoostSerializedNetworkSource : public NetworkSource<T> {

    private:

    public:
        BoostSerializedNetworkSource() : NetworkSource<T>() {
            this->deserialize_func = [](std::pair<uint32_t, void *> data) {
                boost::iostreams::basic_array_source<char> source((char *) data.second, data.first);
                boost::iostreams::stream<boost::iostreams::basic_array_source<char> > stream(source);
                boost::archive::binary_iarchive iar(stream);

                T val;

                iar >> (val);

                iar.delete_created_pointers();
                return optional<T>(val);
            };
        }
    };
}


#endif //GU_EDGENT_BOOSTDESERIALIZEDNETWORKSOURCE_HPP
