aux_source_directory(. CLIENT_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/server/model MODEL_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/server/db DB_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/dbpool DBPOOL_LIST)
# 设置 debug 信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(ChatClient ${CLIENT_LIST} ${MODEL_LIST} ${DB_LIST} ${DBPOOL_LIST})

target_link_libraries(ChatClient ${CMAKE_SOURCE_DIR}/lib/libnet.so mysqlclient hiredis pthread)