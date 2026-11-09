add_library(client
  discovery_handler.cpp
  file_send_command.cpp
  offer_file_command.cpp

  ${PROJECT_SOURCE_DIR}/include/file_transfer/client/connection.hpp
  ${PROJECT_SOURCE_DIR}/include/file_transfer/client/discovery_handler.hpp
  ${PROJECT_SOURCE_DIR}/include/file_transfer/client/file_send_command.hpp
  ${PROJECT_SOURCE_DIR}/include/file_transfer/client/offer_file_command.hpp
)

target_include_directories(client
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(client PRIVATE common)
target_link_libraries(client PRIVATE proto)
