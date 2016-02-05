// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSON_WRITER_DECL_H_INCLUDED
#define JSON_WRITER_DECL_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "value_declaration.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <vector>
#include <string>
#include <ostream>

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

// Disable warning C4251: <data member>: <type> needs to have dll-interface to
// be used by...
#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

namespace Json {

template<typename _Traits, typename _Alloc>
class JSON_API StreamWriter; //Forward declare
template<typename _Traits, typename _Alloc>
class JSON_API StreamWriterBuilder; //Forward declare

template<typename _Traits, typename _Alloc>
class Value;


template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToQuotedStringN(const char* value, unsigned length);

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(LargestInt value);

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(LargestUInt value);

#if defined(JSON_HAS_INT64)

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(Int value);

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(UInt value);
#endif // # if defined(JSON_HAS_INT64)

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(double value, bool useSpecialFloats, unsigned int precision);

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(double value);

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(bool value);

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToQuotedString(const char* value);

/** \brief Write into stringstream, then return string, for convenience.
 * A StreamWriter will be created from the factory, used, and then deleted.
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> JSON_API writeString(typename StreamWriter<_Traits, _Alloc>::Factory const& factory, Value<_Traits, _Alloc> const& root);

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::ostream& operator<<(std::ostream& sout, Value<_Traits, _Alloc> const& root);

/**
 * Internal utilities for converting to output
 */
class WriterUtils {
	public:
		static bool containsControlCharacter(const char* str);
		static bool containsControlCharacter0(const char* str, unsigned len);
		// https://github.com/upcaste/upcaste/blob/master/src/upcore/src/cstring/strnpbrk.cpp
		static char const* strnpbrk(char const* s, char const* accept, size_t n);

		/// Converts a unicode code-point to UTF-8.
		static std::string codePointToUTF8(unsigned int cp);
		/// Returns true if ch is a control character (in range [1,31]).
		static bool isControlCharacter(char ch);
		/** Converts an unsigned integer to string.
		 * @param value Unsigned interger to convert to string
		 * @param current Input/Output string buffer.
		 *        Must have at least uintToStringBufferSize chars free.
		 */
		static void uintToString(LargestUInt value, char*& current);

		/** Change ',' to '.' everywhere in buffer.
		 *
		 * We had a sophisticated way, but it did not work in WinCE.
		 * @see https://github.com/open-source-parsers/jsoncpp/pull/9
		 */
		static void fixNumericLocale(char* begin, char* end);

		// Defines a char buffer for use with uintToString().
		typedef char UIntToStringBuffer[3 * sizeof(LargestUInt) + 1];
};

/**

Usage:
\code
  using namespace Json;
  void writeToStdout(StreamWriter::Factory const& factory, Value const& value) {
    std::unique_ptr<StreamWriter> const writer(
      factory.newStreamWriter());
    writer->write(value, &std::cout);
    std::cout << std::endl;  // add lf and flush
  }
\endcode
*/
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API StreamWriter {
protected:
  std::ostream* sout_;  // not owned; will not delete
public:
  StreamWriter();
  virtual ~StreamWriter();
  /** Write Value into document as configured in sub-class.
      Do not take ownership of sout, but maintain a reference during function.
      \pre sout != NULL
      \return zero on success (For now, we always return zero, so check the stream instead.)
      \throw std::exception possibly, depending on configuration
   */
  virtual int write(Value<_Traits, _Alloc> const& root, std::ostream* sout) = 0;

  /** \brief A simple abstract factory.
   */
  class JSON_API Factory {
  public:
    virtual ~Factory();
    /** \brief Allocate a CharReader via operator new().
     * \throw std::exception if something goes wrong (e.g. invalid settings)
     */
    virtual StreamWriter* newStreamWriter() const = 0;
  };  // Factory
};  // StreamWriter

/** \brief Build a StreamWriter implementation.

Usage:
\code
  using namespace Json;
  Value value = ...;
  StreamWriterBuilder builder;
  builder["commentStyle"] = "None";
  builder["indentation"] = "   ";  // or whatever you like
  std::unique_ptr<Json::StreamWriter> writer(
      builder.newStreamWriter());
  writer->write(value, &std::cout);
  std::cout << std::endl;  // add lf and flush
\endcode
*/
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API StreamWriterBuilder : public StreamWriter<_Traits, _Alloc>::Factory {
public:
  // Note: We use a Json::Value so that we can add data-members to this class
  // without a major version bump.
  /** Configuration of this builder.
    Available settings (case-sensitive):
    - "commentStyle": "None" or "All"
    - "indentation":  "<anything>"
    - "enableYAMLCompatibility": false or true
      - slightly change the whitespace around colons
    - "dropNullPlaceholders": false or true
      - Drop the "null" string from the writer's output for nullValues.
        Strictly speaking, this is not valid JSON. But when the output is being
        fed to a browser's Javascript, it makes for smaller output and the
        browser can handle the output just fine.
    - "useSpecialFloats": false or true
      - If true, outputs non-finite floating point values in the following way:
        NaN values as "NaN", positive infinity as "Infinity", and negative infinity
        as "-Infinity".

    You can examine 'settings_` yourself
    to see the defaults. You can also write and read them just like any
    JSON Value.
    \sa setDefaults()
    */
  Json::Value<_Traits, _Alloc> settings_;

  StreamWriterBuilder();
  ~StreamWriterBuilder() override;

  /**
   * \throw std::exception if something goes wrong (e.g. invalid settings)
   */
  StreamWriter<_Traits, _Alloc>* newStreamWriter() const override;

  /** \return true if 'settings' are legal and consistent;
   *   otherwise, indicate bad settings via 'invalid'.
   */
  bool validate(Json::Value<_Traits, _Alloc>* invalid) const;
  /** A simple way to update a specific setting.
   */
  Value<_Traits, _Alloc>& operator[](std::basic_string<char, _Traits, _Alloc> key);

  /** Called by ctor, but you can use this to reset settings_.
   * \pre 'settings' != NULL (but Json::null is fine)
   * \remark Defaults:
   * \snippet src/lib_json/json_writer.cpp StreamWriterBuilderDefaults
   */
  static void setDefaults(Value<_Traits, _Alloc>* settings);
  static void getValidWriterKeys(std::set<std::basic_string<char, _Traits, _Alloc>>* valid_keys);
};

/** \brief Abstract class for writers.
 * \deprecated Use StreamWriter. (And really, this is an implementation detail.)
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API Writer {
public:
  virtual ~Writer();

  virtual std::basic_string<char, _Traits, _Alloc> write(const Value<_Traits, _Alloc>& root) = 0;
};

/** \brief Outputs a Value in <a HREF="http://www.json.org">JSON</a> format
 *without formatting (not human friendly).
 *
 * The JSON document is written in a single line. It is not intended for 'human'
 *consumption,
 * but may be usefull to support feature such as RPC where bandwith is limited.
 * \sa Reader, Value
 * \deprecated Use StreamWriterBuilder.
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API FastWriter : public Writer<_Traits, _Alloc> {

public:
  FastWriter();
  ~FastWriter() override {}

  void enableYAMLCompatibility();

  /** \brief Drop the "null" string from the writer's output for nullValues.
   * Strictly speaking, this is not valid JSON. But when the output is being
   * fed to a browser's Javascript, it makes for smaller output and the
   * browser can handle the output just fine.
   */
  void dropNullPlaceholders();

  void omitEndingLineFeed();

public: // overridden from Writer
  std::basic_string<char, _Traits, _Alloc> write(const Value<_Traits, _Alloc>& root) override;

private:
  void writeValue(const Value<_Traits, _Alloc>& value);

  std::basic_string<char, _Traits, _Alloc> document_;
  bool yamlCompatiblityEnabled_;
  bool dropNullPlaceholders_;
  bool omitEndingLineFeed_;
};

/** \brief Writes a Value in <a HREF="http://www.json.org">JSON</a> format in a
 *human friendly way.
 *
 * The rules for line break and indent are as follow:
 * - Object value:
 *     - if empty then print {} without indent and line break
 *     - if not empty the print '{', line break & indent, print one value per
 *line
 *       and then unindent and line break and print '}'.
 * - Array value:
 *     - if empty then print [] without indent and line break
 *     - if the array contains no object value, empty array or some other value
 *types,
 *       and all the values fit on one lines, then print the array on a single
 *line.
 *     - otherwise, it the values do not fit on one line, or the array contains
 *       object or non empty array, then print one value per line.
 *
 * If the Value have comments then they are outputed according to their
 *#CommentPlacement.
 *
 * \sa Reader, Value, Value::setComment()
 * \deprecated Use StreamWriterBuilder.
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API StyledWriter : public Writer<_Traits, _Alloc> {
public:
  StyledWriter();
  ~StyledWriter() override {}

public: // overridden from Writer
  /** \brief Serialize a Value in <a HREF="http://www.json.org">JSON</a> format.
   * \param root Value to serialize.
   * \return String containing the JSON document that represents the root value.
   */
  std::basic_string<char, _Traits, _Alloc> write(const Value<_Traits, _Alloc>& root) override;

private:
  void writeValue(const Value<_Traits, _Alloc>& value);
  void writeArrayValue(const Value<_Traits, _Alloc>& value);
  bool isMultineArray(const Value<_Traits, _Alloc>& value);
  void pushValue(const std::basic_string<char, _Traits, _Alloc>& value);
  void writeIndent();
  void writeWithIndent(const std::basic_string<char, _Traits, _Alloc>& value);
  void indent();
  void unindent();
  void writeCommentBeforeValue(const Value<_Traits, _Alloc>& root);
  void writeCommentAfterValueOnSameLine(const Value<_Traits, _Alloc>& root);
  bool hasCommentForValue(const Value<_Traits, _Alloc>& value);
  static std::basic_string<char, _Traits, _Alloc> normalizeEOL(const std::basic_string<char, _Traits, _Alloc>& text);

  typedef std::vector<std::basic_string<char, _Traits, _Alloc>> ChildValues;

  ChildValues childValues_;
  std::basic_string<char, _Traits, _Alloc> document_;
  std::basic_string<char, _Traits, _Alloc> indentString_;
  int rightMargin_;
  int indentSize_;
  bool addChildValues_;
};

/** \brief Writes a Value in <a HREF="http://www.json.org">JSON</a> format in a
 human friendly way,
     to a stream rather than to a string.
 *
 * The rules for line break and indent are as follow:
 * - Object value:
 *     - if empty then print {} without indent and line break
 *     - if not empty the print '{', line break & indent, print one value per
 line
 *       and then unindent and line break and print '}'.
 * - Array value:
 *     - if empty then print [] without indent and line break
 *     - if the array contains no object value, empty array or some other value
 types,
 *       and all the values fit on one lines, then print the array on a single
 line.
 *     - otherwise, it the values do not fit on one line, or the array contains
 *       object or non empty array, then print one value per line.
 *
 * If the Value have comments then they are outputed according to their
 #CommentPlacement.
 *
 * \param indentation Each level will be indented by this amount extra.
 * \sa Reader, Value, Value::setComment()
 * \deprecated Use StreamWriterBuilder.
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API StyledStreamWriter {
public:
  StyledStreamWriter(std::basic_string<char, _Traits, _Alloc> indentation = "\t");
  ~StyledStreamWriter() {}

public:
  /** \brief Serialize a Value in <a HREF="http://www.json.org">JSON</a> format.
   * \param out Stream to write to. (Can be ostringstream, e.g.)
   * \param root Value to serialize.
   * \note There is no point in deriving from Writer, since write() should not
   * return a value.
   */
  void write(std::ostream& out, const Value<_Traits, _Alloc>& root);

private:
  void writeValue(const Value<_Traits, _Alloc>& value);
  void writeArrayValue(const Value<_Traits, _Alloc>& value);
  bool isMultineArray(const Value<_Traits, _Alloc>& value);
  void pushValue(const std::basic_string<char, _Traits, _Alloc>& value);
  void writeIndent();
  void writeWithIndent(const std::basic_string<char, _Traits, _Alloc>& value);
  void indent();
  void unindent();
  void writeCommentBeforeValue(const Value<_Traits, _Alloc>& root);
  void writeCommentAfterValueOnSameLine(const Value<_Traits, _Alloc>& root);
  bool hasCommentForValue(const Value<_Traits, _Alloc>& value);
  static std::basic_string<char, _Traits, _Alloc> normalizeEOL(const std::basic_string<char, _Traits, _Alloc>& text);

  typedef std::vector<std::basic_string<char, _Traits, _Alloc>> ChildValues;

  ChildValues childValues_;
  std::ostream* document_;
  std::basic_string<char, _Traits, _Alloc> indentString_;
  int rightMargin_;
  std::basic_string<char, _Traits, _Alloc> indentation_;
  bool addChildValues_ : 1;
  bool indented_ : 1;
};

} // namespace Json

#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(pop)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

#endif // JSON_WRITER_DECL_H_INCLUDED
