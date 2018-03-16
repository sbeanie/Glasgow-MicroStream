#include "gtest/gtest.h"
#include "Stream.hpp"

using namespace glasgow_ustream;

class StreamPacketTest : public ::testing::Test {
protected:
    uint32_t delimiters_size = (uint32_t) (strlen(STREAM_PACKET_START_DELIMITER) + strlen(STREAM_PACKET_END_DELIMITER) + 2);

    StreamPacket *streamPacket = nullptr;
    void *data = nullptr;

    virtual void SetUp() {};
    virtual void TearDown() {
        if (streamPacket != nullptr) {
            delete(streamPacket);
        }
        if (data != nullptr) {
            free(data);
        }
    }
};

TEST_F(StreamPacketTest, ConstructableButBadDelimiter) {
    data = calloc(1, STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE);
    streamPacket = new StreamPacket({STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE, data}, true);
    EXPECT_TRUE(streamPacket->is_invalid());
    EXPECT_FALSE(streamPacket->is_complete());
}

TEST_F(StreamPacketTest, NotEnoughDataToConstruct) {
    data = calloc(1, STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE - 1);
    streamPacket = new StreamPacket({STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE - 1, data}, true);
    EXPECT_TRUE(streamPacket->is_invalid());
    EXPECT_FALSE(streamPacket->is_complete());
}

TEST_F(StreamPacketTest, ConstructableWithDelimiter) {
    data = calloc(1, STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE);
    memcpy(data, STREAM_PACKET_START_DELIMITER, strlen(STREAM_PACKET_START_DELIMITER) + 1);
    streamPacket = new StreamPacket({STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE, data}, true);
    EXPECT_FALSE(streamPacket->is_invalid());
    EXPECT_FALSE(streamPacket->is_complete());
}

TEST_F(StreamPacketTest, PartialAdd) {
    uint32_t total_size = this->delimiters_size + sizeof(uint32_t) + 1; // + 1 for data
    data = calloc(1, total_size);
    char *ptr = (char *) data;

    memcpy(data, STREAM_PACKET_START_DELIMITER, strlen(STREAM_PACKET_START_DELIMITER) + 1);
    ptr += strlen(STREAM_PACKET_START_DELIMITER) + 1;

    *((uint32_t *) ptr) = 1;
    ptr += sizeof(uint32_t);

    uint32_t current_size = ptr - (char *) data;

    streamPacket = new StreamPacket({current_size, data}, true);
    ASSERT_FALSE(streamPacket->is_invalid());
    ASSERT_FALSE(streamPacket->is_complete());

    *ptr = 'a';
    streamPacket->add_data({1, ptr});
    ptr += 1;
    ASSERT_FALSE(streamPacket->is_complete());
    ASSERT_FALSE(streamPacket->is_invalid());

    memcpy(ptr, STREAM_PACKET_END_DELIMITER, strlen(STREAM_PACKET_END_DELIMITER) + 1);
    char *end =  ptr + strlen(STREAM_PACKET_END_DELIMITER) + 1;
    uint32_t  remaining = end - (char *) ptr;

    streamPacket->add_data({remaining, ptr});
    ASSERT_TRUE(streamPacket->is_complete());
    ASSERT_FALSE(streamPacket->is_invalid());
}


TEST_F(StreamPacketTest, TooMuchDataFromStart) {
    uint32_t total_size = this->delimiters_size + sizeof(uint32_t) + 2; // + 1 for data
    data = calloc(1, total_size);
    char *ptr = (char *) data;

    memcpy(data, STREAM_PACKET_START_DELIMITER, strlen(STREAM_PACKET_START_DELIMITER) + 1);
    ptr += strlen(STREAM_PACKET_START_DELIMITER) + 1;

    *((uint32_t *) ptr) = 1;
    ptr += sizeof(uint32_t);

    *ptr = 'a';
    ptr += 1;

    memcpy(ptr, STREAM_PACKET_END_DELIMITER, strlen(STREAM_PACKET_END_DELIMITER) + 1);
    ptr += strlen(STREAM_PACKET_END_DELIMITER) + 1;

    void *remainder_ptr = ptr;

    streamPacket = new StreamPacket({total_size, data}, true);

    ASSERT_TRUE(streamPacket->is_complete());
    ASSERT_FALSE(streamPacket->is_invalid());
    ASSERT_EQ(remainder_ptr, streamPacket->get_remainder());
}

TEST_F(StreamPacketTest, TooMuchDataLater) {
    uint32_t total_size = this->delimiters_size + sizeof(uint32_t) + 2; // + 1 for data
    data = calloc(1, total_size);
    char *ptr = (char *) data;

    memcpy(data, STREAM_PACKET_START_DELIMITER, strlen(STREAM_PACKET_START_DELIMITER) + 1);
    ptr += strlen(STREAM_PACKET_START_DELIMITER) + 1;

    *((uint32_t *) ptr) = 1;
    ptr += sizeof(uint32_t);

    *ptr = 'a';
    ptr += 1;

    memcpy(ptr, STREAM_PACKET_END_DELIMITER, strlen(STREAM_PACKET_END_DELIMITER) + 1);
    ptr += strlen(STREAM_PACKET_END_DELIMITER) + 1;

    void *remainder_ptr = ptr;

    streamPacket = new StreamPacket({STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE, data}, true);

    ASSERT_FALSE(streamPacket->is_complete());
    ASSERT_FALSE(streamPacket->is_invalid());
    ASSERT_EQ(nullptr, streamPacket->get_remainder());

    size_t remaining_bytes = total_size - STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE;
    ptr = (char *) data + STREAM_PACKET_MINIMUM_CONSTRUCTION_SIZE;
    streamPacket->add_data({remaining_bytes, ptr});

    ASSERT_TRUE(streamPacket->is_complete());
    ASSERT_FALSE(streamPacket->is_invalid());
    ASSERT_EQ(remainder_ptr, streamPacket->get_remainder());
}


TEST_F(StreamPacketTest, FullPacketButBadEndDelimiter) {
    uint32_t total_size = this->delimiters_size + sizeof(uint32_t) + 1; // + 1 for data
    data = calloc(1, total_size);
    char *ptr = (char *) data;

    memcpy(data, STREAM_PACKET_START_DELIMITER, strlen(STREAM_PACKET_START_DELIMITER) + 1);
    ptr += strlen(STREAM_PACKET_START_DELIMITER) + 1;

    *((uint32_t *) ptr) = 1;
    ptr += sizeof(uint32_t);

    *ptr = 'a';
    ptr += 1;

    // Skip copying end delimiter
    // memcpy(ptr, STREAM_PACKET_END_DELIMITER, strlen(STREAM_PACKET_END_DELIMITER) + 1);
    // ptr += strlen(STREAM_PACKET_END_DELIMITER) + 1;

    streamPacket = new StreamPacket({total_size, data}, true);

    EXPECT_FALSE(streamPacket->is_complete());
    EXPECT_TRUE(streamPacket->is_invalid());
    EXPECT_EQ(nullptr, streamPacket->get_remainder());
}

TEST_F(StreamPacketTest, ValidPacket) {
    uint32_t total_size = this->delimiters_size + sizeof(uint32_t) + 1; // + 1 for data
    data = calloc(1, total_size);
    char *ptr = (char *) data;

    memcpy(data, STREAM_PACKET_START_DELIMITER, strlen(STREAM_PACKET_START_DELIMITER) + 1);
    ptr += strlen(STREAM_PACKET_START_DELIMITER) + 1;

    *((uint32_t *) ptr) = 1;
    ptr += sizeof(uint32_t);

    *ptr = 'a';
    ptr += 1;

    memcpy(ptr, STREAM_PACKET_END_DELIMITER, strlen(STREAM_PACKET_END_DELIMITER) + 1);
    ptr += strlen(STREAM_PACKET_END_DELIMITER) + 1;

    streamPacket = new StreamPacket({total_size, data}, true);

    ASSERT_TRUE(streamPacket->is_complete());
    ASSERT_FALSE(streamPacket->is_invalid());
    ASSERT_EQ(nullptr, streamPacket->get_remainder());
}