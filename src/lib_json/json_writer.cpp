// Copyright 2011 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/writer.h>
#include "json_tool.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <iomanip>
#include <memory>
#include <sstream>
#include <utility>
#include <set>
#include <cassert>
#include <cstring>
#include <cstdio>

#if defined(_MSC_VER) && _MSC_VER >= 1200 && _MSC_VER < 1800 // Between VC++ 6.0 and VC++ 11.0
#include <float.h>
#define isfinite _finite
#elif defined(__sun) && defined(__SVR4) //Solaris
#if !defined(isfinite)
#include <ieeefp.h>
#define isfinite finite
#endif
#elif defined(_AIX)
#if !defined(isfinite)
#include <math.h>
#define isfinite finite
#endif
#elif defined(__hpux)
#if !defined(isfinite)
#if defined(__ia64) && !defined(finite)
#define isfinite(x) ((sizeof(x) == sizeof(float) ? \
                     _Isfinitef(x) : _IsFinite(x)))
#else
#include <math.h>
#define isfinite finite
#endif
#endif
#else
#include <cmath>
#if !(defined(__QNXNTO__)) // QNX already defines isfinite
#define isfinite std::isfinite
#endif
#endif

#if defined(_MSC_VER)
#if !defined(WINCE) && defined(__STDC_SECURE_LIB__) && _MSC_VER >= 1500 // VC++ 9.0 and above
#define snprintf sprintf_s
#elif _MSC_VER >= 1900 // VC++ 14.0 and above
#define snprintf std::snprintf
#else
#define snprintf _snprintf
#endif
#elif defined(__ANDROID__) || defined(__QNXNTO__)
#define snprintf snprintf
#elif __cplusplus >= 201103L
#define snprintf std::snprintf
#endif

#if defined(__BORLANDC__)  
#include <float.h>
#define isfinite _finite
#define snprintf _snprintf
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400 // VC++ 8.0
// Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif

namespace Json {

bool WriterUtils::containsControlCharacter(const char* str) {
  while (*str) {
    if (isControlCharacter(*(str++)))
      return true;
  }
  return false;
}

bool WriterUtils::containsControlCharacter0(const char* str, unsigned len) {
  char const* end = str + len;
  while (end != str) {
    if (isControlCharacter(*str) || 0==*str)
      return true;
    ++str;
  }
  return false;
}

// https://github.com/upcaste/upcaste/blob/master/src/upcore/src/cstring/strnpbrk.cpp
char const* WriterUtils::strnpbrk(char const* s, char const* accept, size_t n) {
  assert((s || !n) && accept);

  char const* const end = s + n;
  for (char const* cur = s; cur < end; ++cur) {
    int const c = *cur;
    for (char const* a = accept; *a; ++a) {
      if (*a == c) {
        return cur;
      }
    }
  }
  return NULL;
}

std::string WriterUtils::codePointToUTF8(unsigned int cp) {
  return JsonTool::codePointToUTF8(cp);
}
bool WriterUtils::isControlCharacter(char ch) {
  return JsonTool::isControlCharacter(ch);
}
void WriterUtils::uintToString(LargestUInt value, char*& current) {
  JsonTool::uintToString(value, current);
}
void WriterUtils::fixNumericLocale(char* begin, char* end) {
  JsonTool::fixNumericLocale(begin, end);
}

} // namespace Json
