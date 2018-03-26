#ifndef GU_EDGENT_BOOSTSERIALIZEDNETWORKSINK_HPP
#define GU_EDGENT_BOOSTSERIALIZEDNETWORKSINK_HPP

#include "../StreamTypes.hpp"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

namespace glasgow_ustream {

    template<typename T>
    class BoostSerializedNetworkSink : public NetworkSink<T> {

    public:

        BoostSerializedNetworkSink(Topology *topology, const char *stream_id) : NetworkSink<T>(topology, stream_id) {
            this->val_to_bytes = [](T val) {
                std::string serial_str;
                boost::iostreams::back_insert_device<std::string> inserter(serial_str);
                boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s(inserter);

                boost::archive::binary_oarchive oa(s);
                oa << val;

                s.flush();

                const char *str = serial_str.c_str();
                size_t bytes_written = serial_str.size();

                void *data = malloc(bytes_written);
                memcpy(data, str, bytes_written);

                return std::pair<uint32_t, void *>(bytes_written, data);
            };
        }

        virtual ~BoostSerializedNetworkSink() = default;
    };

}

#endif //GU_EDGENT_BOOSTSERIALIZEDNETWORKSINK_HPP
