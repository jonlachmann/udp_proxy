cmake_minimum_required(VERSION 3.26)
project(udp_proxy)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DBOOST_ASIO_STANDALONE)

# Boost is required for UDP networking for both server and client
include_directories(${CMAKE_SOURCE_DIR}/external/boost/libs/system/include)
include_directories(${CMAKE_SOURCE_DIR}/external/boost/libs/asio/include)
include_directories(${CMAKE_SOURCE_DIR}/external/boost/libs/config/include)
include_directories(${CMAKE_SOURCE_DIR}/external/boost/libs/assert/include)
include_directories(${CMAKE_SOURCE_DIR}/external/boost/libs/container_hash/include)

add_executable(udp_proxy main.cpp UdpSimulator.cpp UdpSimulator.h
)
