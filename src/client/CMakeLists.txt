aux_source_directory(. CLIENT_LIST)
aux_source_directory(../server/model MODEL_LIST)
aux_source_directory(../server/db DB_LIST)

add_executable(ChatClient ${CLIENT_LIST} ${MODEL_LIST} ${DB_LIST})

target_link_libraries(ChatClient muduo_net muduo_base mysqlclient hiredis pthread)