// Copyright 2007-2011 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/assertions.h>
#include <json/reader.h>
#include <json/value.h>
#include <json/tool.h>
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <utility>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <istream>
#include <sstream>
#include <memory>
#include <set>
#include <limits>

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

#if defined(__QNXNTO__)
#define sscanf std::sscanf
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400 // VC++ 8.0
// Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif

static int const stackLimit_g = 1000;
static int       stackDepth_g = 0;  // see readValue()

namespace Json {

#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
typedef std::unique_ptr<CharReader> CharReaderPtr;
#else
typedef std::auto_ptr<CharReader>   CharReaderPtr;
#endif

// Implementation of class Features
// ////////////////////////////////

Features::Features()
    : allowComments_(true), strictRoot_(false),
      allowDroppedNullPlaceholders_(false), allowNumericKeys_(false) {}

Features Features::all() { return Features(); }

Features Features::strictMode() {
  Features features;
  features.allowComments_ = false;
  features.strictRoot_ = true;
  features.allowDroppedNullPlaceholders_ = false;
  features.allowNumericKeys_ = false;
  return features;
}

// exact copy of Implementation of class Features
// ////////////////////////////////

OurFeatures OurFeatures::all() { return OurFeatures(); }

} // namespace Json
