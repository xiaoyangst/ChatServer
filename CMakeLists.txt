cmake_minimum_required(VERSION 3.16)
project(chat CXX)

# C++17
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
include_directories(${PROJECT_SOURCE_DIR}/src/server/model)
include_directories(${PROJECT_SOURCE_DIR}/src/dbpool)
include_directories(${PROJECT_SOURCE_DIR}/src/redispool)
include_directories(${PROJECT_SOURCE_DIR}/src/utils)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(src)

add_executable(Test test.cpp)


