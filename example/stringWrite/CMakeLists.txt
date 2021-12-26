add_executable(stringWrite stringWrite.cpp)
target_include_directories(stringWrite PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(stringWrite jsoncpp_lib)