# The new Python3 module is much more robust than the previous PythonInterp
FIND_PACKAGE(Python3 COMPONENTS Interpreter)
# Set variables for backwards compatibility with cmake < 3.12.0
SET(PYTHONINTERP_FOUND ${Python3_Interpreter_FOUND})
SET(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})

ADD_EXECUTABLE(jsontestrunner_exe main.cpp)
TARGET_LINK_LIBRARIES(jsontestrunner_exe PRIVATE jsoncpp::framework)

set_target_properties(jsontestrunner_exe PROPERTIES OUTPUT_NAME jsontestrunner_exe)

if(PYTHONINTERP_FOUND)
    # Run end to end parser/writer tests
    set(RUNJSONTESTS_PATH ${JSONCPP_ROOT_DIR}/test/runjsontests.py)

    # Run unit tests in post-build
    # (default cmake workflow hides away the test result into a file, resulting in poor dev workflow?!?)
    add_custom_target(jsoncpp_readerwriter_tests
        "${PYTHON_EXECUTABLE}" -B "${RUNJSONTESTS_PATH}" $<TARGET_FILE:jsontestrunner_exe> "${JSONCPP_ROOT_DIR}/test/data"
        DEPENDS jsontestrunner_exe jsoncpp_test
    )
    add_custom_target(jsoncpp_check DEPENDS jsoncpp_readerwriter_tests)

    ## Create tests for dashboard submission, allows easy review of CI results https://my.cdash.org/index.php?project=jsoncpp
    add_test(NAME jsoncpp_readerwriter
        COMMAND "${PYTHON_EXECUTABLE}" -B "${RUNJSONTESTS_PATH}" $<TARGET_FILE:jsontestrunner_exe> "${JSONCPP_ROOT_DIR}/test/data"
        WORKING_DIRECTORY "${JSONCPP_ROOT_DIR}/test/data"
    )
    add_test(NAME jsoncpp_readerwriter_json_checker
        COMMAND "${PYTHON_EXECUTABLE}" -B "${RUNJSONTESTS_PATH}" --with-json-checker  $<TARGET_FILE:jsontestrunner_exe> "${JSONCPP_ROOT_DIR}/test/data"
        WORKING_DIRECTORY "${JSONCPP_ROOT_DIR}/test/data"
    )
endif()
