add_library(common
  id_generator.cpp

  ${PROJECT_SOURCE_DIR}/include/file_transfer/common/encoder_decoder.hpp
  ${PROJECT_SOURCE_DIR}/include/file_transfer/common/id_generator.hpp
)

target_include_directories(common
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)


