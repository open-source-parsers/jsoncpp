#ifndef JSON_VERSION_H_INCLUDED
#define JSON_VERSION_H_INCLUDED

// Note: version must be updated in three places when doing a release. This
// annoying process ensures that amalgamate, CMake, and meson all report the
// correct version.
// 1. /meson.build
// 2. /include/json/version.h
// 3. /CMakeLists.txt
// IMPORTANT: update both the PROJECT_VERSION_STRING as well as the
//            major, minor, and patch fields as appropriate here.

#define PROJECT_VERSION_STRING "1.9.4"
#define PROJECT_VERSION_MAJOR 1
#define PROJECT_VERSION_MINOR 9
#define PROJECT_VERSION_PATCH 4
#define PROJECT_VERSION_QUALIFIER
#define PROJECT_VERSION_HEXA                                                   \
  ((PROJECT_VERSION_MAJOR << 24) | (PROJECT_VERSION_MINOR << 16) |             \
   (PROJECT_VERSION_PATCH << 8))

#ifdef JSONCPP_USING_SECURE_MEMORY
#undef JSONCPP_USING_SECURE_MEMORY
#endif
#define JSONCPP_USING_SECURE_MEMORY 0
// If non-zero, the library zeroes any memory that it has allocated before
// it frees its memory.

#endif // JSON_VERSION_H_INCLUDED
