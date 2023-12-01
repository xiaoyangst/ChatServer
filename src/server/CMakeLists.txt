cmake_minimum_required(VERSION 3.16)
project(SERVER C CXX)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(. SRC_LIST)

add_executable(ChatServer ${SRC_LIST})

target_link_libraries(ChatServer muduo_net muduo_base pthread)