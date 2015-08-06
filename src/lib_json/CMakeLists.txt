if( CMAKE_COMPILER_IS_GNUCXX )
  #Get compiler version.
  execute_process( COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
                   OUTPUT_VARIABLE GNUCXX_VERSION )

  #-Werror=* was introduced -after- GCC 4.1.2
  if( GNUCXX_VERSION VERSION_GREATER 4.1.2 )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=strict-aliasing")
  endif()
endif( CMAKE_COMPILER_IS_GNUCXX )

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
ENDIF(JSONCPP_WITH_CMAKE_PACKAGE)

set( project_name jsoncpp )

IF(JSONCPP_LIBRARY_TYPE STREQUAL "SHARED")
    ADD_DEFINITIONS( -DJSON_DLL_BUILD )
    SET(lib_type SHARED)
ELSEIF(JSONCPP_LIBRARY_TYPE STREQUAL "STATIC")
    SET(lib_type STATIC)
ENDIF()

ADD_LIBRARY(${project_name} ${lib_type} ${PUBLIC_HEADERS} ${jsoncpp_sources})

SET_TARGET_PROPERTIES(${project_name} PROPERTIES
    VERSION ${JSONCPP_VERSION}
    SOVERSION ${JSONCPP_VERSION_MAJOR}
    OUTPUT_NAME jsoncpp
    DEBUG_OUTPUT_NAME jsoncpp${DEBUG_LIBNAME_SUFFIX}
)

INSTALL(TARGETS ${project_name} ${INSTALL_EXPORT}
    RUNTIME DESTINATION ${RUNTIME_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${ARCHIVE_INSTALL_DIR}
)

IF(NOT CMAKE_VERSION VERSION_LESS 2.8.11)
    TARGET_INCLUDE_DIRECTORIES( ${project_name} PUBLIC
        $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${JSONCPP_INCLUDE_DIR}>
    )
ENDIF()
