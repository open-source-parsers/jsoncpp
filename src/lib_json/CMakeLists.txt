if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.1.2)
    #-Werror=* was introduced -after- GCC 4.1.2
    add_compile_options("-Werror=strict-aliasing")
endif()

include(CheckIncludeFileCXX)
include(CheckTypeSize)
include(CheckStructHasMember)
include(CheckCXXSymbolExists)

check_include_file_cxx(clocale HAVE_CLOCALE)
check_cxx_symbol_exists(localeconv clocale HAVE_LOCALECONV)

set(CMAKE_EXTRA_INCLUDE_FILES clocale)
check_type_size(lconv LCONV_SIZE LANGUAGE CXX)
unset(CMAKE_EXTRA_INCLUDE_FILES)
check_struct_has_member(lconv decimal_point clocale HAVE_DECIMAL_POINT LANGUAGE CXX)

if(NOT (HAVE_CLOCALE AND HAVE_LCONV_SIZE AND HAVE_DECIMAL_POINT AND HAVE_LOCALECONV))
    message(WARNING "Locale functionality is not supported")
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12.0)
        add_compile_definitions(JSONCPP_NO_LOCALE_SUPPORT)
    else()
        add_definitions(-DJSONCPP_NO_LOCALE_SUPPORT)
    endif()
endif()


# Specify compiler features required when compiling a given target.
# See https://cmake.org/cmake/help/v3.1/prop_gbl/CMAKE_CXX_KNOWN_FEATURES.html#prop_gbl:CMAKE_CXX_KNOWN_FEATURES
# for complete list of features available
list(APPEND REQUIRED_FEATURES
        #MSVC 1900 cxx_alignas # Alignment control alignas, as defined in N2341.
        #MSVC 1900 cxx_alignof # Alignment control alignof, as defined in N2341.
        #MSVC 1900 cxx_attributes # Generic attributes, as defined in N2761.
        cxx_auto_type # Automatic type deduction, as defined in N1984.
        #MSVC 1900 cxx_constexpr # Constant expressions, as defined in N2235.
        cxx_decltype # Decltype, as defined in N2343.
        cxx_default_function_template_args # Default template arguments for function templates, as defined in DR226
        cxx_defaulted_functions # Defaulted functions, as defined in N2346.
        #MSVC 1900 cxx_defaulted_move_initializers # Defaulted move initializers, as defined in N3053.
        cxx_delegating_constructors # Delegating constructors, as defined in N1986.
        #MSVC 1900 cxx_deleted_functions # Deleted functions, as defined in N2346.
        cxx_enum_forward_declarations # Enum forward declarations, as defined in N2764.
        cxx_explicit_conversions # Explicit conversion operators, as defined in N2437.
        cxx_extended_friend_declarations # Extended friend declarations, as defined in N1791.
        cxx_extern_templates # Extern templates, as defined in N1987.
        cxx_final # Override control final keyword, as defined in N2928, N3206 and N3272.
        #MSVC 1900 cxx_func_identifier # Predefined __func__ identifier, as defined in N2340.
        #MSVC 1900 cxx_generalized_initializers # Initializer lists, as defined in N2672.
        #MSVC 1900 cxx_inheriting_constructors # Inheriting constructors, as defined in N2540.
        #MSVC 1900 cxx_inline_namespaces # Inline namespaces, as defined in N2535.
        cxx_lambdas # Lambda functions, as defined in N2927.
        #MSVC 1900 cxx_local_type_template_args # Local and unnamed types as template arguments, as defined in N2657.
        cxx_long_long_type # long long type, as defined in N1811.
        #MSVC 1900 cxx_noexcept # Exception specifications, as defined in N3050.
        #MSVC 1900 cxx_nonstatic_member_init # Non-static data member initialization, as defined in N2756.
        cxx_nullptr # Null pointer, as defined in N2431.
        cxx_override # Override control override keyword, as defined in N2928, N3206 and N3272.
        cxx_range_for # Range-based for, as defined in N2930.
        cxx_raw_string_literals # Raw string literals, as defined in N2442.
        #MSVC 1900 cxx_reference_qualified_functions # Reference qualified functions, as defined in N2439.
        cxx_right_angle_brackets # Right angle bracket parsing, as defined in N1757.
        cxx_rvalue_references # R-value references, as defined in N2118.
        #MSVC 1900 cxx_sizeof_member # Size of non-static data members, as defined in N2253.
        cxx_static_assert # Static assert, as defined in N1720.
        cxx_strong_enums # Strongly typed enums, as defined in N2347.
        #MSVC 1900 cxx_thread_local # Thread-local variables, as defined in N2659.
        cxx_trailing_return_types # Automatic function return type, as defined in N2541.
        #MSVC 1900 cxx_unicode_literals # Unicode string literals, as defined in N2442.
        cxx_uniform_initialization # Uniform initialization, as defined in N2640.
        #MSVC 1900 cxx_unrestricted_unions # Unrestricted unions, as defined in N2544.
        #MSVC 1900 cxx_user_literals # User-defined literals, as defined in N2765.
        cxx_variadic_macros # Variadic macros, as defined in N1653.
        cxx_variadic_templates # Variadic templates, as defined in N2242.
)

# The OBJECT's can be used to compile the sources in the list given to add_library
# to object files, but then neither archiving them into a static library nor
# linking them into a shared object. The use of object libraries is
# particularly useful if one needs to create both static and shared libraries
# in one go.

# Property: OBJECT


add_library(jsoncpp.framework.object OBJECT
        json_reader.cpp
        json_value.cpp
        json_writer.cpp
        )

set_target_properties(jsoncpp.framework.object PROPERTIES CXX_STANDARD 11)
set_target_properties(jsoncpp.framework.object PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(jsoncpp.framework.object PROPERTIES SOVERSION ${PROJECT_SOVERSION})

target_compile_features(jsoncpp.framework.object PUBLIC ${REQUIRED_FEATURES})
target_include_directories(jsoncpp.framework.object PUBLIC $<BUILD_INTERFACE:${JSONCPP_ROOT_DIR}/include>)


if (JSONCPP_BUILD_SHARED_LIBS)

    # CMake link shared library on Windows
    # Ref: https://stackoverflow.com/a/41618677
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

    # The POSITION_INDEPENDENT_CODE property determines whether position independent executables or
    # shared libraries will be created. This property is True by default for SHARED and MODULE library
    # targets and False otherwise.

    # Property: SHARED, POSITION_INDEPENDENT_CODE
    add_library(jsoncpp.framework SHARED $<TARGET_OBJECTS:jsoncpp.framework.object>)

    # Needed for legacy code
    target_compile_definitions(jsoncpp.framework PUBLIC JSON_DLL_BUILD)
    target_compile_definitions(jsoncpp.framework PUBLIC JSON_DLL)

else ()

    add_library(jsoncpp.framework STATIC $<TARGET_OBJECTS:jsoncpp.framework.object>)

endif ()

add_library(jsoncpp::framework ALIAS jsoncpp.framework)
set_target_properties(jsoncpp.framework PROPERTIES CXX_STANDARD 11)
set_target_properties(jsoncpp.framework PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(jsoncpp.framework PROPERTIES SOVERSION ${PROJECT_SOVERSION})

target_compile_features(jsoncpp.framework PUBLIC ${REQUIRED_FEATURES})
# It is necessary to repeat these instructions again, in case of not doing it, the executables
# will be unable to locate the Headers needed to perform the linking
target_include_directories(jsoncpp.framework PUBLIC $<BUILD_INTERFACE:${JSONCPP_ROOT_DIR}/include>)