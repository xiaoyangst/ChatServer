# 定义了一个SRC_LIST变量，包含了该目录下所有源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/server/model MODEL_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/server/db DB_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/server/redis REDIS_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/dbpool DBPOOL_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/redispool REDISPOOL_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/utils UTILS_LIST)

# 指定生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${REDIS_LIST} ${MODEL_LIST} ${DBPOOL_LIST} ${REDISPOOL_LIST} ${UTILS_LIST})

# 指定可执行文件链接时需要依赖的库文件
target_link_libraries(ChatServer log4cpp  hv mysqlclient hiredis pthread)