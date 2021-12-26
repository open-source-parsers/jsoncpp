add_executable(streamWrite streamWrite.cpp)
target_include_directories(streamWrite PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(streamWrite jsoncpp_lib)