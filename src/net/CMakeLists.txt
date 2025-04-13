cmake_minimum_required(VERSION 3.16)
project(net)


set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")

aux_source_directory(. SRC_LIST)

add_library(net SHARED ${SRC_LIST})