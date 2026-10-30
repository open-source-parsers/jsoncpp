# vim: et ts=4 sts=4 sw=4 tw=0

if(JSONCPP_WITH_TESTS)

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

endif()  # JSONCPP_WITH_TESTS

# Standalone fuzz target (used by OSS-Fuzz). The JSONCPP_WITH_FUZZING option
# is declared in the top-level CMakeLists.txt, so the fuzz target can also be
# built with -DJSONCPP_WITH_TESTS=OFF.
#
# Requires a fuzzing engine (JSONCPP_WITH_FUZZING=ON fails at configure time
# without one, since the target has no main()): compile everything with the
# engine's coverage instrumentation only (e.g.
# -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer-no-link") and link the engine itself
# into the fuzz target (e.g. -DJSONCPP_FUZZ_ENGINE="-fsanitize=fuzzer"), so
# that the engine's main() is not pulled into compiler checks and other
# executables.
#
# The libprotobuf-mutator based fuzzer (jsoncpp_fuzz_proto.cc, json.proto,
# json_proto_converter.*) is built by OSS-Fuzz separately with LPM; the
# sources are kept here so they live with the other jsoncpp fuzzers.
set(JSONCPP_FUZZ_ENGINE "" CACHE STRING
    "Fuzzing engine link flags for jsoncpp_fuzzer, e.g. -fsanitize=fuzzer")
if(JSONCPP_WITH_FUZZING)
    # jsoncpp_fuzzer has no main(): the fuzzing engine provides it, so its
    # link flags are required and there is nothing buildable without them.
    if(NOT JSONCPP_FUZZ_ENGINE)
        message(FATAL_ERROR
            "JSONCPP_WITH_FUZZING=ON requires JSONCPP_FUZZ_ENGINE linking "
            "flags, e.g. -DJSONCPP_FUZZ_ENGINE=\"-fsanitize=fuzzer\"")
    endif()
    add_executable(jsoncpp_fuzzer
        fuzz.cpp
        fuzz.h
    )
    if(BUILD_SHARED_LIBS)
        target_link_libraries(jsoncpp_fuzzer jsoncpp_lib)
    elseif(BUILD_STATIC_LIBS)
        target_link_libraries(jsoncpp_fuzzer jsoncpp_static)
    else()
        # Linking to an object library via target_link_libraries needs
        # CMake 3.12 or newer.
        if(CMAKE_VERSION VERSION_LESS 3.12.0)
            message(FATAL_ERROR
                "JSONCPP_WITH_FUZZING with object libraries only requires "
                "CMake 3.12 or newer")
        endif()
        target_link_libraries(jsoncpp_fuzzer jsoncpp_object)
    endif()
    set_target_properties(jsoncpp_fuzzer PROPERTIES
        LINK_FLAGS "${JSONCPP_FUZZ_ENGINE}"
    )
    # Ship the dictionary next to the fuzzer binary.
    add_custom_command(TARGET jsoncpp_fuzzer
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/fuzz.dict
                $<TARGET_FILE_DIR:jsoncpp_fuzzer>/jsoncpp_fuzzer.dict
    )
endif()
