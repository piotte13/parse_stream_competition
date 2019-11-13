cmake_minimum_required(VERSION 3.15)
project(parse_stream_competition)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")


add_executable(parse_stream_competition main.cpp)