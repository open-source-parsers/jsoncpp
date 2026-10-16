# vim: et ts=4 sts=4 sw=4 tw=0

add_executable(jsoncpp_test
    jsontest.cpp
    jsontest.h
    fuzz.cpp
    fuzz.h
    main.cpp
)


if(BUILD_SHARED_LIBS)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12.0)
        add_compile_definitions( JSON_DLL )
    else()
        add_definitions( -DJSON_DLL )
    endif()
    target_link_libraries(jsoncpp_test jsoncpp_lib)
else()
    target_link_libraries(jsoncpp_test jsoncpp_static)
endif()

# another way to solve issue #90
#set_target_properties(jsoncpp_test PROPERTIES COMPILE_FLAGS -ffloat-store)

## Create tests for dashboard submission, allows easy review of CI results https://my.cdash.org/index.php?project=jsoncpp
add_test(NAME jsoncpp_test
    COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:jsoncpp_test>
)
set_target_properties(jsoncpp_test PROPERTIES OUTPUT_NAME jsoncpp_test)

# Run unit tests in post-build
# (default cmake workflow hides away the test result into a file, resulting in poor dev workflow?!?)
if(JSONCPP_WITH_POST_BUILD_UNITTEST)
    add_custom_command(TARGET jsoncpp_test
        POST_BUILD
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:jsoncpp_test>
    )
endif()

# Standalone fuzz target (used by OSS-Fuzz). Requires a fuzzing engine, e.g.
#   cmake -DJSONCPP_WITH_FUZZING=ON -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer" ..
# The libprotobuf-mutator based fuzzer (jsoncpp_fuzz_proto.cc, json.proto,
# json_proto_converter.*) is built by OSS-Fuzz separately with LPM; the sources
# are kept here so they live with the other jsoncpp fuzzers.
option(JSONCPP_WITH_FUZZING "Build the jsoncpp_fuzzer fuzz target (needs a fuzzing engine)" OFF)
if(JSONCPP_WITH_FUZZING)
    add_executable(jsoncpp_fuzzer
        fuzz.cpp
        fuzz.h
    )
    if(BUILD_SHARED_LIBS)
        target_link_libraries(jsoncpp_fuzzer jsoncpp_lib)
    else()
        target_link_libraries(jsoncpp_fuzzer jsoncpp_static)
    endif()
    # Ship the dictionary next to the fuzzer binary.
    add_custom_command(TARGET jsoncpp_fuzzer
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/fuzz.dict
                $<TARGET_FILE_DIR:jsoncpp_fuzzer>/jsoncpp_fuzzer.dict
    )
endif()
