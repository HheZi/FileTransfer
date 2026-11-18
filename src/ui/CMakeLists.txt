find_package(wxWidgets REQUIRED COMPONENTS core base)

add_library(ui
  main_frame.cpp
  file_transfer_app.cpp

  ${PROJECT_SOURCE_DIR}/include/file_transfer/ui/main_frame.hpp
  ${PROJECT_SOURCE_DIR}/include/file_transfer/ui/file_transfer_app.hpp
)

target_include_directories(ui
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ui
  PUBLIC
    wxWidgets::wxWidgets
  PRIVATE
    proto
    common
    client
    server
)
