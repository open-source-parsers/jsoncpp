if( CMAKE_COMPILER_IS_GNUCXX )
  #Get compiler version.
  execute_process( COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
                   OUTPUT_VARIABLE GNUCXX_VERSION )

  #-Werror=* was introduced -after- GCC 4.1.2
  if( GNUCXX_VERSION VERSION_GREATER 4.1.2 )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=strict-aliasing")
  endif()
endif( CMAKE_COMPILER_IS_GNUCXX )

include(CheckIncludeFileCXX)
include(CheckTypeSize)
include(CheckStructHasMember)
include(CheckCXXSymbolExists)

check_include_file_cxx(clocale HAVE_CLOCALE)
check_cxx_symbol_exists(localeconv clocale HAVE_LOCALECONV)

if(CMAKE_VERSION VERSION_LESS 3.0.0)
	# The "LANGUAGE CXX" parameter is not supported in CMake versions below 3,
	# so the C compiler and header has to be used.
	check_include_file(locale.h HAVE_LOCALE_H)
	set(CMAKE_EXTRA_INCLUDE_FILES locale.h)
	check_type_size("struct lconv" LCONV_SIZE)
	unset(CMAKE_EXTRA_INCLUDE_FILES)
	check_struct_has_member("struct lconv" decimal_point locale.h HAVE_DECIMAL_POINT)
else()
	set(CMAKE_EXTRA_INCLUDE_FILES clocale)
	check_type_size(lconv LCONV_SIZE LANGUAGE CXX)
	unset(CMAKE_EXTRA_INCLUDE_FILES)
	check_struct_has_member(lconv decimal_point clocale HAVE_DECIMAL_POINT LANGUAGE CXX)
endif()

if(NOT (HAVE_CLOCALE AND HAVE_LCONV_SIZE AND HAVE_DECIMAL_POINT AND HAVE_LOCALECONV))
	message(WARNING "Locale functionality is not supported")
	add_definitions(-DJSONCPP_NO_LOCALE_SUPPORT)
endif()

SET( JSONCPP_INCLUDE_DIR ../../include )

SET( PUBLIC_HEADERS
    ${JSONCPP_INCLUDE_DIR}/json/config.h
    ${JSONCPP_INCLUDE_DIR}/json/forwards.h
    ${JSONCPP_INCLUDE_DIR}/json/features.h
    ${JSONCPP_INCLUDE_DIR}/json/value.h
    ${JSONCPP_INCLUDE_DIR}/json/reader.h
    ${JSONCPP_INCLUDE_DIR}/json/writer.h
    ${JSONCPP_INCLUDE_DIR}/json/assertions.h
    ${JSONCPP_INCLUDE_DIR}/json/version.h
    )

SOURCE_GROUP( "Public API" FILES ${PUBLIC_HEADERS} )

SET(jsoncpp_sources
                json_tool.h
                json_reader.cpp
                json_valueiterator.inl
                json_value.cpp
                json_writer.cpp
                version.h.in)

# Install instructions for this target
IF(JSONCPP_WITH_CMAKE_PACKAGE)
    SET(INSTALL_EXPORT EXPORT jsoncpp)
ELSE(JSONCPP_WITH_CMAKE_PACKAGE)
    SET(INSTALL_EXPORT)
ENDIF()

IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS( -DJSON_DLL_BUILD )
    ADD_LIBRARY(jsoncpp_lib SHARED ${PUBLIC_HEADERS} ${jsoncpp_sources})
    SET_TARGET_PROPERTIES( jsoncpp_lib PROPERTIES VERSION ${JSONCPP_VERSION} SOVERSION ${JSONCPP_SOVERSION})
    SET_TARGET_PROPERTIES( jsoncpp_lib PROPERTIES OUTPUT_NAME jsoncpp
                           DEBUG_OUTPUT_NAME jsoncpp${DEBUG_LIBNAME_SUFFIX} )

    # Set library's runtime search path on OSX
    IF(APPLE)
        SET_TARGET_PROPERTIES( jsoncpp_lib PROPERTIES INSTALL_RPATH "@loader_path/." )
    ENDIF()

    INSTALL( TARGETS jsoncpp_lib ${INSTALL_EXPORT}
         RUNTIME DESTINATION ${RUNTIME_INSTALL_DIR}
         LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
         ARCHIVE DESTINATION ${ARCHIVE_INSTALL_DIR})

    IF(NOT CMAKE_VERSION VERSION_LESS 2.8.11)
        TARGET_INCLUDE_DIRECTORIES( jsoncpp_lib PUBLIC
                                   $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${JSONCPP_INCLUDE_DIR}>)
    ENDIF()

ENDIF()

IF(BUILD_STATIC_LIBS)
    ADD_LIBRARY(jsoncpp_lib_static STATIC ${PUBLIC_HEADERS} ${jsoncpp_sources})
    SET_TARGET_PROPERTIES( jsoncpp_lib_static PROPERTIES VERSION ${JSONCPP_VERSION} SOVERSION ${JSONCPP_SOVERSION})
    SET_TARGET_PROPERTIES( jsoncpp_lib_static PROPERTIES OUTPUT_NAME jsoncpp
                           DEBUG_OUTPUT_NAME jsoncpp${DEBUG_LIBNAME_SUFFIX} )

    INSTALL( TARGETS jsoncpp_lib_static ${INSTALL_EXPORT}
         RUNTIME DESTINATION ${RUNTIME_INSTALL_DIR}
         LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
             ARCHIVE DESTINATION ${ARCHIVE_INSTALL_DIR})

    IF(NOT CMAKE_VERSION VERSION_LESS 2.8.11)
        TARGET_INCLUDE_DIRECTORIES( jsoncpp_lib_static PUBLIC
                                $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${JSONCPP_INCLUDE_DIR}>
                                )
     ENDIF()

ENDIF()
