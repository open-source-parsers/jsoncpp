find_package(PythonInterp 2.6)

add_executable(jsontestrunner_exe
               main.cpp
               )

if(BUILD_SHARED_LIBS)
    add_definitions( -DJSON_DLL )
endif()
target_link_libraries(jsontestrunner_exe jsoncpp_lib)

set_target_properties(jsontestrunner_exe PROPERTIES OUTPUT_NAME jsontestrunner_exe)

if(PYTHONINTERP_FOUND)
    # Run end to end parser/writer tests
    set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../test)
    set(RUNJSONTESTS_PATH ${TEST_DIR}/runjsontests.py)
    add_custom_target(jsoncpp_readerwriter_tests
                      "${PYTHON_EXECUTABLE}" -B "${RUNJSONTESTS_PATH}" $<TARGET_FILE:jsontestrunner_exe> "${TEST_DIR}/data"
                      DEPENDS jsontestrunner_exe jsoncpp_test
                      )
    add_custom_target(jsoncpp_check DEPENDS jsoncpp_readerwriter_tests)
endif()
