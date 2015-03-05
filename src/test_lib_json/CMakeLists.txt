# vim: et ts=4 sts=4 sw=4 tw=0

IF(JSONCPP_LIB_BUILD_SHARED)
  ADD_DEFINITIONS( -DJSON_DLL )
ENDIF(JSONCPP_LIB_BUILD_SHARED)

ADD_EXECUTABLE( jsoncpp_test 
                jsontest.cpp
                jsontest.h
                main.cpp
                )


IF(JSONCPP_LIB_BUILD_SHARED)
    TARGET_LINK_LIBRARIES(jsoncpp_test jsoncpp_lib)
ELSE(JSONCPP_LIB_BUILD_SHARED)
    TARGET_LINK_LIBRARIES(jsoncpp_test jsoncpp_lib_static)
ENDIF(JSONCPP_LIB_BUILD_SHARED)

# another way to solve issue #90
#set_target_properties(jsoncpp_test PROPERTIES COMPILE_FLAGS -ffloat-store)

# Run unit tests in post-build
# (default cmake workflow hides away the test result into a file, resulting in poor dev workflow?!?)
IF(JSONCPP_WITH_POST_BUILD_UNITTEST)
    IF(JSONCPP_LIB_BUILD_SHARED)
        # First, copy the shared lib, for Microsoft.
        # Then, run the test executable.
        ADD_CUSTOM_COMMAND( TARGET jsoncpp_test
                            POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:jsoncpp_lib> $<TARGET_FILE_DIR:jsoncpp_test>
                            COMMAND $<TARGET_FILE:jsoncpp_test>)
    ELSE(JSONCPP_LIB_BUILD_SHARED)
        # Just run the test executable.
        ADD_CUSTOM_COMMAND( TARGET jsoncpp_test
                            POST_BUILD
                            COMMAND $<TARGET_FILE:jsoncpp_test>)
    ENDIF(JSONCPP_LIB_BUILD_SHARED)
ENDIF(JSONCPP_WITH_POST_BUILD_UNITTEST)

SET_TARGET_PROPERTIES(jsoncpp_test PROPERTIES OUTPUT_NAME jsoncpp_test) 
