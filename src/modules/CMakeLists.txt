cmake_minimum_required(VERSION 3.28)

add_library(jsoncpp_module)

target_sources(jsoncpp_module
    PUBLIC
        FILE_SET CXX_MODULES FILES
            jsoncpp.cppm
)

target_compile_features(jsoncpp_module PUBLIC cxx_std_20)

if(BUILD_SHARED_LIBS)
    target_link_libraries(jsoncpp_module PUBLIC jsoncpp_lib)
elseif(BUILD_STATIC_LIBS)
    target_link_libraries(jsoncpp_module PUBLIC jsoncpp_static)
endif()

target_include_directories(jsoncpp_module PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(jsoncpp::module ALIAS jsoncpp_module)

# Installation
if(JSONCPP_WITH_CMAKE_PACKAGE)
    set(MODULE_INSTALL_EXPORT EXPORT jsoncpp)
else()
    set(MODULE_INSTALL_EXPORT)
endif()

if(JSONCPP_WITH_INSTALL)
    install(TARGETS jsoncpp_module
        ${MODULE_INSTALL_EXPORT}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jsoncpp/modules
    )
endif()
