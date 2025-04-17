aux_source_directory(. CLIENT_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/server/model MODEL_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/server/db DB_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/dbpool DBPOOL_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/utils UTILS_LIST)
# 设置 debug 信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(ChatClient ${CLIENT_LIST} ${MODEL_LIST} ${DB_LIST} ${DBPOOL_LIST} ${UTILS_LIST})

target_link_libraries(ChatClient hv mysqlclient hiredis pthread)