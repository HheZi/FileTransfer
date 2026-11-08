find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIRS})

add_library(proto
  Announce.proto
  FileOffer.proto
)

target_link_libraries(proto PUBLIC
    protobuf::libprotobuf
)

target_include_directories(proto PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

protobuf_generate(
    TARGET proto
    IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
