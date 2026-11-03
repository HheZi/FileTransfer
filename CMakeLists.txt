cmake_minimum_required(VERSION 4.2)

project(FileTrasnfer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

add_executable(FileTrasnfer src/proto/FileOffer.pb.cc src/proto/Announce.pb.cc src/discovery_handler.cpp src/server_discovery.cpp src/client_connection_handler.cpp src/file_send_command.cpp src/offer_file_command.cpp src/server.cpp src/main.cpp)

target_link_libraries(FileTrasnfer PRIVATE protobuf::libprotobuf)
