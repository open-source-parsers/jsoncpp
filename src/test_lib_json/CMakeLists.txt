# vim: et ts=4 sts=4 sw=4 tw=0

ADD_EXECUTABLE(jsoncpp_test
        jsontest.cpp
        jsontest.h
        fuzz.cpp
        fuzz.h
        main.cpp
        )

TARGET_LINK_LIBRARIES(jsoncpp_test PRIVATE jsoncpp::framework)

# another way to solve issue #90
#set_target_properties(jsoncpp_test PROPERTIES COMPILE_FLAGS -ffloat-store)

## Create tests for dashboard submission, allows easy review of CI results https://my.cdash.org/index.php?project=jsoncpp
ADD_TEST(NAME jsoncpp_test
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:jsoncpp_test>
        )
SET_TARGET_PROPERTIES(jsoncpp_test PROPERTIES OUTPUT_NAME jsoncpp_test)

# Run unit tests in post-build
# (default cmake workflow hides away the test result into a file, resulting in poor dev workflow?!?)
IF (JSONCPP_WITH_POST_BUILD_UNITTEST)
    ADD_CUSTOM_COMMAND(TARGET jsoncpp_test
            POST_BUILD
            COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:jsoncpp_test>
            )
ENDIF ()
