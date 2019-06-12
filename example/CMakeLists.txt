#vim: et ts =4 sts = 4 sw = 4 tw = 0
cmake_minimum_required(VERSION 3.1)

set(EXAMPLES
	readFromString
	readFromStream
	stringWrite
	streamWrite
   )

add_definitions(-D_GLIBCXX_USE_CXX11_ABI)
set_property(DIRECTORY PROPERTY COMPILE_OPTIONS ${EXTRA_CXX_FLAGS})

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fpermissive -g -fjsoncpp")
else()	
	add_definitions(-D_SCL_SECURE_NO_WARNINGS
		D_CRT_SECURE_NO_WARNINGS
		-D_WIN32_WINNT=0x601
		-D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

foreach (example ${EXAMPLES})
	add_executable(${example} ${example}/${example}.cpp)
endforeach()
