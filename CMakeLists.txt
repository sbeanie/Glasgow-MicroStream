cmake_minimum_required (VERSION 2.6)
project (GU-Edgent)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_CXX_FLAGS  "-g --std=c++11 -W -Wall -Wpedantic" )

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/include")

#add_executable(GU-Edgent main.cpp)
add_executable(GU-Edgent NetworkTest.cpp)

find_package( Boost 1.66 REQUIRED COMPONENTS serialization )
#TARGET_LINK_LIBRARIES( GU-Edgent LINK_PUBLIC ${Boost_LIBRARIES} Threads::Threads )
TARGET_LINK_LIBRARIES(GU-Edgent LINK_PUBLIC Threads::Threads)

add_executable(runTests tests/main.cpp)
TARGET_LINK_LIBRARIES(runTests LINK_PUBLIC Threads::Threads gtest)

add_executable(boostserialize boostserialize.cpp)
TARGET_LINK_LIBRARIES(boostserialize LINK_PUBLIC Threads::Threads ${Boost_LIBRARIES})