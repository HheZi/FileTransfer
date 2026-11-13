add_executable(app main.cpp)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

target_link_libraries(app PRIVATE ${wxWidgets_LIBRARIES})
target_link_libraries(app PRIVATE proto)
target_link_libraries(app PRIVATE common)
target_link_libraries(app PRIVATE client)
target_link_libraries(app PRIVATE server)
