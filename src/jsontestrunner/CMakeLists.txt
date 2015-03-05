FIND_PACKAGE(PythonInterp 2.6)

IF(JSONCPP_LIB_BUILD_SHARED)
  ADD_DEFINITIONS( -DJSON_DLL )
ENDIF(JSONCPP_LIB_BUILD_SHARED)

ADD_EXECUTABLE(jsontestrunner_exe
               main.cpp
               )

IF(JSONCPP_LIB_BUILD_SHARED)
    TARGET_LINK_LIBRARIES(jsontestrunner_exe jsoncpp_lib)
ELSE(JSONCPP_LIB_BUILD_SHARED)
    TARGET_LINK_LIBRARIES(jsontestrunner_exe jsoncpp_lib_static)
ENDIF(JSONCPP_LIB_BUILD_SHARED)

SET_TARGET_PROPERTIES(jsontestrunner_exe PROPERTIES OUTPUT_NAME jsontestrunner_exe)

IF(PYTHONINTERP_FOUND)
    # Run end to end parser/writer tests
    SET(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../test)
    SET(RUNJSONTESTS_PATH ${TEST_DIR}/runjsontests.py)
    ADD_CUSTOM_TARGET(jsoncpp_readerwriter_tests
                      "${PYTHON_EXECUTABLE}" -B "${RUNJSONTESTS_PATH}" $<TARGET_FILE:jsontestrunner_exe> "${TEST_DIR}/data"
                      DEPENDS jsontestrunner_exe jsoncpp_test
                      )
    ADD_CUSTOM_TARGET(jsoncpp_check DEPENDS jsoncpp_readerwriter_tests)
ENDIF(PYTHONINTERP_FOUND)
