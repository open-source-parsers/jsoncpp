add_subdirectory(lib_json)
if(JSONCPP_WITH_TESTS)
    add_subdirectory(jsontestrunner)
    add_subdirectory(test_lib_json)
endif()

#install the example
if(JSONCPP_WITH_MODULE)
    add_subdirectory(modules)
endif()
