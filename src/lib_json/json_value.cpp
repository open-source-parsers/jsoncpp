// Copyright 2011 Baptiste Lepilleur and The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/assertions.h>
#include <json/value.h>
#include <json/writer.h>
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <cassert>
#include <cstring>
#include <math.h>
#include <sstream>
#include <utility>
#ifdef JSON_USE_CPPTL
#include <cpptl/conststring.h>
#endif
#include <algorithm> // min()
#include <cstddef>   // size_t

// Disable warning C4702 : unreachable code
#if defined(_MSC_VER) && _MSC_VER >= 1800 // VC++ 12.0 and above
#pragma warning(disable : 4702)
#endif

#define JSON_ASSERT_UNREACHABLE assert(false)

namespace Json {

// This is a walkaround to avoid the static initialization of Value::null.
// kNull must be word-aligned to avoid crashing on ARM.  We use an alignment of
// 8 (instead of 4) as a bit of future-proofing.
#if defined(__ARMEL__)
#define ALIGNAS(byte_alignment) __attribute__((aligned(byte_alignment)))
#else
#define ALIGNAS(byte_alignment)
#endif
// static const unsigned char ALIGNAS(8) kNull[sizeof(Value)] = { 0 };
// const unsigned char& kNullRef = kNull[0];
// const Value& Value::null = reinterpret_cast<const Value&>(kNullRef);
// const Value& Value::nullRef = null;

// static
Value const& Value::nullSingleton() {
  static Value const nullStatic;
  return nullStatic;
}

// for backwards compatibility, we'll leave these global references around, but
// DO NOT use them in JSONCPP library code any more!
Value const& Value::null = Value::nullSingleton();
Value const& Value::nullRef = Value::nullSingleton();

const Int Value::minInt = Int(~(UInt(-1) / 2));
const Int Value::maxInt = Int(UInt(-1) / 2);
const UInt Value::maxUInt = UInt(-1);
#if defined(JSON_HAS_INT64)
const Int64 Value::minInt64 = Int64(~(UInt64(-1) / 2));
const Int64 Value::maxInt64 = Int64(UInt64(-1) / 2);
const UInt64 Value::maxUInt64 = UInt64(-1);
// The constant is hard-coded because some compiler have trouble
// converting Value::maxUInt64 to a double correctly (AIX/xlC).
// Assumes that UInt64 is a 64 bits integer.
static const double maxUInt64AsDouble = 18446744073709551615.0;
#endif // defined(JSON_HAS_INT64)
const LargestInt Value::minLargestInt = LargestInt(~(LargestUInt(-1) / 2));
const LargestInt Value::maxLargestInt = LargestInt(LargestUInt(-1) / 2);
const LargestUInt Value::maxLargestUInt = LargestUInt(-1);

const UInt Value::defaultRealPrecision = 17;

#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
template <typename T, typename U>
static inline bool InRange(double d, T min, U max) {
  // The casts can lose precision, but we are looking only for
  // an approximate range. Might fail on edge cases though. ~cdunn
  // return d >= static_cast<double>(min) && d <= static_cast<double>(max);
  return d >= min && d <= max;
}
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
static inline double integerToDouble(Json::UInt64 value) {
  return static_cast<double>(Int64(value / 2)) * 2.0 +
         static_cast<double>(Int64(value & 1));
}

template <typename T> static inline double integerToDouble(T value) {
  return static_cast<double>(value);
}

template <typename T, typename U>
static inline bool InRange(double d, T min, U max) {
  return d >= integerToDouble(min) && d <= integerToDouble(max);
}
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)

/** Duplicates the specified string value.
 * @param value Pointer to the string to duplicate. Must be zero-terminated if
 *              length is "unknown".
 * @param length Length of the value. if equals to unknown, then it will be
 *               computed using strlen(value).
 * @return Pointer on the duplicate instance of string.
 */
static inline char* duplicateStringValue(const char* value, size_t length) {
  // Avoid an integer overflow in the call to malloc below by limiting length
  // to a sane value.
  if (length >= static_cast<size_t>(Value::maxInt))
    length = Value::maxInt - 1;

  char* newString = static_cast<char*>(malloc(length + 1));
  if (newString == NULL) {
    throwRuntimeError("in Json::Value::duplicateStringValue(): "
                      "Failed to allocate string value buffer");
  }
  memcpy(newString, value, length);
  newString[length] = 0;
  return newString;
}

/* Record the length as a prefix.
 */
static inline char* duplicateAndPrefixStringValue(const char* value,
                                                  unsigned int length) {
  // Avoid an integer overflow in the call to malloc below by limiting length
  // to a sane value.
  JSON_ASSERT_MESSAGE(length <= static_cast<unsigned>(Value::maxInt) -
                                    sizeof(unsigned) - 1U,
                      "in Json::Value::duplicateAndPrefixStringValue(): "
                      "length too big for prefixing");
  unsigned actualLength = length + static_cast<unsigned>(sizeof(unsigned)) + 1U;
  char* newString = static_cast<char*>(malloc(actualLength));
  if (newString == 0) {
    throwRuntimeError("in Json::Value::duplicateAndPrefixStringValue(): "
                      "Failed to allocate string value buffer");
  }
  *reinterpret_cast<unsigned*>(newString) = length;
  memcpy(newString + sizeof(unsigned), value, length);
  newString[actualLength - 1U] =
      0; // to avoid buffer over-run accidents by users later
  return newString;
}
inline static void decodePrefixedString(bool isPrefixed,
                                        char const* prefixed,
                                        unsigned* length,
                                        char const** value) {
  if (!isPrefixed) {
    *length = static_cast<unsigned>(strlen(prefixed));
    *value = prefixed;
  } else {
    *length = *reinterpret_cast<unsigned const*>(prefixed);
    *value = prefixed + sizeof(unsigned);
  }
}
/** Free the string duplicated by
 * duplicateStringValue()/duplicateAndPrefixStringValue().
 */
#if JSONCPP_USING_SECURE_MEMORY
static inline void releasePrefixedStringValue(char* value) {
  unsigned length = 0;
  char const* valueDecoded;
  decodePrefixedString(true, value, &length, &valueDecoded);
  size_t const size = sizeof(unsigned) + length + 1U;
  memset(value, 0, size);
  free(value);
}
static inline void releaseStringValue(char* value, unsigned length) {
  // length==0 => we allocated the strings memory
  size_t size = (length == 0) ? strlen(value) : length;
  memset(value, 0, size);
  free(value);
}
#else  // !JSONCPP_USING_SECURE_MEMORY
static inline void releasePrefixedStringValue(char* value) { free(value); }
static inline void releaseStringValue(char* value, unsigned) { free(value); }
#endif // JSONCPP_USING_SECURE_MEMORY

} // namespace Json

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// ValueInternals...
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
#if !defined(JSON_IS_AMALGAMATION)

#include "json_valueiterator.inl"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace Json {

Exception::Exception(JSONCPP_STRING const& msg) : msg_(msg) {}
Exception::~Exception() JSONCPP_NOEXCEPT {}
char const* Exception::what() const JSONCPP_NOEXCEPT { return msg_.c_str(); }
RuntimeError::RuntimeError(JSONCPP_STRING const& msg) : Exception(msg) {}
LogicError::LogicError(JSONCPP_STRING const& msg) : Exception(msg) {}
JSONCPP_NORETURN void throwRuntimeError(JSONCPP_STRING const& msg) {
  throw RuntimeError(msg);
}
JSONCPP_NORETURN void throwLogicError(JSONCPP_STRING const& msg) {
  throw LogicError(msg);
}

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Value::CommentInfo
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

Value::CommentInfo::CommentInfo() : comment_(0) {}

Value::CommentInfo::~CommentInfo() {
  if (comment_)
    releaseStringValue(comment_, 0u);
}

void Value::CommentInfo::setComment(const char* text, size_t len) {
  if (comment_) {
    releaseStringValue(comment_, 0u);
    comment_ = 0;
  }
  JSON_ASSERT(text != 0);
  JSON_ASSERT_MESSAGE(
      text[0] == '\0' || text[0] == '/',
      "in Json::Value::setComment(): Comments must start with /");
  // It seems that /**/ style comments are acceptable as well.
  comment_ = duplicateStringValue(text, len);
}

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Value::CZString
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

// Notes: policy_ indicates if the string was allocated when
// a string is stored.

Value::CZString::CZString(ArrayIndex aindex) : cstr_(0), index_(aindex) {}

Value::CZString::CZString(char const* str,
                          unsigned ulength,
                          DuplicationPolicy allocate)
    : cstr_(str) {
  // allocate != duplicate
  storage_.policy_ = allocate & 0x3;
  storage_.length_ = ulength & 0x3FFFFFFF;
}

Value::CZString::CZString(const CZString& other) {
  cstr_ = (other.storage_.policy_ != noDuplication && other.cstr_ != 0
               ? duplicateStringValue(other.cstr_, other.storage_.length_)
               : other.cstr_);
  storage_.policy_ =
      static_cast<unsigned>(
          other.cstr_
              ? (static_cast<DuplicationPolicy>(other.storage_.policy_) ==
                         noDuplication
                     ? noDuplication
                     : duplicate)
              : static_cast<DuplicationPolicy>(other.storage_.policy_)) &
      3U;
  storage_.length_ = other.storage_.length_;
}

#if JSON_HAS_RVALUE_REFERENCES
Value::CZString::CZString(CZString&& other)
    : cstr_(other.cstr_), index_(other.index_) {
  other.cstr_ = nullptr;
}
#endif

Value::CZString::~CZString() {
  if (cstr_ && storage_.policy_ == duplicate) {
    releaseStringValue(const_cast<char*>(cstr_),
                       storage_.length_ + 1u); // +1 for null terminating
                                               // character for sake of
                                               // completeness but not actually
                                               // necessary
  }
}

void Value::CZString::swap(CZString& other) {
  std::swap(cstr_, other.cstr_);
  std::swap(index_, other.index_);
}

Value::CZString& Value::CZString::operator=(const CZString& other) {
  cstr_ = other.cstr_;
  index_ = other.index_;
  return *this;
}

#if JSON_HAS_RVALUE_REFERENCES
Value::CZString& Value::CZString::operator=(CZString&& other) {
  cstr_ = other.cstr_;
  index_ = other.index_;
  other.cstr_ = nullptr;
  return *this;
}
#endif

bool Value::CZString::operator<(const CZString& other) const {
  if (!cstr_)
    return index_ < other.index_;
  // return strcmp(cstr_, other.cstr_) < 0;
  // Assume both are strings.
  unsigned this_len = this->storage_.length_;
  unsigned other_len = other.storage_.length_;
  unsigned min_len = std::min<unsigned>(this_len, other_len);
  JSON_ASSERT(this->cstr_ && other.cstr_);
  int comp = memcmp(this->cstr_, other.cstr_, min_len);
  if (comp < 0)
    return true;
  if (comp > 0)
    return false;
  return (this_len < other_len);
}

bool Value::CZString::operator==(const CZString& other) const {
  if (!cstr_)
    return index_ == other.index_;
  // return strcmp(cstr_, other.cstr_) == 0;
  // Assume both are strings.
  unsigned this_len = this->storage_.length_;
  unsigned other_len = other.storage_.length_;
  if (this_len != other_len)
    return false;
  JSON_ASSERT(this->cstr_ && other.cstr_);
  int comp = memcmp(this->cstr_, other.cstr_, this_len);
  return comp == 0;
}

ArrayIndex Value::CZString::index() const { return index_; }

// const char* Value::CZString::c_str() const { return cstr_; }
const char* Value::CZString::data() const { return cstr_; }
unsigned Value::CZString::length() const { return storage_.length_; }
bool Value::CZString::isStaticString() const {
  return storage_.policy_ == noDuplication;
}

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Value::Value
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

/*! \internal Default constructor initialization must be equivalent to:
 * memset( this, 0, sizeof(Value) )
 * This optimization is used in ValueInternalMap fast allocator.
 */
Value::Value(ValueType vtype) {
  static char const emptyString[] = "";
  initBasic(vtype);
  switch (vtype) {
  case nullValue:
    break;
  case intValue:
  case uintValue:
    value_.int_ = 0;
    break;
  case realValue:
    value_.real_ = 0.0;
    break;
  case stringValue:
    // allocated_ == false, so this is safe.
    value_.string_ = const_cast<char*>(static_cast<char const*>(emptyString));
    break;
  case arrayValue:
  case objectValue:
    value_.map_ = new ObjectValues();
    break;
  case booleanValue:
    value_.bool_ = false;
    break;
  default:
    JSON_ASSERT_UNREACHABLE;
  }
}

Value::Value(Int value) {
  initBasic(intValue);
  value_.int_ = value;
}

Value::Value(UInt value) {
  initBasic(uintValue);
  value_.uint_ = value;
}
#if defined(JSON_HAS_INT64)
Value::Value(Int64 value) {
  initBasic(intValue);
  value_.int_ = value;
}
Value::Value(UInt64 value) {
  initBasic(uintValue);
  value_.uint_ = value;
}
#endif // defined(JSON_HAS_INT64)

Value::Value(double value) {
  initBasic(realValue);
  value_.real_ = value;
}

Value::Value(const char* value) {
  initBasic(stringValue, true);
  JSON_ASSERT_MESSAGE(value != NULL, "Null Value Passed to Value Constructor");
  value_.string_ = duplicateAndPrefixStringValue(
      value, static_cast<unsigned>(strlen(value)));
}

Value::Value(const char* beginValue, const char* endValue) {
  initBasic(stringValue, true);
  value_.string_ = duplicateAndPrefixStringValue(
      beginValue, static_cast<unsigned>(endValue - beginValue));
}

Value::Value(const JSONCPP_STRING& value) {
  initBasic(stringValue, true);
  value_.string_ = duplicateAndPrefixStringValue(
      value.data(), static_cast<unsigned>(value.length()));
}

Value::Value(const StaticString& value) {
  initBasic(stringValue);
  value_.string_ = const_cast<char*>(value.c_str());
}

#ifdef JSON_USE_CPPTL
Value::Value(const CppTL::ConstString& value) {
  initBasic(stringValue, true);
  value_.string_ = duplicateAndPrefixStringValue(
      value, static_cast<unsigned>(value.length()));
}
#endif

Value::Value(bool value) {
  initBasic(booleanValue);
  value_.bool_ = value;
}

Value::Value(const Value& other) {
  dupPayload(other);
  dupMeta(other);
}

#if JSON_HAS_RVALUE_REFERENCES
// Move constructor
Value::Value(Value&& other) {
  initBasic(nullValue);
  swap(other);
}
#endif

Value::~Value() {
  releasePayload();

  delete[] comments_;

  value_.uint_ = 0;
}

Value& Value::operator=(Value other) {
  swap(other);
  return *this;
}

void Value::swapPayload(Value& other) {
  ValueType temp = type_;
  type_ = other.type_;
  other.type_ = temp;
  std::swap(value_, other.value_);
  int temp2 = allocated_;
  allocated_ = other.allocated_;
  other.allocated_ = temp2 & 0x1;
}

void Value::copyPayload(const Value& other) {
  releasePayload();
  dupPayload(other);
}

void Value::swap(Value& other) {
  swapPayload(other);
  std::swap(comments_, other.comments_);
  std::swap(start_, other.start_);
  std::swap(limit_, other.limit_);
}

void Value::copy(const Value& other) {
  copyPayload(other);
  delete[] comments_;
  dupMeta(other);
}

ValueType Value::type() const { return type_; }

int Value::compare(const Value& other) const {
  if (*this < other)
    return -1;
  if (*this > other)
    return 1;
  return 0;
}

bool Value::operator<(const Value& other) const {
  int typeDelta = type_ - other.type_;
  if (typeDelta)
    return typeDelta < 0 ? true : false;
  switch (type_) {
  case nullValue:
    return false;
  case intValue:
    return value_.int_ < other.value_.int_;
  case uintValue:
    return value_.uint_ < other.value_.uint_;
  case realValue:
    return value_.real_ < other.value_.real_;
  case booleanValue:
    return value_.bool_ < other.value_.bool_;
  case stringValue: {
    if ((value_.string_ == 0) || (other.value_.string_ == 0)) {
      if (other.value_.string_)
        return true;
      else
        return false;
    }
    unsigned this_len;
    unsigned other_len;
    char const* this_str;
    char const* other_str;
    decodePrefixedString(this->allocated_, this->value_.string_, &this_len,
                         &this_str);
    decodePrefixedString(other.allocated_, other.value_.string_, &other_len,
                         &other_str);
    unsigned min_len = std::min<unsigned>(this_len, other_len);
    JSON_ASSERT(this_str && other_str);
    int comp = memcmp(this_str, other_str, min_len);
    if (comp < 0)
      return true;
    if (comp > 0)
      return false;
    return (this_len < other_len);
  }
  case arrayValue:
  case objectValue: {
    int delta = int(value_.map_->size() - other.value_.map_->size());
    if (delta)
      return delta < 0;
    return (*value_.map_) < (*other.value_.map_);
  }
  default:
    JSON_ASSERT_UNREACHABLE;
  }
  return false; // unreachable
}

bool Value::operator<=(const Value& other) const { return !(other < *this); }

bool Value::operator>=(const Value& other) const { return !(*this < other); }

bool Value::operator>(const Value& other) const { return other < *this; }

bool Value::operator==(const Value& other) const {
  // if ( type_ != other.type_ )
  // GCC 2.95.3 says:
  // attempt to take address of bit-field structure member `Json::Value::type_'
  // Beats me, but a temp solves the problem.
  int temp = other.type_;
  if (type_ != temp)
    return false;
  switch (type_) {
  case nullValue:
    return true;
  case intValue:
    return value_.int_ == other.value_.int_;
  case uintValue:
    return value_.uint_ == other.value_.uint_;
  case realValue:
    return value_.real_ == other.value_.real_;
  case booleanValue:
    return value_.bool_ == other.value_.bool_;
  case stringValue: {
    if ((value_.string_ == 0) || (other.value_.string_ == 0)) {
      return (value_.string_ == other.value_.string_);
    }
    unsigned this_len;
    unsigned other_len;
    char const* this_str;
    char const* other_str;
    decodePrefixedString(this->allocated_, this->value_.string_, &this_len,
                         &this_str);
    decodePrefixedString(other.allocated_, other.value_.string_, &other_len,
                         &other_str);
    if (this_len != other_len)
      return false;
    JSON_ASSERT(this_str && other_str);
    int comp = memcmp(this_str, other_str, this_len);
    return comp == 0;
  }
  case arrayValue:
  case objectValue:
    return value_.map_->size() == other.value_.map_->size() &&
           (*value_.map_) == (*other.value_.map_);
  default:
    JSON_ASSERT_UNREACHABLE;
  }
  return false; // unreachable
}

bool Value::operator!=(const Value& other) const { return !(*this == other); }

const char* Value::asCString() const {
  JSON_ASSERT_MESSAGE(type_ == stringValue,
                      "in Json::Value::asCString(): requires stringValue");
  if (value_.string_ == 0)
    return 0;
  unsigned this_len;
  char const* this_str;
  decodePrefixedString(this->allocated_, this->value_.string_, &this_len,
                       &this_str);
  return this_str;
}

#if JSONCPP_USING_SECURE_MEMORY
unsigned Value::getCStringLength() const {
  JSON_ASSERT_MESSAGE(type_ == stringValue,
                      "in Json::Value::asCString(): requires stringValue");
  if (value_.string_ == 0)
    return 0;
  unsigned this_len;
  char const* this_str;
  decodePrefixedString(this->allocated_, this->value_.string_, &this_len,
                       &this_str);
  return this_len;
}
#endif

bool Value::getString(char const** str, char const** cend) const {
  if (type_ != stringValue)
    return false;
  if (value_.string_ == 0)
    return false;
  unsigned length;
  decodePrefixedString(this->allocated_, this->value_.string_, &length, str);
  *cend = *str + length;
  return true;
}

JSONCPP_STRING Value::asString() const {
  switch (type_) {
  case nullValue:
    return "";
  case stringValue: {
    if (value_.string_ == 0)
      return "";
    unsigned this_len;
    char const* this_str;
    decodePrefixedString(this->allocated_, this->value_.string_, &this_len,
                         &this_str);
    return JSONCPP_STRING(this_str, this_len);
  }
  case booleanValue:
    return value_.bool_ ? "true" : "false";
  case intValue:
    return valueToString(value_.int_);
  case uintValue:
    return valueToString(value_.uint_);
  case realValue:
    return valueToString(value_.real_);
  default:
    JSON_FAIL_MESSAGE("Type is not convertible to string");
  }
}

#ifdef JSON_USE_CPPTL
CppTL::ConstString Value::asConstString() const {
  unsigned len;
  char const* str;
  decodePrefixedString(allocated_, value_.string_, &len, &str);
  return CppTL::ConstString(str, len);
}
#endif

Value::Int Value::asInt() const {
  switch (type_) {
  case intValue:
    JSON_ASSERT_MESSAGE(isInt(), "LargestInt out of Int range");
    return Int(value_.int_);
  case uintValue:
    JSON_ASSERT_MESSAGE(isInt(), "LargestUInt out of Int range");
    return Int(value_.uint_);
  case realValue:
    JSON_ASSERT_MESSAGE(InRange(value_.real_, minInt, maxInt),
                        "double out of Int range");
    return Int(value_.real_);
  case nullValue:
    return 0;
  case booleanValue:
    return value_.bool_ ? 1 : 0;
  default:
    break;
  }
  JSON_FAIL_MESSAGE("Value is not convertible to Int.");
}

Value::UInt Value::asUInt() const {
  switch (type_) {
  case intValue:
    JSON_ASSERT_MESSAGE(isUInt(), "LargestInt out of UInt range");
    return UInt(value_.int_);
  case uintValue:
    JSON_ASSERT_MESSAGE(isUInt(), "LargestUInt out of UInt range");
    return UInt(value_.uint_);
  case realValue:
    JSON_ASSERT_MESSAGE(InRange(value_.real_, 0, maxUInt),
                        "double out of UInt range");
    return UInt(value_.real_);
  case nullValue:
    return 0;
  case booleanValue:
    return value_.bool_ ? 1 : 0;
  default:
    break;
  }
  JSON_FAIL_MESSAGE("Value is not convertible to UInt.");
}

#if defined(JSON_HAS_INT64)

Value::Int64 Value::asInt64() const {
  switch (type_) {
  case intValue:
    return Int64(value_.int_);
  case uintValue:
    JSON_ASSERT_MESSAGE(isInt64(), "LargestUInt out of Int64 range");
    return Int64(value_.uint_);
  case realValue:
    JSON_ASSERT_MESSAGE(InRange(value_.real_, minInt64, maxInt64),
                        "double out of Int64 range");
    return Int64(value_.real_);
  case nullValue:
    return 0;
  case booleanValue:
    return value_.bool_ ? 1 : 0;
  default:
    break;
  }
  JSON_FAIL_MESSAGE("Value is not convertible to Int64.");
}

Value::UInt64 Value::asUInt64() const {
  switch (type_) {
  case intValue:
    JSON_ASSERT_MESSAGE(isUInt64(), "LargestInt out of UInt64 range");
    return UInt64(value_.int_);
  case uintValue:
    return UInt64(value_.uint_);
  case realValue:
    JSON_ASSERT_MESSAGE(InRange(value_.real_, 0, maxUInt64),
                        "double out of UInt64 range");
    return UInt64(value_.real_);
  case nullValue:
    return 0;
  case booleanValue:
    return value_.bool_ ? 1 : 0;
  default:
    break;
  }
  JSON_FAIL_MESSAGE("Value is not convertible to UInt64.");
}
#endif // if defined(JSON_HAS_INT64)

LargestInt Value::asLargestInt() const {
#if defined(JSON_NO_INT64)
  return asInt();
#else
  return asInt64();
#endif
}

LargestUInt Value::asLargestUInt() const {
#if defined(JSON_NO_INT64)
  return asUInt();
#else
  return asUInt64();
#endif
}

double Value::asDouble() const {
  switch (type_) {
  case intValue:
    return static_cast<double>(value_.int_);
  case uintValue:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
    return static_cast<double>(value_.uint_);
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
    return integerToDouble(value_.uint_);
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
  case realValue:
    return value_.real_;
  case nullValue:
    return 0.0;
  case booleanValue:
    return value_.bool_ ? 1.0 : 0.0;
  default:
    break;
  }
  JSON_FAIL_MESSAGE("Value is not convertible to double.");
}

float Value::asFloat() const {
  switch (type_) {
  case intValue:
    return static_cast<float>(value_.int_);
  case uintValue:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
    return static_cast<float>(value_.uint_);
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
    // This can fail (silently?) if the value is bigger than MAX_FLOAT.
    return static_cast<float>(integerToDouble(value_.uint_));
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
  case realValue:
    return static_cast<float>(value_.real_);
  case nullValue:
    return 0.0;
  case booleanValue:
    return value_.bool_ ? 1.0f : 0.0f;
  default:
    break;
  }
  JSON_FAIL_MESSAGE("Value is not convertible to float.");
}

bool Value::asBool() const {
  switch (type_) {
  case booleanValue:
    return value_.bool_;
  case nullValue:
    return false;
  case intValue:
    return value_.int_ ? true : false;
  case uintValue:
    return value_.uint_ ? true : false;
  case realValue:
    // This is kind of strange. Not recommended.
    return (value_.real_ != 0.0) ? true : false;
  default:
    break;
  }
  JSON_FAIL_MESSAGE("Value is not convertible to bool.");
}

bool Value::isConvertibleTo(ValueType other) const {
  switch (other) {
  case nullValue:
    return (isNumeric() && asDouble() == 0.0) ||
           (type_ == booleanValue && value_.bool_ == false) ||
           (type_ == stringValue && asString().empty()) ||
           (type_ == arrayValue && value_.map_->size() == 0) ||
           (type_ == objectValue && value_.map_->size() == 0) ||
           type_ == nullValue;
  case intValue:
    return isInt() ||
           (type_ == realValue && InRange(value_.real_, minInt, maxInt)) ||
           type_ == booleanValue || type_ == nullValue;
  case uintValue:
    return isUInt() ||
           (type_ == realValue && InRange(value_.real_, 0, maxUInt)) ||
           type_ == booleanValue || type_ == nullValue;
  case realValue:
    return isNumeric() || type_ == booleanValue || type_ == nullValue;
  case booleanValue:
    return isNumeric() || type_ == booleanValue || type_ == nullValue;
  case stringValue:
    return isNumeric() || type_ == booleanValue || type_ == stringValue ||
           type_ == nullValue;
  case arrayValue:
    return type_ == arrayValue || type_ == nullValue;
  case objectValue:
    return type_ == objectValue || type_ == nullValue;
  }
  JSON_ASSERT_UNREACHABLE;
  return false;
}

/// Number of values in array or object
ArrayIndex Value::size() const {
  switch (type_) {
  case nullValue:
  case intValue:
  case uintValue:
  case realValue:
  case booleanValue:
  case stringValue:
    return 0;
  case arrayValue: // size of the array is highest index + 1
    if (!value_.map_->empty()) {
      ObjectValues::const_iterator itLast = value_.map_->end();
      --itLast;
      return (*itLast).first.index() + 1;
    }
    return 0;
  case objectValue:
    return ArrayIndex(value_.map_->size());
  }
  JSON_ASSERT_UNREACHABLE;
  return 0; // unreachable;
}

bool Value::empty() const {
  if (isNull() || isArray() || isObject())
    return size() == 0u;
  else
    return false;
}

Value::operator bool() const { return !isNull(); }

void Value::clear() {
  JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == arrayValue ||
                          type_ == objectValue,
                      "in Json::Value::clear(): requires complex value");
  start_ = 0;
  limit_ = 0;
  switch (type_) {
  case arrayValue:
  case objectValue:
    value_.map_->clear();
    break;
  default:
    break;
  }
}

void Value::resize(ArrayIndex newSize) {
  JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == arrayValue,
                      "in Json::Value::resize(): requires arrayValue");
  if (type_ == nullValue)
    *this = Value(arrayValue);
  ArrayIndex oldSize = size();
  if (newSize == 0)
    clear();
  else if (newSize > oldSize)
    this->operator[](newSize - 1);
  else {
    for (ArrayIndex index = newSize; index < oldSize; ++index) {
      value_.map_->erase(index);
    }
    JSON_ASSERT(size() == newSize);
  }
}

Value& Value::operator[](ArrayIndex index) {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == arrayValue,
      "in Json::Value::operator[](ArrayIndex): requires arrayValue");
  if (type_ == nullValue)
    *this = Value(arrayValue);
  CZString key(index);
  ObjectValues::iterator it = value_.map_->lower_bound(key);
  if (it != value_.map_->end() && (*it).first == key)
    return (*it).second;

  ObjectValues::value_type defaultValue(key, nullSingleton());
  it = value_.map_->insert(it, defaultValue);
  return (*it).second;
}

Value& Value::operator[](int index) {
  JSON_ASSERT_MESSAGE(
      index >= 0,
      "in Json::Value::operator[](int index): index cannot be negative");
  return (*this)[ArrayIndex(index)];
}

const Value& Value::operator[](ArrayIndex index) const {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == arrayValue,
      "in Json::Value::operator[](ArrayIndex)const: requires arrayValue");
  if (type_ == nullValue)
    return nullSingleton();
  CZString key(index);
  ObjectValues::const_iterator it = value_.map_->find(key);
  if (it == value_.map_->end())
    return nullSingleton();
  return (*it).second;
}

const Value& Value::operator[](int index) const {
  JSON_ASSERT_MESSAGE(
      index >= 0,
      "in Json::Value::operator[](int index) const: index cannot be negative");
  return (*this)[ArrayIndex(index)];
}

void Value::initBasic(ValueType vtype, bool allocated) {
  type_ = vtype;
  allocated_ = allocated;
  comments_ = 0;
  start_ = 0;
  limit_ = 0;
}

void Value::dupPayload(const Value& other) {
  type_ = other.type_;
  allocated_ = false;
  switch (type_) {
  case nullValue:
  case intValue:
  case uintValue:
  case realValue:
  case booleanValue:
    value_ = other.value_;
    break;
  case stringValue:
    if (other.value_.string_ && other.allocated_) {
      unsigned len;
      char const* str;
      decodePrefixedString(other.allocated_, other.value_.string_, &len, &str);
      value_.string_ = duplicateAndPrefixStringValue(str, len);
      allocated_ = true;
    } else {
      value_.string_ = other.value_.string_;
    }
    break;
  case arrayValue:
  case objectValue:
    value_.map_ = new ObjectValues(*other.value_.map_);
    break;
  default:
    JSON_ASSERT_UNREACHABLE;
  }
}

void Value::releasePayload() {
  switch (type_) {
  case nullValue:
  case intValue:
  case uintValue:
  case realValue:
  case booleanValue:
    break;
  case stringValue:
    if (allocated_)
      releasePrefixedStringValue(value_.string_);
    break;
  case arrayValue:
  case objectValue:
    delete value_.map_;
    break;
  default:
    JSON_ASSERT_UNREACHABLE;
  }
}

void Value::dupMeta(const Value& other) {
  if (other.comments_) {
    comments_ = new CommentInfo[numberOfCommentPlacement];
    for (int comment = 0; comment < numberOfCommentPlacement; ++comment) {
      const CommentInfo& otherComment = other.comments_[comment];
      if (otherComment.comment_)
        comments_[comment].setComment(otherComment.comment_,
                                      strlen(otherComment.comment_));
    }
  } else {
    comments_ = 0;
  }
  start_ = other.start_;
  limit_ = other.limit_;
}

// Access an object value by name, create a null member if it does not exist.
// @pre Type of '*this' is object or null.
// @param key is null-terminated.
Value& Value::resolveReference(const char* key) {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == objectValue,
      "in Json::Value::resolveReference(): requires objectValue");
  if (type_ == nullValue)
    *this = Value(objectValue);
  CZString actualKey(key, static_cast<unsigned>(strlen(key)),
                     CZString::noDuplication); // NOTE!
  ObjectValues::iterator it = value_.map_->lower_bound(actualKey);
  if (it != value_.map_->end() && (*it).first == actualKey)
    return (*it).second;

  ObjectValues::value_type defaultValue(actualKey, nullSingleton());
  it = value_.map_->insert(it, defaultValue);
  Value& value = (*it).second;
  return value;
}

// @param key is not null-terminated.
Value& Value::resolveReference(char const* key, char const* cend) {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == objectValue,
      "in Json::Value::resolveReference(key, end): requires objectValue");
  if (type_ == nullValue)
    *this = Value(objectValue);
  CZString actualKey(key, static_cast<unsigned>(cend - key),
                     CZString::duplicateOnCopy);
  ObjectValues::iterator it = value_.map_->lower_bound(actualKey);
  if (it != value_.map_->end() && (*it).first == actualKey)
    return (*it).second;

  ObjectValues::value_type defaultValue(actualKey, nullSingleton());
  it = value_.map_->insert(it, defaultValue);
  Value& value = (*it).second;
  return value;
}

Value Value::get(ArrayIndex index, const Value& defaultValue) const {
  const Value* value = &((*this)[index]);
  return value == &nullSingleton() ? defaultValue : *value;
}

bool Value::isValidIndex(ArrayIndex index) const { return index < size(); }

Value const* Value::find(char const* key, char const* cend) const {
  JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == objectValue,
                      "in Json::Value::find(key, end, found): requires "
                      "objectValue or nullValue");
  if (type_ == nullValue)
    return NULL;
  CZString actualKey(key, static_cast<unsigned>(cend - key),
                     CZString::noDuplication);
  ObjectValues::const_iterator it = value_.map_->find(actualKey);
  if (it == value_.map_->end())
    return NULL;
  return &(*it).second;
}
const Value& Value::operator[](const char* key) const {
  Value const* found = find(key, key + strlen(key));
  if (!found)
    return nullSingleton();
  return *found;
}
Value const& Value::operator[](JSONCPP_STRING const& key) const {
  Value const* found = find(key.data(), key.data() + key.length());
  if (!found)
    return nullSingleton();
  return *found;
}

Value& Value::operator[](const char* key) {
  return resolveReference(key, key + strlen(key));
}

Value& Value::operator[](const JSONCPP_STRING& key) {
  return resolveReference(key.data(), key.data() + key.length());
}

Value& Value::operator[](const StaticString& key) {
  return resolveReference(key.c_str());
}

#ifdef JSON_USE_CPPTL
Value& Value::operator[](const CppTL::ConstString& key) {
  return resolveReference(key.c_str(), key.end_c_str());
}
Value const& Value::operator[](CppTL::ConstString const& key) const {
  Value const* found = find(key.c_str(), key.end_c_str());
  if (!found)
    return nullSingleton();
  return *found;
}
#endif

Value& Value::append(const Value& value) { return (*this)[size()] = value; }

#if JSON_HAS_RVALUE_REFERENCES
Value& Value::append(Value&& value) {
  return (*this)[size()] = std::move(value);
}
#endif

Value Value::get(char const* key,
                 char const* cend,
                 Value const& defaultValue) const {
  Value const* found = find(key, cend);
  return !found ? defaultValue : *found;
}
Value Value::get(char const* key, Value const& defaultValue) const {
  return get(key, key + strlen(key), defaultValue);
}
Value Value::get(JSONCPP_STRING const& key, Value const& defaultValue) const {
  return get(key.data(), key.data() + key.length(), defaultValue);
}

bool Value::removeMember(const char* key, const char* cend, Value* removed) {
  if (type_ != objectValue) {
    return false;
  }
  CZString actualKey(key, static_cast<unsigned>(cend - key),
                     CZString::noDuplication);
  ObjectValues::iterator it = value_.map_->find(actualKey);
  if (it == value_.map_->end())
    return false;
  if (removed)
#if JSON_HAS_RVALUE_REFERENCES
    *removed = std::move(it->second);
#else
    *removed = it->second;
#endif
  value_.map_->erase(it);
  return true;
}
bool Value::removeMember(const char* key, Value* removed) {
  return removeMember(key, key + strlen(key), removed);
}
bool Value::removeMember(JSONCPP_STRING const& key, Value* removed) {
  return removeMember(key.data(), key.data() + key.length(), removed);
}
void Value::removeMember(const char* key) {
  JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == objectValue,
                      "in Json::Value::removeMember(): requires objectValue");
  if (type_ == nullValue)
    return;

  CZString actualKey(key, unsigned(strlen(key)), CZString::noDuplication);
  value_.map_->erase(actualKey);
}
void Value::removeMember(const JSONCPP_STRING& key) {
  removeMember(key.c_str());
}

bool Value::removeIndex(ArrayIndex index, Value* removed) {
  if (type_ != arrayValue) {
    return false;
  }
  CZString key(index);
  ObjectValues::iterator it = value_.map_->find(key);
  if (it == value_.map_->end()) {
    return false;
  }
  *removed = it->second;
  ArrayIndex oldSize = size();
  // shift left all items left, into the place of the "removed"
  for (ArrayIndex i = index; i < (oldSize - 1); ++i) {
    CZString keey(i);
    (*value_.map_)[keey] = (*this)[i + 1];
  }
  // erase the last one ("leftover")
  CZString keyLast(oldSize - 1);
  ObjectValues::iterator itLast = value_.map_->find(keyLast);
  value_.map_->erase(itLast);
  return true;
}

#ifdef JSON_USE_CPPTL
Value Value::get(const CppTL::ConstString& key,
                 const Value& defaultValue) const {
  return get(key.c_str(), key.end_c_str(), defaultValue);
}
#endif

bool Value::isMember(char const* key, char const* cend) const {
  Value const* value = find(key, cend);
  return NULL != value;
}
bool Value::isMember(char const* key) const {
  return isMember(key, key + strlen(key));
}
bool Value::isMember(JSONCPP_STRING const& key) const {
  return isMember(key.data(), key.data() + key.length());
}

#ifdef JSON_USE_CPPTL
bool Value::isMember(const CppTL::ConstString& key) const {
  return isMember(key.c_str(), key.end_c_str());
}
#endif

Value::Members Value::getMemberNames() const {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == objectValue,
      "in Json::Value::getMemberNames(), value must be objectValue");
  if (type_ == nullValue)
    return Value::Members();
  Members members;
  members.reserve(value_.map_->size());
  ObjectValues::const_iterator it = value_.map_->begin();
  ObjectValues::const_iterator itEnd = value_.map_->end();
  for (; it != itEnd; ++it) {
    members.push_back(JSONCPP_STRING((*it).first.data(), (*it).first.length()));
  }
  return members;
}
//
//# ifdef JSON_USE_CPPTL
// EnumMemberNames
// Value::enumMemberNames() const
//{
//   if ( type_ == objectValue )
//   {
//      return CppTL::Enum::any(  CppTL::Enum::transform(
//         CppTL::Enum::keys( *(value_.map_), CppTL::Type<const CZString &>() ),
//         MemberNamesTransform() ) );
//   }
//   return EnumMemberNames();
//}
//
//
// EnumValues
// Value::enumValues() const
//{
//   if ( type_ == objectValue  ||  type_ == arrayValue )
//      return CppTL::Enum::anyValues( *(value_.map_),
//                                     CppTL::Type<const Value &>() );
//   return EnumValues();
//}
//
//# endif

static bool IsIntegral(double d) {
  double integral_part;
  return modf(d, &integral_part) == 0.0;
}

bool Value::isNull() const { return type_ == nullValue; }

bool Value::isBool() const { return type_ == booleanValue; }

bool Value::isInt() const {
  switch (type_) {
  case intValue:
#if defined(JSON_HAS_INT64)
    return value_.int_ >= minInt && value_.int_ <= maxInt;
#else
    return true;
#endif
  case uintValue:
    return value_.uint_ <= UInt(maxInt);
  case realValue:
    return value_.real_ >= minInt && value_.real_ <= maxInt &&
           IsIntegral(value_.real_);
  default:
    break;
  }
  return false;
}

bool Value::isUInt() const {
  switch (type_) {
  case intValue:
#if defined(JSON_HAS_INT64)
    return value_.int_ >= 0 && LargestUInt(value_.int_) <= LargestUInt(maxUInt);
#else
    return value_.int_ >= 0;
#endif
  case uintValue:
#if defined(JSON_HAS_INT64)
    return value_.uint_ <= maxUInt;
#else
    return true;
#endif
  case realValue:
    return value_.real_ >= 0 && value_.real_ <= maxUInt &&
           IsIntegral(value_.real_);
  default:
    break;
  }
  return false;
}

bool Value::isInt64() const {
#if defined(JSON_HAS_INT64)
  switch (type_) {
  case intValue:
    return true;
  case uintValue:
    return value_.uint_ <= UInt64(maxInt64);
  case realValue:
    // Note that maxInt64 (= 2^63 - 1) is not exactly representable as a
    // double, so double(maxInt64) will be rounded up to 2^63. Therefore we
    // require the value to be strictly less than the limit.
    return value_.real_ >= double(minInt64) &&
           value_.real_ < double(maxInt64) && IsIntegral(value_.real_);
  default:
    break;
  }
#endif // JSON_HAS_INT64
  return false;
}

bool Value::isUInt64() const {
#if defined(JSON_HAS_INT64)
  switch (type_) {
  case intValue:
    return value_.int_ >= 0;
  case uintValue:
    return true;
  case realValue:
    // Note that maxUInt64 (= 2^64 - 1) is not exactly representable as a
    // double, so double(maxUInt64) will be rounded up to 2^64. Therefore we
    // require the value to be strictly less than the limit.
    return value_.real_ >= 0 && value_.real_ < maxUInt64AsDouble &&
           IsIntegral(value_.real_);
  default:
    break;
  }
#endif // JSON_HAS_INT64
  return false;
}

bool Value::isIntegral() const {
  switch (type_) {
  case intValue:
  case uintValue:
    return true;
  case realValue:
#if defined(JSON_HAS_INT64)
    // Note that maxUInt64 (= 2^64 - 1) is not exactly representable as a
    // double, so double(maxUInt64) will be rounded up to 2^64. Therefore we
    // require the value to be strictly less than the limit.
    return value_.real_ >= double(minInt64) &&
           value_.real_ < maxUInt64AsDouble && IsIntegral(value_.real_);
#else
    return value_.real_ >= minInt && value_.real_ <= maxUInt &&
           IsIntegral(value_.real_);
#endif // JSON_HAS_INT64
  default:
    break;
  }
  return false;
}

bool Value::isDouble() const {
  return type_ == intValue || type_ == uintValue || type_ == realValue;
}

bool Value::isNumeric() const { return isDouble(); }

bool Value::isString() const { return type_ == stringValue; }

bool Value::isArray() const { return type_ == arrayValue; }

bool Value::isObject() const { return type_ == objectValue; }

void Value::setComment(const char* comment,
                       size_t len,
                       CommentPlacement placement) {
  if (!comments_)
    comments_ = new CommentInfo[numberOfCommentPlacement];
  if ((len > 0) && (comment[len - 1] == '\n')) {
    // Always discard trailing newline, to aid indentation.
    len -= 1;
  }
  comments_[placement].setComment(comment, len);
}

void Value::setComment(const char* comment, CommentPlacement placement) {
  setComment(comment, strlen(comment), placement);
}

void Value::setComment(const JSONCPP_STRING& comment,
                       CommentPlacement placement) {
  setComment(comment.c_str(), comment.length(), placement);
}

bool Value::hasComment(CommentPlacement placement) const {
  return comments_ != 0 && comments_[placement].comment_ != 0;
}

JSONCPP_STRING Value::getComment(CommentPlacement placement) const {
  if (hasComment(placement))
    return comments_[placement].comment_;
  return "";
}

void Value::setOffsetStart(ptrdiff_t start) { start_ = start; }

void Value::setOffsetLimit(ptrdiff_t limit) { limit_ = limit; }

ptrdiff_t Value::getOffsetStart() const { return start_; }

ptrdiff_t Value::getOffsetLimit() const { return limit_; }

JSONCPP_STRING Value::toStyledString() const {
  StreamWriterBuilder builder;

  JSONCPP_STRING out = this->hasComment(commentBefore) ? "\n" : "";
  out += Json::writeString(builder, *this);
  out += "\n";

  return out;
}

Value::const_iterator Value::begin() const {
  switch (type_) {
  case arrayValue:
  case objectValue:
    if (value_.map_)
      return const_iterator(value_.map_->begin());
    break;
  default:
    break;
  }
  return const_iterator();
}

Value::const_iterator Value::end() const {
  switch (type_) {
  case arrayValue:
  case objectValue:
    if (value_.map_)
      return const_iterator(value_.map_->end());
    break;
  default:
    break;
  }
  return const_iterator();
}

Value::iterator Value::begin() {
  switch (type_) {
  case arrayValue:
  case objectValue:
    if (value_.map_)
      return iterator(value_.map_->begin());
    break;
  default:
    break;
  }
  return iterator();
}

Value::iterator Value::end() {
  switch (type_) {
  case arrayValue:
  case objectValue:
    if (value_.map_)
      return iterator(value_.map_->end());
    break;
  default:
    break;
  }
  return iterator();
}

// class PathArgument
// //////////////////////////////////////////////////////////////////

PathArgument::PathArgument() : key_(), index_(), kind_(kindNone) {}

PathArgument::PathArgument(ArrayIndex index)
    : key_(), index_(index), kind_(kindIndex) {}

PathArgument::PathArgument(const char* key)
    : key_(key), index_(), kind_(kindKey) {}

PathArgument::PathArgument(const JSONCPP_STRING& key)
    : key_(key.c_str()), index_(), kind_(kindKey) {}

// class Path
// //////////////////////////////////////////////////////////////////

Path::Path(const JSONCPP_STRING& path,
           const PathArgument& a1,
           const PathArgument& a2,
           const PathArgument& a3,
           const PathArgument& a4,
           const PathArgument& a5) {
  InArgs in;
  in.reserve(5);
  in.push_back(&a1);
  in.push_back(&a2);
  in.push_back(&a3);
  in.push_back(&a4);
  in.push_back(&a5);
  makePath(path, in);
}

void Path::makePath(const JSONCPP_STRING& path, const InArgs& in) {
  const char* current = path.c_str();
  const char* end = current + path.length();
  InArgs::const_iterator itInArg = in.begin();
  while (current != end) {
    if (*current == '[') {
      ++current;
      if (*current == '%')
        addPathInArg(path, in, itInArg, PathArgument::kindIndex);
      else {
        ArrayIndex index = 0;
        for (; current != end && *current >= '0' && *current <= '9'; ++current)
          index = index * 10 + ArrayIndex(*current - '0');
        args_.push_back(index);
      }
      if (current == end || *++current != ']')
        invalidPath(path, int(current - path.c_str()));
    } else if (*current == '%') {
      addPathInArg(path, in, itInArg, PathArgument::kindKey);
      ++current;
    } else if (*current == '.' || *current == ']') {
      ++current;
    } else {
      const char* beginName = current;
      while (current != end && !strchr("[.", *current))
        ++current;
      args_.push_back(JSONCPP_STRING(beginName, current));
    }
  }
}

void Path::addPathInArg(const JSONCPP_STRING& /*path*/,
                        const InArgs& in,
                        InArgs::const_iterator& itInArg,
                        PathArgument::Kind kind) {
  if (itInArg == in.end()) {
    // Error: missing argument %d
  } else if ((*itInArg)->kind_ != kind) {
    // Error: bad argument type
  } else {
    args_.push_back(**itInArg++);
  }
}

void Path::invalidPath(const JSONCPP_STRING& /*path*/, int /*location*/) {
  // Error: invalid path.
}

const Value& Path::resolve(const Value& root) const {
  const Value* node = &root;
  for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
    const PathArgument& arg = *it;
    if (arg.kind_ == PathArgument::kindIndex) {
      if (!node->isArray() || !node->isValidIndex(arg.index_)) {
        // Error: unable to resolve path (array value expected at position...
        return Value::null;
      }
      node = &((*node)[arg.index_]);
    } else if (arg.kind_ == PathArgument::kindKey) {
      if (!node->isObject()) {
        // Error: unable to resolve path (object value expected at position...)
        return Value::null;
      }
      node = &((*node)[arg.key_]);
      if (node == &Value::nullSingleton()) {
        // Error: unable to resolve path (object has no member named '' at
        // position...)
        return Value::null;
      }
    }
  }
  return *node;
}

Value Path::resolve(const Value& root, const Value& defaultValue) const {
  const Value* node = &root;
  for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
    const PathArgument& arg = *it;
    if (arg.kind_ == PathArgument::kindIndex) {
      if (!node->isArray() || !node->isValidIndex(arg.index_))
        return defaultValue;
      node = &((*node)[arg.index_]);
    } else if (arg.kind_ == PathArgument::kindKey) {
      if (!node->isObject())
        return defaultValue;
      node = &((*node)[arg.key_]);
      if (node == &Value::nullSingleton())
        return defaultValue;
    }
  }
  return *node;
}

Value& Path::make(Value& root) const {
  Value* node = &root;
  for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
    const PathArgument& arg = *it;
    if (arg.kind_ == PathArgument::kindIndex) {
      if (!node->isArray()) {
        // Error: node is not an array at position ...
      }
      node = &((*node)[arg.index_]);
    } else if (arg.kind_ == PathArgument::kindKey) {
      if (!node->isObject()) {
        // Error: node is not an object at position...
      }
      node = &((*node)[arg.key_]);
    }
  }
  return *node;
}

} // namespace Json
