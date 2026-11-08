add_executable(src discovery_handler.cpp server_discovery.cpp client_connection_handler.cpp file_send_command.cpp offer_file_command.cpp server.cpp main.cpp)

target_link_libraries(src PRIVATE proto)

