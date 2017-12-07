cmake_minimum_required (VERSION 2.6)
project (GU-Edgent)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories ("${PROJECT_SOURCE_DIR}/include")

find_package( Boost REQUIRED COMPONENTS system chrono thread )
include_directories( ${Boost_INCLUDE_DIRS} )

add_executable(GU-Edgent main.cpp)

TARGET_LINK_LIBRARIES( GU-Edgent LINK_PUBLIC ${Boost_LIBRARIES} )


