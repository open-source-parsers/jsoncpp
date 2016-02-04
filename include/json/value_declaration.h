// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef CPPTL_JSON_DECL_H_INCLUDED
#define CPPTL_JSON_DECL_H_INCLUDED
#if !defined(JSON_IS_AMALGAMATION)
#include "forwards.h"
#include <json/assertions.h>
#include <json/writer_declaration.h>
#endif // if !defined(JSON_IS_AMALGAMATION)

#include <math.h>
#include <sstream>
#include <utility>
#include <cstring>
#include <string>
#include <cassert>
#include <cstddef> // size_t
#include <algorithm> // min()

#include <cassert>
#include <memory>
#include <vector>
#include <exception>

#ifndef JSON_USE_CPPTL_SMALLMAP
#include <map>
#else
#include <cpptl/smallmap.h>
#endif
#ifdef JSON_USE_CPPTL
#include <cpptl/forwards.h>
#include <cpptl/conststring.h>
#endif

// Disable warning C4251: <data member>: <type> needs to have dll-interface to
// be used by...
#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

#define JSON_ASSERT_UNREACHABLE assert(false)

/** \brief JSON (JavaScript Object Notation).
 */
namespace Json {

/** \brief Type of the value held by a Value object.
 */
enum ValueType {
  nullValue = 0, ///< 'null' value
  intValue,      ///< signed integer value
  uintValue,     ///< unsigned integer value
  realValue,     ///< double value
  stringValue,   ///< UTF-8 string value
  booleanValue,  ///< bool value
  arrayValue,    ///< array value (ordered list)
  objectValue    ///< object value (collection of name/value pairs).
};

enum CommentPlacement {
  commentBefore = 0,      ///< a comment placed on the line before a value
  commentAfterOnSameLine, ///< a comment just after a value on the same line
  commentAfter, ///< a comment on the line after a value (only make sense for
  /// root value)
  numberOfCommentPlacement
};

/// used internally
void throwRuntimeError(std::string const& msg);
/// used internally
void throwLogicError(std::string const& msg);

/** Base class for all exceptions we throw.
 *
 * We use nothing but these internally. Of course, STL can throw others.
 */
class JSON_API Exception : public std::exception {
public:
  Exception(std::string const& msg);
  ~Exception() throw() override;
  char const* what() const throw() override;
protected:
  std::string msg_;
};

/** Exceptions which the user cannot easily avoid.
 *
 * E.g. out-of-memory (when we use malloc), stack-overflow, malicious input
 * 
 * \remark derived from Json::Exception
 */
class JSON_API RuntimeError : public Exception {
public:
  RuntimeError(std::string const& msg);
};

/** Exceptions thrown by JSON_ASSERT/JSON_FAIL macros.
 *
 * These are precondition-violations (user bugs) and internal errors (our bugs).
 * 
 * \remark derived from Json::Exception
 */
class JSON_API LogicError : public Exception {
public:
  LogicError(std::string const& msg);
};

//# ifdef JSON_USE_CPPTL
//   typedef CppTL::AnyEnumerator<const char *> EnumMemberNames;
//   typedef CppTL::AnyEnumerator<const Value &> EnumValues;
//# endif

/** \brief Lightweight wrapper to tag static string.
 *
 * Value constructor and objectValue member assignement takes advantage of the
 * StaticString and avoid the cost of string duplication when storing the
 * string or the member name.
 *
 * Example of usage:
 * \code
 * Json::Value aValue( StaticString("some text") );
 * Json::Value object;
 * static const StaticString code("code");
 * object[code] = 1234;
 * \endcode
 */
class JSON_API StaticString {
public:
  explicit StaticString(const char* czstring) : c_str_(czstring) {}

  operator const char*() const { return c_str_; }

  const char* c_str() const { return c_str_; }

private:
  const char* c_str_;
};

template<typename _Traits, typename _Alloc>
class ValueHolder; //Forward declaration

/** \brief Represents a <a HREF="http://www.json.org">JSON</a> value.
 *
 * This class is a discriminated union wrapper that can represents a:
 * - signed integer [range: Value::minInt - Value::maxInt]
 * - unsigned integer (range: 0 - Value::maxUInt)
 * - double
 * - UTF-8 string
 * - boolean
 * - 'null'
 * - an ordered list of Value
 * - collection of name/value pairs (javascript object)
 *
 * The type of the held value is represented by a #ValueType and
 * can be obtained using type().
 *
 * Values of an #objectValue or #arrayValue can be accessed using operator[]()
 * methods.
 * Non-const methods will automatically create the a #nullValue element
 * if it does not exist.
 * The sequence of an #arrayValue will be automatically resized and initialized
 * with #nullValue. resize() can be used to enlarge or truncate an #arrayValue.
 *
 * The get() methods can be used to obtain default value in the case the
 * required element does not exist.
 *
 * It is possible to iterate over the list of a #objectValue values using
 * the getMemberNames() method.
 *
 * \note #Value string-length fit in size_t, but keys must be < 2^30.
 * (The reason is an implementation detail.) A #CharReader will raise an
 * exception if a bound is exceeded to avoid security holes in your app,
 * but the Value API does *not* check bounds. That is the responsibility
 * of the caller.
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API Value {
  template<typename T, typename A>
  friend class ValueIteratorBase;
public:
  typedef _Traits traits_type;
  typedef typename _Traits::char_type value_type;
  typedef _Alloc allocator_type;
  typedef std::basic_string<value_type, traits_type, allocator_type> string_type;
  typedef std::vector<value_type, allocator_type> string_data_type_in_ptr;
  typedef std::unique_ptr<string_data_type_in_ptr> string_data_type;
  typedef Value<traits_type, allocator_type> this_type;

  typedef std::vector<string_type> Members;
  typedef ValueIterator<_Traits, _Alloc> iterator;
  typedef ValueConstIterator<_Traits, _Alloc> const_iterator;
  typedef Json::UInt UInt;
  typedef Json::Int Int;
#if defined(JSON_HAS_INT64)
  typedef Json::UInt64 UInt64;
  typedef Json::Int64 Int64;
#endif // defined(JSON_HAS_INT64)
  typedef Json::LargestInt LargestInt;
  typedef Json::LargestUInt LargestUInt;
  typedef Json::ArrayIndex ArrayIndex;

  static const this_type& null;  ///< We regret this reference to a global instance; prefer the simpler Value().
  static const this_type& nullRef;  ///< just a kludge for binary-compatibility; same as null
  /// Minimum signed integer value that can be stored in a Json::Value.
  static const LargestInt minLargestInt;
  /// Maximum signed integer value that can be stored in a Json::Value.
  static const LargestInt maxLargestInt;
  /// Maximum unsigned integer value that can be stored in a Json::Value.
  static const LargestUInt maxLargestUInt;

  /// Minimum signed int value that can be stored in a Json::Value.
  static const Int minInt;
  /// Maximum signed int value that can be stored in a Json::Value.
  static const Int maxInt;
  /// Maximum unsigned int value that can be stored in a Json::Value.
  static const UInt maxUInt;

#if defined(JSON_HAS_INT64)
  /// Minimum signed 64 bits int value that can be stored in a Json::Value.
  static const Int64 minInt64;
  /// Maximum signed 64 bits int value that can be stored in a Json::Value.
  static const Int64 maxInt64;
  /// Maximum unsigned 64 bits int value that can be stored in a Json::Value.
  static const UInt64 maxUInt64;
#endif // defined(JSON_HAS_INT64)
  static const double maxUInt64AsDouble;

private:
#ifndef JSONCPP_DOC_EXCLUDE_IMPLEMENTATION
  class CZString {
  public:
    enum DuplicationPolicy {
      noDuplication = 0,
      duplicate,
      duplicateOnCopy
    };
    CZString(ArrayIndex index);
    CZString(char const* str, unsigned length, DuplicationPolicy allocate);
    CZString(CZString const& other);
#if JSON_HAS_RVALUE_REFERENCES
    CZString(CZString&& other);
#endif
    ~CZString();
    CZString& operator=(CZString other);
    bool operator<(CZString const& other) const;
    bool operator==(CZString const& other) const;
    ArrayIndex index() const;
    //const char* c_str() const; ///< \deprecated
    char const* data() const;
    unsigned length() const;
    bool isStaticString() const;

  private:
    void swap(CZString& other);

    struct StringStorage {
      unsigned policy_: 2;
      unsigned length_: 30; // 1GB max
    };

    char const* cstrNoDup_;  // actually, a prefixed string, unless policy is noDup
    string_data_type cstr_;  // actually, a prefixed string, unless policy is noDup
    union {
      ArrayIndex index_;
      StringStorage storage_;
    };
  };

public:
#ifndef JSON_USE_CPPTL_SMALLMAP
  typedef std::map<CZString, Value<_Traits, _Alloc>> ObjectValues;
#else
  typedef CppTL::SmallMap<CZString, this_type> ObjectValues;
#endif // ifndef JSON_USE_CPPTL_SMALLMAP
#endif // ifndef JSONCPP_DOC_EXCLUDE_IMPLEMENTATION

public:
  /** \brief Create a default Value of the given type.

    This is a very useful constructor.
    To create an empty array, pass arrayValue.
    To create an empty object, pass objectValue.
    Another Value can then be set to this one by assignment.
This is useful since clear() and resize() will not alter types.

    Examples:
\code
Json::Value null_value; // null
Json::Value arr_value(Json::arrayValue); // []
Json::Value obj_value(Json::objectValue); // {}
\endcode
  */
  Value(ValueType type = nullValue);
  Value(Int value);
  Value(UInt value);
#if defined(JSON_HAS_INT64)
  Value(Int64 value);
  Value(UInt64 value);
#endif // if defined(JSON_HAS_INT64)
  Value(double value);
  Value(const char* value); ///< Copy til first 0. (NULL causes to seg-fault.)
  Value(const char* begin, const char* end); ///< Copy all, incl zeroes.
  /** \brief Constructs a value from a static string.

   * Like other value string constructor but do not duplicate the string for
   * internal storage. The given string must remain alive after the call to this
   * constructor.
   * \note This works only for null-terminated strings. (We cannot change the
   *   size of this class, so we have nowhere to store the length,
   *   which might be computed later for various operations.)
   *
   * Example of usage:
   * \code
   * static StaticString foo("some text");
   * Json::Value aValue(foo);
   * \endcode
   */
  Value(const StaticString& value);
  Value(const string_type& value); ///< Copy data() til size(). Embedded zeroes too.
#ifdef JSON_USE_CPPTL
  Value(const CppTL::ConstString& value);
#endif
  Value(bool value);
  /// Deep copy.
  Value(const this_type& other);
#if JSON_HAS_RVALUE_REFERENCES
  /// Move constructor
  Value(this_type&& other);
#endif
  ~Value();

  /// Deep copy, then swap(other).
  /// \note Over-write existing comments. To preserve comments, use #swapPayload().
  Value& operator=(this_type other);
  /// Swap everything.
  void swap(this_type& other);
  /// Swap values but leave comments and source offsets in place.
  void swapPayload(this_type& other);

  ValueType type() const;

  /// Compare payload only, not comments etc.
  bool operator<(const this_type& other) const;
  bool operator<=(const this_type& other) const;
  bool operator>=(const this_type& other) const;
  bool operator>(const this_type& other) const;
  bool operator==(const this_type& other) const;
  bool operator!=(const this_type& other) const;
  int compare(const this_type& other) const;

  const char* asCString() const; ///< Embedded zeroes could cause you trouble!
  string_type asString() const; ///< Embedded zeroes are possible.
  /** Get raw char* of string-value.
   *  \return false if !string. (Seg-fault if str or end are NULL.)
   */
  bool getString(
      char const** begin, char const** end) const;
#ifdef JSON_USE_CPPTL
  CppTL::ConstString asConstString() const;
#endif
  Int asInt() const;
  UInt asUInt() const;
#if defined(JSON_HAS_INT64)
  Int64 asInt64() const;
  UInt64 asUInt64() const;
#endif // if defined(JSON_HAS_INT64)
  LargestInt asLargestInt() const;
  LargestUInt asLargestUInt() const;
  float asFloat() const;
  double asDouble() const;
  bool asBool() const;

  bool isNull() const;
  bool isBool() const;
  bool isInt() const;
  bool isInt64() const;
  bool isUInt() const;
  bool isUInt64() const;
  bool isIntegral() const;
  bool isDouble() const;
  bool isNumeric() const;
  bool isString() const;
  bool isArray() const;
  bool isObject() const;

  bool isConvertibleTo(ValueType other) const;

  /// Number of values in array or object
  ArrayIndex size() const;

  /// \brief Return true if empty array, empty object, or null;
  /// otherwise, false.
  bool empty() const;

  /// Return isNull()
  bool operator!() const;

  /// Remove all object members and array elements.
  /// \pre type() is arrayValue, objectValue, or nullValue
  /// \post type() is unchanged
  void clear();

  /// Resize the array to size elements.
  /// New elements are initialized to null.
  /// May only be called on nullValue or arrayValue.
  /// \pre type() is arrayValue or nullValue
  /// \post type() is arrayValue
  void resize(ArrayIndex size);

  /// Access an array element (zero based index ).
  /// If the array contains less than index element, then null value are
  /// inserted
  /// in the array so that its size is index+1.
  /// (You may need to say 'value[0u]' to get your compiler to distinguish
  ///  this from the operator[] which takes a string.)
  this_type& operator[](ArrayIndex index);

  /// Access an array element (zero based index ).
  /// If the array contains less than index element, then null value are
  /// inserted
  /// in the array so that its size is index+1.
  /// (You may need to say 'value[0u]' to get your compiler to distinguish
  ///  this from the operator[] which takes a string.)
  this_type& operator[](int index);

  /// Access an array element (zero based index )
  /// (You may need to say 'value[0u]' to get your compiler to distinguish
  ///  this from the operator[] which takes a string.)
  const this_type& operator[](ArrayIndex index) const;

  /// Access an array element (zero based index )
  /// (You may need to say 'value[0u]' to get your compiler to distinguish
  ///  this from the operator[] which takes a string.)
  const this_type& operator[](int index) const;

  /// If the array contains at least index+1 elements, returns the element
  /// value,
  /// otherwise returns defaultValue.
  this_type get(ArrayIndex index, const this_type& defaultValue) const;
  /// Return true if index < size().
  bool isValidIndex(ArrayIndex index) const;
  /// \brief Append value to array at the end.
  ///
  /// Equivalent to jsonvalue[jsonvalue.size()] = value;
  this_type& append(const this_type& value);

  /// Access an object value by name, create a null member if it does not exist.
  /// \note Because of our implementation, keys are limited to 2^30 -1 chars.
  ///  Exceeding that will cause an exception.
  this_type& operator[](const char* key);
  /// Access an object value by name, returns null if there is no member with
  /// that name.
  const this_type& operator[](const char* key) const;
  /// Access an object value by name, create a null member if it does not exist.
  /// \param key may contain embedded nulls.
  this_type& operator[](const string_type& key);
  /// Access an object value by name, returns null if there is no member with
  /// that name.
  /// \param key may contain embedded nulls.
  const this_type& operator[](const string_type& key) const;
  /** \brief Access an object value by name, create a null member if it does not
   exist.

   * If the object has no entry for that name, then the member name used to store
   * the new entry is not duplicated.
   * Example of use:
   * \code
   * Json::Value object;
   * static const StaticString code("code");
   * object[code] = 1234;
   * \endcode
   */
  this_type& operator[](const StaticString& key);
#ifdef JSON_USE_CPPTL
  /// Access an object value by name, create a null member if it does not exist.
  this_type& operator[](const CppTL::ConstString& key);
  /// Access an object value by name, returns null if there is no member with
  /// that name.
  const this_type& operator[](const CppTL::ConstString& key) const;
#endif
  /// Return the member named key if it exist, defaultValue otherwise.
  /// \note deep copy
  this_type get(const char* key, const this_type& defaultValue) const;
  /// Return the member named key if it exist, defaultValue otherwise.
  /// \note deep copy
  /// \note key may contain embedded nulls.
  this_type get(const char* begin, const char* end, const this_type& defaultValue) const;
  /// Return the member named key if it exist, defaultValue otherwise.
  /// \note deep copy
  /// \param key may contain embedded nulls.
  this_type get(const string_type& key, const this_type& defaultValue) const;
#ifdef JSON_USE_CPPTL
  /// Return the member named key if it exist, defaultValue otherwise.
  /// \note deep copy
  this_type get(const CppTL::ConstString& key, const this_type& defaultValue) const;
#endif
  /// Most general and efficient version of isMember()const, get()const,
  /// and operator[]const
  /// \note As stated elsewhere, behavior is undefined if (end-begin) >= 2^30
  this_type const* find(char const* begin, char const* end) const;
  /// Most general and efficient version of object-mutators.
  /// \note As stated elsewhere, behavior is undefined if (end-begin) >= 2^30
  /// \return non-zero, but JSON_ASSERT if this is neither object nor nullValue.
  this_type const* demand(char const* begin, char const* end);
  /// \brief Remove and return the named member.
  ///
  /// Do nothing if it did not exist.
  /// \return the removed Value, or null.
  /// \pre type() is objectValue or nullValue
  /// \post type() is unchanged
  /// \deprecated
  this_type removeMember(const char* key);
  /// Same as removeMember(const char*)
  /// \param key may contain embedded nulls.
  /// \deprecated
  this_type removeMember(const string_type& key);
  /// Same as removeMember(const char* begin, const char* end, Value* removed),
  /// but 'key' is null-terminated.
  bool removeMember(const char* key, this_type* removed);
  /** \brief Remove the named map member.

      Update 'removed' iff removed.
      \param key may contain embedded nulls.
      \return true iff removed (no exceptions)
  */
  bool removeMember(string_type const& key, this_type* removed);
  /// Same as removeMember(string_type const& key, this_type* removed)
  bool removeMember(const char* begin, const char* end, this_type* removed);
  /** \brief Remove the indexed array element.

      O(n) expensive operations.
      Update 'removed' iff removed.
      \return true iff removed (no exceptions)
  */
  bool removeIndex(ArrayIndex i, this_type* removed);

  /// Return true if the object has a member named key.
  /// \note 'key' must be null-terminated.
  bool isMember(const char* key) const;
  /// Return true if the object has a member named key.
  /// \param key may contain embedded nulls.
  bool isMember(const string_type& key) const;
  /// Same as isMember(string_type const& key)const
  bool isMember(const char* begin, const char* end) const;
#ifdef JSON_USE_CPPTL
  /// Return true if the object has a member named key.
  bool isMember(const CppTL::ConstString& key) const;
#endif

  /// \brief Return a list of the member names.
  ///
  /// If null, return an empty list.
  /// \pre type() is objectValue or nullValue
  /// \post if type() was nullValue, it remains nullValue
  Members getMemberNames() const;

  //# ifdef JSON_USE_CPPTL
  //      EnumMemberNames enumMemberNames() const;
  //      EnumValues enumValues() const;
  //# endif

  /// \deprecated Always pass len.
  JSONCPP_DEPRECATED("Use setComment(string_type const&) instead.")
  void setComment(const char* comment, CommentPlacement placement);
  /// Comments must be //... or /* ... */
  void setComment(const char* comment, size_t len, CommentPlacement placement);
  /// Comments must be //... or /* ... */
  void setComment(const string_type& comment, CommentPlacement placement);
  bool hasComment(CommentPlacement placement) const;
  /// Include delimiters and embedded newlines.
  string_type getComment(CommentPlacement placement) const;

  string_type toStyledString() const;

  const_iterator begin() const;
  const_iterator end() const;

  iterator begin();
  iterator end();

  // Accessors for the [start, limit) range of bytes within the JSON text from
  // which this value was parsed, if any.
  void setOffsetStart(size_t start);
  void setOffsetLimit(size_t limit);
  size_t getOffsetStart() const;
  size_t getOffsetLimit() const;

private:
  void initBasic(ValueType type, bool allocated = false);
  /** Duplicates the specified string value.
   * @param value Pointer to the string to duplicate. Must be zero-terminated if
   *              length is "unknown".
   * @param length Length of the value. if equals to unknown, then it will be
   *               computed using strlen(value).
   * @return Pointer on the duplicate instance of string.
   */
  static string_data_type duplicateStringValue(const char* value, size_t length);
  /** Record the length as a prefix.
   */
  static string_data_type duplicateAndPrefixStringValue(const char* value, unsigned int length);
  static void decodePrefixedString(bool isPrefixed, const char* prefixed, unsigned* length, char const** value);
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
  template <typename T, typename U>
  static inline bool InRange(double d, T min, U max) {
    return d >= min && d <= max;
  }
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
  static inline double integerToDouble(Json::UInt64 value) {
    return static_cast<double>(Int64(value / 2)) * 2.0 + Int64(value & 1);
  }

  template <typename T> static inline double integerToDouble(T value) {
    return static_cast<double>(value);
  }

  template <typename T, typename U>
  static inline bool InRange(double d, T min, U max) {
    return d >= integerToDouble(min) && d <= integerToDouble(max);
  }
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
  this_type& resolveReference(const char* key);
  this_type& resolveReference(const char* key, const char* end);

  struct CommentInfo {
    CommentInfo();
    ~CommentInfo();

    void setComment(const char* text, size_t len);
    void setComment(const string_data_type& text, size_t len);

    string_data_type comment_;
  };

  // struct MemberNamesTransform
  //{
  //   typedef const char *result_type;
  //   const char *operator()( const CZString &name ) const
  //   {
  //      return name.c_str();
  //   }
  //};

  ValueHolder<_Traits, _Alloc> value_;
  ValueType type_ : 8;
  unsigned int allocated_ : 1; // Notes: if declared as bool, bitfield is useless.
                               // If not allocated_, string_ must be null-terminated.
  CommentInfo* comments_;

  // [start, limit) byte offsets in the source JSON text from which this Value
  // was extracted.
  size_t start_;
  size_t limit_;

  // This is a walkaround to avoid the static initialization of Value::null.
  // kNull must be word-aligned to avoid crashing on ARM.  We use an alignment of
  // 8 (instead of 4) as a bit of future-proofing.
#if defined(__ARMEL__)
#define ALIGNAS(byte_alignment) __attribute__((aligned(byte_alignment)))
#else
#define ALIGNAS(byte_alignment)
#endif
  static const unsigned char ALIGNAS(8) kNull[2048];
  const unsigned char& kNullRef = kNull[0];
};

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class ValueHolder {
public:
  LargestInt int_ = 0;
  LargestUInt uint_ = 0u;
  double real_ = 0;
  bool bool_ = false;
  typename Value<_Traits, _Alloc>::string_data_type stringDuplicate_;  // actually ptr to unsigned, followed by str (allocated)
  const char* stringRaw_ = nullptr;             // the raw string (!allocated)
  typename Value<_Traits, _Alloc>::ObjectValues* map_ = nullptr;

  ValueHolder();
  ValueHolder(const ValueHolder& ref);
  ValueHolder(ValueHolder&& ref);
  ~ValueHolder();
  ValueHolder& operator=(const ValueHolder& value);

  ValueHolder& operator=(ValueHolder&& value);

  void copy(const ValueHolder& value);
  void swap(ValueHolder&& value);
};

/** \brief Experimental and untested: represents an element of the "path" to
 * access a node.
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API PathArgument {
public:
  template<typename T, typename A>
  friend class Path;

  PathArgument();
  PathArgument(ArrayIndex index);
  PathArgument(const char* key);
  PathArgument(const std::string& key);

private:
  enum Kind {
    kindNone = 0,
    kindIndex,
    kindKey
  };
  std::string key_;
  ArrayIndex index_;
  Kind kind_;
};

/** \brief Experimental and untested: represents a "path" to access a node.
 *
 * Syntax:
 * - "." => root node
 * - ".[n]" => elements at index 'n' of root node (an array value)
 * - ".name" => member named 'name' of root node (an object value)
 * - ".name1.name2.name3"
 * - ".[0][1][2].name1[3]"
 * - ".%" => member name is provided as parameter
 * - ".[%]" => index is provied as parameter
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API Path {
public:
  Path(const std::string& path,
       const PathArgument<_Traits, _Alloc>& a1 = PathArgument<_Traits, _Alloc>(),
       const PathArgument<_Traits, _Alloc>& a2 = PathArgument<_Traits, _Alloc>(),
       const PathArgument<_Traits, _Alloc>& a3 = PathArgument<_Traits, _Alloc>(),
       const PathArgument<_Traits, _Alloc>& a4 = PathArgument<_Traits, _Alloc>(),
       const PathArgument<_Traits, _Alloc>& a5 = PathArgument<_Traits, _Alloc>());

  const Value<_Traits, _Alloc>& resolve(const Value<_Traits, _Alloc>& root) const;
  Value<_Traits, _Alloc> resolve(const Value<_Traits, _Alloc>& root, const Value<_Traits, _Alloc>& defaultValue) const;
  /// Creates the "path" to access the specified node and returns a reference on
  /// the node.
  Value<_Traits, _Alloc>& make(Value<_Traits, _Alloc>& root) const;

private:
  using InArgs = std::vector<const PathArgument<_Traits, _Alloc>*>;
  using Args = std::vector<PathArgument<_Traits, _Alloc>>;

  void makePath(const std::string& path, const InArgs& in);
  void addPathInArg(const std::string& path,
                    const InArgs& in,
					typename InArgs::const_iterator& itInArg,
					typename PathArgument<_Traits, _Alloc>::Kind kind);
  void invalidPath(const std::string& path, int location);

  Args args_;
};

/** \brief base class for Value iterators.
 *
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API ValueIteratorBase {
public:
  typedef std::bidirectional_iterator_tag iterator_category;
  typedef unsigned int size_t;
  typedef int difference_type;
  typedef ValueIteratorBase SelfType;

  bool operator==(const SelfType& other) const { return isEqual(other); }

  bool operator!=(const SelfType& other) const { return !isEqual(other); }

  difference_type operator-(const SelfType& other) const {
    return other.computeDistance(*this);
  }

  /// Return either the index or the member name of the referenced value as a
  /// Value.
  Value<_Traits, _Alloc> key() const;

  /// Return the index of the referenced Value, or -1 if it is not an arrayValue.
  UInt index() const;

  /// Return the member name of the referenced Value, or "" if it is not an
  /// objectValue.
  /// \note Avoid `c_str()` on result, as embedded zeroes are possible.
  std::string name() const;

  /// Return the member name of the referenced Value. "" if it is not an
  /// objectValue.
  /// \deprecated This cannot be used for UTF-8 strings, since there can be embedded nulls.
  JSONCPP_DEPRECATED("Use `key = name();` instead.")
  char const* memberName() const;
  /// Return the member name of the referenced Value, or NULL if it is not an
  /// objectValue.
  /// \note Better version than memberName(). Allows embedded nulls.
  char const* memberName(char const** end) const;

protected:
  Value<_Traits, _Alloc>& deref() const;

  void increment();

  void decrement();

  difference_type computeDistance(const SelfType& other) const;

  bool isEqual(const SelfType& other) const;

  void copy(const SelfType& other);

private:
  typename Value<_Traits, _Alloc>::ObjectValues::iterator current_;
  // Indicates that iterator is for a null value.
  bool isNull_;

public:
  // For some reason, BORLAND needs these at the end, rather
  // than earlier. No idea why.
  ValueIteratorBase();
  explicit ValueIteratorBase(const typename Value<_Traits, _Alloc>::ObjectValues::iterator& current);
};

/** \brief const iterator for object and array value.
 *
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API ValueConstIterator : public ValueIteratorBase<_Traits, _Alloc> {
  friend class Value<_Traits, _Alloc>;

public:
  typedef const Value<_Traits, _Alloc> value_type;
  //typedef unsigned int size_t;
  //typedef int difference_type;
  typedef const Value<_Traits, _Alloc>& reference;
  typedef const Value<_Traits, _Alloc>* pointer;
  typedef ValueConstIterator SelfType;

  ValueConstIterator();
  ValueConstIterator(ValueIterator<_Traits, _Alloc> const& other);

private:
/*! \internal Use by Value to create an iterator.
 */
  explicit ValueConstIterator(const typename Value<_Traits, _Alloc>::ObjectValues::iterator& current);
public:
  SelfType& operator=(const ValueIteratorBase<_Traits, _Alloc>& other);

  SelfType operator++(int) {
    SelfType temp(*this);
    ++*this;
    return temp;
  }

  SelfType operator--(int) {
    SelfType temp(*this);
    --*this;
    return temp;
  }

  SelfType& operator--() {
    ValueIteratorBase<_Traits, _Alloc>::decrement();
    return *this;
  }

  SelfType& operator++() {
	ValueIteratorBase<_Traits, _Alloc>::increment();
    return *this;
  }

  reference operator*() const { return ValueIteratorBase<_Traits, _Alloc>::deref(); }

  pointer operator->() const { return &ValueIteratorBase<_Traits, _Alloc>::deref(); }
};

/** \brief Iterator for object and array value.
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API ValueIterator : public ValueIteratorBase<_Traits, _Alloc> {
  friend class Value<_Traits, _Alloc>;

public:
  typedef Value<_Traits, _Alloc> value_type;
  typedef unsigned int size_t;
  typedef int difference_type;
  typedef Value<_Traits, _Alloc>& reference;
  typedef Value<_Traits, _Alloc>* pointer;
  typedef ValueIterator<_Traits, _Alloc> SelfType;

  ValueIterator();
  explicit ValueIterator(const ValueConstIterator<_Traits, _Alloc>& other);
  ValueIterator(const ValueIterator<_Traits, _Alloc>& other);

private:
/*! \internal Use by Value to create an iterator.
 */
  explicit ValueIterator(const typename Value<_Traits, _Alloc>::ObjectValues::iterator& current);
public:
  SelfType& operator=(const SelfType& other);

  SelfType operator++(int) {
    SelfType temp(*this);
    ++*this;
    return temp;
  }

  SelfType operator--(int) {
    SelfType temp(*this);
    --*this;
    return temp;
  }

  SelfType& operator--() {
	ValueIteratorBase<_Traits, _Alloc>::decrement();
    return *this;
  }

  SelfType& operator++() {
	ValueIteratorBase<_Traits, _Alloc>::increment();
    return *this;
  }

  reference operator*() const { return ValueIteratorBase<_Traits, _Alloc>::deref(); }

  pointer operator->() const { return &ValueIteratorBase<_Traits, _Alloc>::deref(); }
};

} // namespace Json


namespace std {
/// Specialize std::swap() for Json::Value.
template<typename _Traits, typename _Alloc>
inline void swap(Json::Value<_Traits, _Alloc>& a, Json::Value<_Traits, _Alloc>& b);
}


#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(pop)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

#endif // CPPTL_JSON_DECL_H_INCLUDED
