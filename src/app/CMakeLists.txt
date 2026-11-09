add_executable(app main.cpp)

target_link_libraries(app PRIVATE proto)
target_link_libraries(app PRIVATE common)
target_link_libraries(app PRIVATE client)
target_link_libraries(app PRIVATE server)
