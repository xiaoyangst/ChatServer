cmake_minimum_required(VERSION 3.16)
project(TEST C CXX)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(. SRC_LIST)
aux_source_directory(../src/client CLIENT_LIST)
aux_source_directory(../src/server SERVER_LIST)
aux_source_directory(../src/server/db DB_LIST)
aux_source_directory(../src/server/model MODEL_LIST)
aux_source_directory(../src/server/redis REDIS_LIST)


add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${REDIS_LIST} ${CLIENT_LIST} ${MODEL_LIST} ${SERVER_LIST})

target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)