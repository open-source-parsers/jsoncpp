OPTION(JSONCPP_LIB_BUILD_SHARED "Build jsoncpp_lib as a shared library." OFF)
IF(JSONCPP_LIB_BUILD_SHARED)
  SET(JSONCPP_LIB_TYPE SHARED)
  ADD_DEFINITIONS( -DJSON_DLL_BUILD )
ELSE(JSONCPP_LIB_BUILD_SHARED)
  SET(JSONCPP_LIB_TYPE STATIC)
ENDIF(JSONCPP_LIB_BUILD_SHARED)


if( CMAKE_COMPILER_IS_GNUCXX )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=strict-aliasing")
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

ADD_LIBRARY( jsoncpp_lib ${JSONCPP_LIB_TYPE}
             ${PUBLIC_HEADERS} 
             json_tool.h
             json_reader.cpp
             json_batchallocator.h
             json_valueiterator.inl
             json_value.cpp
             json_writer.cpp
             version.h.in
             )
SET_TARGET_PROPERTIES( jsoncpp_lib PROPERTIES OUTPUT_NAME jsoncpp )
SET_TARGET_PROPERTIES( jsoncpp_lib PROPERTIES VERSION ${JSONCPP_VERSION} SOVERSION ${JSONCPP_VERSION_MAJOR} )

# Install instructions for this target
IF(JSONCPP_WITH_CMAKE_PACKAGE)
    TARGET_INCLUDE_DIRECTORIES( jsoncpp_lib
                                PUBLIC $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
                                       $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${JSON_INCLUDE_DIR}>
                                )
    SET(INSTALL_EXPORT EXPORT jsoncpp)
ELSE(JSONCPP_WITH_CMAKE_PACKAGE)
    SET(INSTALL_EXPORT)
ENDIF(JSONCPP_WITH_CMAKE_PACKAGE)

INSTALL( TARGETS jsoncpp_lib ${INSTALL_EXPORT}
         RUNTIME DESTINATION ${RUNTIME_INSTALL_DIR}
         LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
         ARCHIVE DESTINATION ${ARCHIVE_INSTALL_DIR}
)
