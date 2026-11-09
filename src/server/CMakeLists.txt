add_library(server
  client_connection_handler.cpp
  server_discovery.cpp
  server.cpp

  ${PROJECT_SOURCE_DIR}/include/file_transfer/server/server.hpp
  ${PROJECT_SOURCE_DIR}/include/file_transfer/server/server_discovery.hpp
  ${PROJECT_SOURCE_DIR}/include/file_transfer/server/client_connection_handler.hpp
)

target_include_directories(server
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(server PRIVATE common)
target_link_libraries(server PRIVATE proto)

