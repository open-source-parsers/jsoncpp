# vim: et ts=4 sts=4 sw=4 tw=0

cmake_minimum_required(VERSION 3.1)

# Ensures that CMAKE_BUILD_TYPE has a default value
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Coverage.")
endif()

project(jsoncpp)
enable_testing()

option(JSONCPP_WITH_TESTS "Compile and (for jsoncpp_check) run JsonCpp test executables" ON)
option(JSONCPP_WITH_POST_BUILD_UNITTEST "Automatically run unit-tests as a post build step" ON)
option(JSONCPP_WITH_WARNING_AS_ERROR "Force compilation to fail if a warning occurs" OFF)
option(JSONCPP_WITH_STRICT_ISO "Issue all the warnings demanded by strict ISO C and ISO C++" ON)
option(JSONCPP_WITH_PKGCONFIG_SUPPORT "Generate and install .pc files" ON)
option(JSONCPP_WITH_CMAKE_PACKAGE "Generate and install cmake package files" ON)
option(BUILD_SHARED_LIBS "Build jsoncpp_lib as a shared library." OFF)

# Enable runtime search path support for dynamic libraries on OSX
if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
endif()

# Adhere to GNU filesystem layout conventions
include(GNUInstallDirs)

set(DEBUG_LIBNAME_SUFFIX "" CACHE STRING "Optional suffix to append to the library name for a debug build")

# Set variable named ${VAR_NAME} to value ${VALUE}
function(set_using_dynamic_name VAR_NAME VALUE)
    set( "${VAR_NAME}" "${VALUE}" PARENT_SCOPE)
endfunction()

# Extract major, minor, patch from version text
# Parse a version string "X.Y.Z" and outputs
# version parts in ${OUPUT_PREFIX}_MAJOR, _MINOR, _PATCH.
# If parse succeeds then ${OUPUT_PREFIX}_FOUND is TRUE.
macro(jsoncpp_parse_version VERSION_TEXT OUPUT_PREFIX)
    set(VERSION_REGEX "[0-9]+\\.[0-9]+\\.[0-9]+(-[a-zA-Z0-9_]+)?")
    if( ${VERSION_TEXT} MATCHES ${VERSION_REGEX} )
        string(REGEX MATCHALL "[0-9]+|-([A-Za-z0-9_]+)" VERSION_PARTS ${VERSION_TEXT})
        list(GET VERSION_PARTS 0 ${OUPUT_PREFIX}_MAJOR)
        list(GET VERSION_PARTS 1 ${OUPUT_PREFIX}_MINOR)
        list(GET VERSION_PARTS 2 ${OUPUT_PREFIX}_PATCH)
        set_using_dynamic_name( "${OUPUT_PREFIX}_FOUND" TRUE )
    else( ${VERSION_TEXT} MATCHES ${VERSION_REGEX} )
        set_using_dynamic_name( "${OUPUT_PREFIX}_FOUND" FALSE )
    endif()
endmacro()

# Read out version from "version" file
#file(STRINGS "version" JSONCPP_VERSION)
#set( JSONCPP_VERSION_MAJOR X )
#set( JSONCPP_VERSION_MINOR Y )
#set( JSONCPP_VERSION_PATCH Z )
set( JSONCPP_VERSION 1.8.4 )
jsoncpp_parse_version( ${JSONCPP_VERSION} JSONCPP_VERSION )
#if(NOT JSONCPP_VERSION_FOUND)
#    message(FATAL_ERROR "Failed to parse version string properly. Expect X.Y.Z")
#endif(NOT JSONCPP_VERSION_FOUND)
set( JSONCPP_SOVERSION 19 )
set( JSONCPP_USE_SECURE_MEMORY "0" CACHE STRING "-D...=1 to use memory-wiping allocator for STL" )

message(STATUS "JsonCpp Version: ${JSONCPP_VERSION_MAJOR}.${JSONCPP_VERSION_MINOR}.${JSONCPP_VERSION_PATCH}")
# File version.h is only regenerated on CMake configure step
configure_file( "${PROJECT_SOURCE_DIR}/src/lib_json/version.h.in"
                "${PROJECT_BINARY_DIR}/include/json/version.h"
                NEWLINE_STYLE UNIX )
configure_file( "${PROJECT_SOURCE_DIR}/version.in"
                "${PROJECT_BINARY_DIR}/version"
                NEWLINE_STYLE UNIX )

macro(UseCompilationWarningAsError)
    if(MSVC)
        # Only enabled in debug because some old versions of VS STL generate
        # warnings when compiled in release configuration.
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /WX ")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
        if(JSONCPP_WITH_STRICT_ISO)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors")
        endif()
    endif()
endmacro()

# Include our configuration header
include_directories( ${jsoncpp_SOURCE_DIR}/include )

if(MSVC)
    # Only enabled in debug because some old versions of VS STL generate
    # unreachable code warning when compiled in release configuration.
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 ")
endif()

# Require C++11 support, prefer ISO C++ over GNU variants,
# as relying solely on ISO C++ is more portable.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using regular Clang or AppleClang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Wshadow -Werror=conversion -Werror=sign-compare")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Wshadow -Wextra")
    # not yet ready for -Wsign-conversion

    if(JSONCPP_WITH_STRICT_ISO)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
    endif()
    if(JSONCPP_WITH_WARNING_AS_ERROR)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=conversion")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    # using Intel compiler
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Wshadow -Wextra -Werror=conversion")

    if(JSONCPP_WITH_STRICT_ISO AND NOT JSONCPP_WITH_WARNING_AS_ERROR)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
    endif()
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

if(JSONCPP_WITH_WARNING_AS_ERROR)
    UseCompilationWarningAsError()
endif()

if(JSONCPP_WITH_PKGCONFIG_SUPPORT)
    configure_file(
        "pkg-config/jsoncpp.pc.in"
        "pkg-config/jsoncpp.pc"
        @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pkg-config/jsoncpp.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()

if(JSONCPP_WITH_CMAKE_PACKAGE)
        install(EXPORT jsoncpp
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jsoncpp
                FILE        jsoncppConfig.cmake)
endif()

# Build the different applications
add_subdirectory( src )

#install the includes
add_subdirectory( include )
