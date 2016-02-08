// Copyright 2011 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef CPPTL_JSON_VALUE_INL_INCLUDED
#define CPPTL_JSON_VALUE_INL_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "assertions.h"
#include "value.h"
#include "writer.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <math.h>
#include <sstream>
#include <utility>
#include <cstring>
#include <cassert>
#ifdef JSON_USE_CPPTL
#include <cpptl/conststring.h>
#endif
#include <cstddef> // size_t
#include <algorithm> // min()

#define JSON_ASSERT_UNREACHABLE assert(false)

namespace Json {
namespace detail {

// This is a walkaround to avoid the static initialization of Value::null.
// kNull must be word-aligned to avoid crashing on ARM.  We use an alignment of
// 8 (instead of 4) as a bit of future-proofing.
#if defined(__ARMEL__)
#define ALIGNAS(byte_alignment) __attribute__((aligned(byte_alignment)))
#else
#define ALIGNAS(byte_alignment)
#endif
static const unsigned char ALIGNAS(8) kNull[2048] = { 0 }; //FIXME no sizeof(Value) exists
static const unsigned char& kNullRef = kNull[0];
template <typename T, typename U>
const Value<T, U>& Value<T, U>::null = reinterpret_cast<const Value<T, U>&>(kNullRef);
template <typename T, typename U>
const Value<T, U>& Value<T, U>::nullRef = null;

template <typename T, typename U>
const Int Value<T, U>::minInt = Int(~(UInt(-1) / 2));
template <typename T, typename U>
const Int Value<T, U>::maxInt = Int(UInt(-1) / 2);
template <typename T, typename U>
const UInt Value<T, U>::maxUInt = UInt(-1);
#if defined(JSON_HAS_INT64)
template <typename T, typename U>
const Int64 Value<T, U>::minInt64 = Int64(~(UInt64(-1) / 2));
template <typename T, typename U>
const Int64 Value<T, U>::maxInt64 = Int64(UInt64(-1) / 2);
template <typename T, typename U>
const UInt64 Value<T, U>::maxUInt64 = UInt64(-1);
// The constant is hard-coded because some compiler have trouble
// converting Value::maxUInt64 to a double correctly (AIX/xlC).
// Assumes that UInt64 is a 64 bits integer.
static const double maxUInt64AsDouble = 18446744073709551615.0;
#endif // defined(JSON_HAS_INT64)
template <typename T, typename U>
const LargestInt Value<T, U>::minLargestInt = LargestInt(~(LargestUInt(-1) / 2));
template <typename T, typename U>
const LargestInt Value<T, U>::maxLargestInt = LargestInt(LargestUInt(-1) / 2);
template <typename T, typename U>
const LargestUInt Value<T, U>::maxLargestUInt = LargestUInt(-1);

#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
template <typename T, typename U>
static inline bool InRange(double d, T min, U max) {
  // The casts can lose precision, but we are looking only for
  // an approximate range. Might fail on edge cases though. ~cdunn
  //return d >= static_cast<double>(min) && d <= static_cast<double>(max);
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

/** Duplicates the specified string value.
 * @param value Pointer to the string to duplicate. Must be zero-terminated if
 *              length is "unknown".
 * @param length Length of the value. if equals to unknown, then it will be
 *               computed using strlen(value).
 * @return Pointer on the duplicate instance of string.
 */
template<class _Value>
static inline typename _Value::StringDataPtr duplicateStringValue(const char* value,
                                         size_t length) {
  // Avoid an integer overflow in the call to malloc below by limiting length
  // to a sane value.
  if (length >= (size_t)_Value::maxInt)
    length = _Value::maxInt - 1;

  try {
	  typename _Value::StringDataPtr newString(new typename _Value::StringData(value, value + length));
	  newString->push_back(0);
	  return newString;
  } catch (...) {
    throwRuntimeError(
        "in Json::Value::duplicateStringValue(): "
        "Failed to allocate string value buffer");
  }
  return typename _Value::StringDataPtr(); //Not reachable but compiler warns if not here
}

/* Record the length as a prefix.
 */
template<class _Value>
static inline typename _Value::StringDataPtr duplicateAndPrefixStringValue(
    const char* value,
    unsigned int length)
{
  // Avoid an integer overflow in the call to malloc below by limiting length
  // to a sane value.
  JSON_ASSERT_MESSAGE(length <= (unsigned)_Value::maxInt - sizeof(unsigned) - 1U,
                      "in Json::Value::duplicateAndPrefixStringValue(): "
                      "length too big for prefixing");

  try {
	typename  _Value::StringDataPtr newString(new typename _Value::StringData());
    for (unsigned int i=0; i<sizeof(unsigned); i++)
      newString->push_back(reinterpret_cast<char*>(&length)[i]);
    newString->insert(newString->end(), value, value+length);
    newString->push_back(0);
    return newString;
  } catch (...) {
    throwRuntimeError(
        "in Json::Value::duplicateAndPrefixStringValue(): "
        "Failed to allocate string value buffer");
  }
  return typename _Value::StringDataPtr(); //Not reachable but compiler warns if not here
}
template<class _Value>
inline static void decodePrefixedString(
    bool isPrefixed, char const* prefixed,
    unsigned* length, char const** value)
{
  if (!isPrefixed) {
    *length = static_cast<unsigned>(strlen(prefixed));
    *value = prefixed;
  } else {
    *length = *reinterpret_cast<unsigned const*>(prefixed);
    *value = prefixed + sizeof(unsigned);
  }
}
/** Free the string duplicated by duplicateStringValue()/duplicateAndPrefixStringValue().
 */
template<class _Value>
static inline void releaseStringValue(char* value) { (void)(value);/* Unused */ } //FIXME Remove!

} // namespace detail
} // namespace Json

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// ValueInternals...
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
#if !defined(JSON_IS_AMALGAMATION)

#include "valueiterator.inl"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace Json {
namespace detail {

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Value::CommentInfo
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

template<class _Alloc, class _String>
Value<_Alloc, _String>::CommentInfo::CommentInfo() : comment_(0) {}

template<class _Alloc, class _String>
Value<_Alloc, _String>::CommentInfo::~CommentInfo() {
  if (comment_.GetString())
    releaseStringValue<Value<_Alloc, _String>>(comment_.GetString());
}

template<class _Alloc, class _String>
void Value<_Alloc, _String>::CommentInfo::setComment(const char* text, size_t len) {
  if (comment_.GetString()) {
    releaseStringValue<Value<_Alloc, _String>>(comment_.GetString());
    comment_.SetString(0);
  }
  JSON_ASSERT(text != 0);
  JSON_ASSERT_MESSAGE(
      text[0] == '\0' || text[0] == '/',
      "in Json::Value::setComment(): Comments must start with /");
  // It seems that /**/ style comments are acceptable as well.
  comment_.SetString(duplicateStringValue<Value<_Alloc, _String>>(text, len));
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

template<class _Alloc, class _String>
Value<_Alloc, _String>::CZString::CZString(ArrayIndex aindex) : index_(aindex) {}

template<class _Alloc, class _String>
Value<_Alloc, _String>::CZString::CZString(char const* str, unsigned ulength, DuplicationPolicy allocate)
    : cstr_(const_cast<char*>(str)) {
  // allocate != duplicate
  storage_.policy_ = allocate & 0x3;
  storage_.length_ = ulength & 0x3FFFFFFF;
}

template<class _Alloc, class _String>
Value<_Alloc, _String>::CZString::CZString(const CZString& other) {
  if (other.storage_.policy_ != noDuplication && other.cstr_.GetString() != 0) {
	  cstr_.SetString(std::move(duplicateStringValue<Value<_Alloc, _String>>(other.cstr_.GetString(), other.storage_.length_)));
  } else {
	  cstr_.SetString(const_cast<char*>(other.cstr_.GetString()));
  }
  storage_.policy_ = (other.cstr_.GetString()
                 ? (static_cast<DuplicationPolicy>(other.storage_.policy_) == noDuplication
                     ? noDuplication : duplicate)
                 : static_cast<DuplicationPolicy>(other.storage_.policy_));
  storage_.length_ = other.storage_.length_;
}

#if JSON_HAS_RVALUE_REFERENCES
template<class _Alloc, class _String>
Value<_Alloc, _String>::CZString::CZString(CZString&& other)
  : cstr_(other.cstr_), index_(other.index_) {
  other.cstr_ = nullptr;
}
#endif

template<class _Alloc, class _String>
Value<_Alloc, _String>::CZString::~CZString() {
  if (cstr_.GetString() && storage_.policy_ == duplicate)
    releaseStringValue<Value<_Alloc, _String>>(const_cast<char*>(cstr_.GetString()));
}

template<class _Alloc, class _String>
void Value<_Alloc, _String>::CZString::swap(CZString& other) {
  std::swap(cstr_, other.cstr_);
  std::swap(index_, other.index_);
}

template<class _Alloc, class _String>
typename Value<_Alloc, _String>::CZString& Value<_Alloc, _String>::CZString::operator=(CZString other) {
  swap(other);
  return *this;
}

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::CZString::operator<(const CZString& other) const {
  if (!cstr_.GetString()) return index_ < other.index_;
  //return strcmp(cstr_, other.cstr_) < 0;
  // Assume both are strings.
  unsigned this_len = this->storage_.length_;
  unsigned other_len = other.storage_.length_;
  unsigned min_len = std::min(this_len, other_len);
  int comp = memcmp(this->cstr_.GetString(), other.cstr_.GetString(), min_len);
  if (comp < 0) return true;
  if (comp > 0) return false;
  return (this_len < other_len);
}

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::CZString::operator==(const CZString& other) const {
  if (!cstr_.GetString()) return index_ == other.index_;
  //return strcmp(cstr_, other.cstr_) == 0;
  // Assume both are strings.
  unsigned this_len = this->storage_.length_;
  unsigned other_len = other.storage_.length_;
  if (this_len != other_len) return false;
  int comp = memcmp(this->cstr_.GetString(), other.cstr_.GetString(), this_len);
  return comp == 0;
}

template<class _Alloc, class _String>
ArrayIndex Value<_Alloc, _String>::CZString::index() const { return index_; }

//const char* Value::CZString::c_str() const { return cstr_; }
template<class _Alloc, class _String>
const char* Value<_Alloc, _String>::CZString::data() const { return cstr_.GetString(); }
template<class _Alloc, class _String>
unsigned Value<_Alloc, _String>::CZString::length() const { return storage_.length_; }
template<class _Alloc, class _String>
bool Value<_Alloc, _String>::CZString::isStaticString() const { return storage_.policy_ == noDuplication; }

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
template<class _Alloc, class _String>
Value<_Alloc, _String>::Value(ValueType vtype) {
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
    //Unused
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

template<class _Alloc, class _String>
Value<_Alloc, _String>::Value(Int value) {
  initBasic(intValue);
  value_.int_ = value;
}

template<class _Alloc, class _String>
Value<_Alloc, _String>::Value(UInt value) {
  initBasic(uintValue);
  value_.uint_ = value;
}
#if defined(JSON_HAS_INT64)
template<class _Alloc, class _String>
Value<_Alloc, _String>::Value(Int64 value) {
  initBasic(intValue);
  value_.int_ = value;
}
template<class _Alloc, class _String>
Value<_Alloc, _String>::Value(UInt64 value) {
  initBasic(uintValue);
  value_.uint_ = value;
}
#endif // defined(JSON_HAS_INT64)

template<class _Alloc, class _String>
Value<_Alloc, _String>::Value(double value) {
  initBasic(realValue);
  value_.real_ = value;
}

template<class _Alloc, class _String>
Value<_Alloc, _String>::Value(const char* value) {
  initBasic(stringValue, true);
  stringValue_.SetString(duplicateAndPrefixStringValue<Value<_Alloc, _String>>(value, static_cast<unsigned>(strlen(value))));
}

template<class _Alloc, class _String>
Value<_Alloc, _String>::Value(const char* beginValue, const char* endValue) {
  initBasic(stringValue, true);
  stringValue_.SetString(
      duplicateAndPrefixStringValue<Value<_Alloc, _String>>(beginValue, static_cast<unsigned>(endValue - beginValue)));
}

template<class _Alloc, class _String>
template<class CharT, class Traits, class BSAllocator>
Value<_Alloc, _String>::Value(const std::basic_string<CharT, Traits, BSAllocator>& value) {
  initBasic(stringValue, true);
  stringValue_.SetString(
      duplicateAndPrefixStringValue<Value<_Alloc, _String>>(value.data(), static_cast<unsigned>(value.length())));
}

template<class _Alloc, class _String>
Value<_Alloc, _String>::Value(const StaticString& value) {
  initBasic(stringValue);
  stringValue_.SetString(const_cast<char*>(value.c_str()));
}

#ifdef JSON_USE_CPPTL
template<class _Alloc, class _String>
Value::Value(const CppTL::ConstString& value) {
  initBasic(stringValue, true);
  value_.string_ = duplicateAndPrefixStringValue(value, static_cast<unsigned>(value.length()));
}
#endif

template<class _Alloc, class _String>
Value<_Alloc, _String>::Value(bool value) {
  initBasic(booleanValue);
  value_.bool_ = value;
}

template<class _Alloc, class _String>
Value<_Alloc, _String>::Value(Value const& other)
    : type_(other.type_), allocated_(false)
      ,
      comments_(0), start_(other.start_), limit_(other.limit_)
{
  switch (type_) {
  case nullValue:
  case intValue:
  case uintValue:
  case realValue:
  case booleanValue:
    value_ = other.value_;
    break;
  case stringValue:
    if (other.stringValue_.GetString() && other.allocated_) {
      unsigned len;
      char const* str;
      decodePrefixedString<Value<_Alloc, _String>>(other.allocated_, other.stringValue_.GetString(),
          &len, &str);
      stringValue_.SetString(duplicateAndPrefixStringValue<Value<_Alloc, _String>>(str, len));
      allocated_ = true;
    } else {
	  stringValue_.SetString(const_cast<char*>(other.stringValue_.GetString()));
      allocated_ = false;
    }
    break;
  case arrayValue:
  case objectValue:
    value_.map_ = new ObjectValues(*other.value_.map_);
    break;
  default:
    JSON_ASSERT_UNREACHABLE;
  }
  if (other.comments_) {
    comments_ = new CommentInfo[numberOfCommentPlacement];
    for (int comment = 0; comment < numberOfCommentPlacement; ++comment) {
      const CommentInfo& otherComment = other.comments_[comment];
      if (otherComment.comment_.GetString())
        comments_[comment].setComment(
            otherComment.comment_.GetString(), strlen(otherComment.comment_.GetString()));
    }
  }
}

#if JSON_HAS_RVALUE_REFERENCES
// Move constructor
template<class _Alloc, class _String>
Value<_Alloc, _String>::Value(Value&& other) {
  initBasic(nullValue);
  swap(other);
}
#endif

template<class _Alloc, class _String>
Value<_Alloc, _String>::~Value() {
  switch (type_) {
  case nullValue:
  case intValue:
  case uintValue:
  case realValue:
  case booleanValue:
    break;
  case stringValue:
    if (allocated_ && !(stringValue_.IsRaw()))
      releaseStringValue<Value<_Alloc, _String>>(stringValue_.GetString());
    break;
  case arrayValue:
  case objectValue:
    delete value_.map_;
    break;
  default:
    JSON_ASSERT_UNREACHABLE;
  }

  if (comments_)
    delete[] comments_;
}

template<class _Alloc, class _String>
Value<_Alloc, _String>& Value<_Alloc, _String>::operator=(Value other) {
  swap(other);
  return *this;
}

template<class _Alloc, class _String>
void Value<_Alloc, _String>::swapPayload(Value<_Alloc, _String>& other) {
  ValueType temp = type_;
  type_ = other.type_;
  other.type_ = temp;
  std::swap(value_, other.value_);
  int temp2 = allocated_;
  allocated_ = other.allocated_;
  other.allocated_ = temp2 & 0x1;
  std::swap(stringValue_, other.stringValue_);
}

template<class _Alloc, class _String>
void Value<_Alloc, _String>::swap(Value<_Alloc, _String>& other) {
  swapPayload(other);
  std::swap(comments_, other.comments_);
  std::swap(start_, other.start_);
  std::swap(limit_, other.limit_);
}

template<class _Alloc, class _String>
ValueType Value<_Alloc, _String>::type() const { return type_; }

template<class _Alloc, class _String>
int Value<_Alloc, _String>::compare(const Value<_Alloc, _String>& other) const {
  if (*this < other)
    return -1;
  if (*this > other)
    return 1;
  return 0;
}

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::operator<(const Value<_Alloc, _String>& other) const {
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
  case stringValue:
  {
    if ((stringValue_.GetString() == 0) || (other.stringValue_.GetString() == 0)) {
      if (other.stringValue_.GetString()) return true;
      else return false;
    }
    unsigned this_len;
    unsigned other_len;
    char const* this_str;
    char const* other_str;
    decodePrefixedString<Value<_Alloc, _String>>(this->allocated_, this->stringValue_.GetString(), &this_len, &this_str);
    decodePrefixedString<Value<_Alloc, _String>>(other.allocated_, other.stringValue_.GetString(), &other_len, &other_str);
    unsigned min_len = std::min(this_len, other_len);
    int comp = memcmp(this_str, other_str, min_len);
    if (comp < 0) return true;
    if (comp > 0) return false;
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

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::operator<=(const Value<_Alloc, _String>& other) const { return !(other < *this); }

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::operator>=(const Value<_Alloc, _String>& other) const { return !(*this < other); }

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::operator>(const Value<_Alloc, _String>& other) const { return other < *this; }

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::operator==(const Value<_Alloc, _String>& other) const {
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
  case stringValue:
  {
    if ((stringValue_.GetString() == 0) || (other.stringValue_.GetString() == 0)) {
      return (stringValue_.GetString() == other.stringValue_.GetString());
    }
    unsigned this_len;
    unsigned other_len;
    char const* this_str;
    char const* other_str;
    decodePrefixedString<Value<_Alloc, _String>>(this->allocated_, this->stringValue_.GetString(), &this_len, &this_str);
    decodePrefixedString<Value<_Alloc, _String>>(other.allocated_, other.stringValue_.GetString(), &other_len, &other_str);
    if (this_len != other_len) return false;
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

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::operator!=(const Value<_Alloc, _String>& other) const { return !(*this == other); }

template<class _Alloc, class _String>
const char* Value<_Alloc, _String>::asCString() const {
  JSON_ASSERT_MESSAGE(type_ == stringValue,
                      "in Json::Value::asCString(): requires stringValue");
  if (stringValue_.GetString() == 0) return 0;
  unsigned this_len;
  char const* this_str;
  decodePrefixedString<Value<_Alloc, _String>>(this->allocated_, this->stringValue_.GetString(), &this_len, &this_str);
  return this_str;
}

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::getString(char const** str, char const** cend) const {
  if (type_ != stringValue) return false;
  if (stringValue_.GetString() == 0) return false;
  unsigned length;
  decodePrefixedString<Value<_Alloc, _String>>(this->allocated_, stringValue_.GetString(), &length, str);
  *cend = *str + length;
  return true;
}

template<class _Alloc, class _String>
_String Value<_Alloc, _String>::asString() const {
  switch (type_) {
  case nullValue:
    return "";
  case stringValue:
  {
    if (stringValue_.GetString() == 0) return "";
    unsigned this_len;
    char const* this_str;
    decodePrefixedString<Value<_Alloc, _String>>(this->allocated_, this->stringValue_.GetString(), &this_len, &this_str);
    return String(this_str, this_len);
  }
  case booleanValue:
    return value_.bool_ ? "true" : "false";
  case intValue:
    return valueToString<Value<_Alloc, _String>>(value_.int_);
  case uintValue:
    return valueToString<Value<_Alloc, _String>>(value_.uint_);
  case realValue:
    return valueToString<Value<_Alloc, _String>>(value_.real_);
  default:
    JSON_FAIL_MESSAGE("Type is not convertible to string");
  }
}

template<class _Alloc, class _String>
template<class RString>
RString Value<_Alloc, _String>::asTemplateString() const {
  switch (type_) {
    case nullValue:
      return "";
    case stringValue:
    {
	  if (stringValue_.GetString() == 0) return "";
  	  unsigned this_len;
	  char const* this_str;
	  decodePrefixedString<Value<_Alloc, RString>>(this->allocated_, this->stringValue_.GetString(), &this_len, &this_str);
	  return RString(this_str, this_len);
    }
    case booleanValue:
	  return value_.bool_ ? "true" : "false";
    case intValue:
	  return valueToString<Value<_Alloc, RString>>(value_.int_);
    case uintValue:
	  return valueToString<Value<_Alloc, RString>>(value_.uint_);
    case realValue:
	  return valueToString<Value<_Alloc, RString>>(value_.real_);
    default:
	  JSON_FAIL_MESSAGE("Type is not convertible to string");
  }
}

#ifdef JSON_USE_CPPTL
template<class _Alloc, class _String>
CppTL::ConstString Value<_Alloc, _String>::asConstString() const {
  unsigned len;
  char const* str;
  decodePrefixedString(allocated_, value_.string_,
      &len, &str);
  return CppTL::ConstString(str, len);
}
#endif

template<class _Alloc, class _String>
typename Value<_Alloc, _String>::Int Value<_Alloc, _String>::asInt() const {
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

template<class _Alloc, class _String>
typename Value<_Alloc, _String>::UInt Value<_Alloc, _String>::asUInt() const {
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

template<class _Alloc, class _String>
typename Value<_Alloc, _String>::Int64 Value<_Alloc, _String>::asInt64() const {
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

template<class _Alloc, class _String>
typename Value<_Alloc, _String>::UInt64 Value<_Alloc, _String>::asUInt64() const {
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

template<class _Alloc, class _String>
LargestInt Value<_Alloc, _String>::asLargestInt() const {
#if defined(JSON_NO_INT64)
  return asInt();
#else
  return asInt64();
#endif
}

template<class _Alloc, class _String>
LargestUInt Value<_Alloc, _String>::asLargestUInt() const {
#if defined(JSON_NO_INT64)
  return asUInt();
#else
  return asUInt64();
#endif
}

template<class _Alloc, class _String>
double Value<_Alloc, _String>::asDouble() const {
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

template<class _Alloc, class _String>
float Value<_Alloc, _String>::asFloat() const {
  switch (type_) {
  case intValue:
    return static_cast<float>(value_.int_);
  case uintValue:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
    return static_cast<float>(value_.uint_);
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
    return integerToDouble(value_.uint_);
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

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::asBool() const {
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

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isConvertibleTo(ValueType other) const {
  switch (other) {
  case nullValue:
    return (isNumeric() && asDouble() == 0.0) ||
           (type_ == booleanValue && value_.bool_ == false) ||
           (type_ == stringValue && asString() == "") ||
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
template<class _Alloc, class _String>
ArrayIndex Value<_Alloc, _String>::size() const {
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
    	typename ObjectValues::const_iterator itLast = value_.map_->end();
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

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::empty() const {
  if (isNull() || isArray() || isObject())
    return size() == 0u;
  else
    return false;
}

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::operator!() const { return isNull(); }

template<class _Alloc, class _String>
void Value<_Alloc, _String>::clear() {
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

template<class _Alloc, class _String>
void Value<_Alloc, _String>::resize(ArrayIndex newSize) {
  JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == arrayValue,
                      "in Json::Value::resize(): requires arrayValue");
  if (type_ == nullValue)
    *this = Value(arrayValue);
  ArrayIndex oldSize = size();
  if (newSize == 0)
    clear();
  else if (newSize > oldSize)
    (*this)[newSize - 1];
  else {
    for (ArrayIndex index = newSize; index < oldSize; ++index) {
      value_.map_->erase(index);
    }
    assert(size() == newSize);
  }
}

template<class _Alloc, class _String>
Value<_Alloc, _String>& Value<_Alloc, _String>::operator[](ArrayIndex index) {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == arrayValue,
      "in Json::Value::operator[](ArrayIndex): requires arrayValue");
  if (type_ == nullValue)
    *this = Value(arrayValue);
  CZString key(index);
  typename ObjectValues::iterator it = value_.map_->lower_bound(key);
  if (it != value_.map_->end() && (*it).first == key)
    return (*it).second;

  typename ObjectValues::value_type defaultValue(key, nullRef);
  it = value_.map_->insert(it, defaultValue);
  return (*it).second;
}

template<class _Alloc, class _String>
Value<_Alloc, _String>& Value<_Alloc, _String>::operator[](int index) {
  JSON_ASSERT_MESSAGE(
      index >= 0,
      "in Json::Value::operator[](int index): index cannot be negative");
  return (*this)[ArrayIndex(index)];
}

template<class _Alloc, class _String>
const Value<_Alloc, _String>& Value<_Alloc, _String>::operator[](ArrayIndex index) const {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == arrayValue,
      "in Json::Value::operator[](ArrayIndex)const: requires arrayValue");
  if (type_ == nullValue)
    return nullRef;
  CZString key(index);
  typename ObjectValues::const_iterator it = value_.map_->find(key);
  if (it == value_.map_->end())
    return nullRef;
  return (*it).second;
}

template<class _Alloc, class _String>
const Value<_Alloc, _String>& Value<_Alloc, _String>::operator[](int index) const {
  JSON_ASSERT_MESSAGE(
      index >= 0,
      "in Json::Value::operator[](int index) const: index cannot be negative");
  return (*this)[ArrayIndex(index)];
}

template<class _Alloc, class _String>
void Value<_Alloc, _String>::initBasic(ValueType vtype, bool allocated) {
  type_ = vtype;
  allocated_ = allocated;
  comments_ = 0;
  start_ = 0;
  limit_ = 0;
}

// Access an object value by name, create a null member if it does not exist.
// @pre Type of '*this' is object or null.
// @param key is null-terminated.
template<class _Alloc, class _String>
Value<_Alloc, _String>& Value<_Alloc, _String>::resolveReference(const char* key) {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == objectValue,
      "in Json::Value::resolveReference(): requires objectValue");
  if (type_ == nullValue)
    *this = Value(objectValue);
  CZString actualKey(
      key, static_cast<unsigned>(strlen(key)), CZString::noDuplication); // NOTE!
  typename ObjectValues::iterator it = value_.map_->lower_bound(actualKey);
  if (it != value_.map_->end() && (*it).first == actualKey)
    return (*it).second;

  typename ObjectValues::value_type defaultValue(actualKey, nullRef);
  it = value_.map_->insert(it, defaultValue);
  Value& value = (*it).second;
  return value;
}

// @param key is not null-terminated.
template<class _Alloc, class _String>
Value<_Alloc, _String>& Value<_Alloc, _String>::resolveReference(char const* key, char const* cend)
{
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == objectValue,
      "in Json::Value::resolveReference(key, end): requires objectValue");
  if (type_ == nullValue)
    *this = Value(objectValue);
  CZString actualKey(
      key, static_cast<unsigned>(cend-key), CZString::duplicateOnCopy);
  typename ObjectValues::iterator it = value_.map_->lower_bound(actualKey);
  if (it != value_.map_->end() && (*it).first == actualKey)
    return (*it).second;

  typename ObjectValues::value_type defaultValue(actualKey, nullRef);
  it = value_.map_->insert(it, defaultValue);
  Value& value = (*it).second;
  return value;
}

template<class _Alloc, class _String>
Value<_Alloc, _String> Value<_Alloc, _String>::get(ArrayIndex index, const Value& defaultValue) const {
  const Value* value = &((*this)[index]);
  return value == &nullRef ? defaultValue : *value;
}

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isValidIndex(ArrayIndex index) const { return index < size(); }

template<class _Alloc, class _String>
Value<_Alloc, _String> const* Value<_Alloc, _String>::find(char const* key, char const* cend) const
{
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == objectValue,
      "in Json::Value::find(key, end, found): requires objectValue or nullValue");
  if (type_ == nullValue) return NULL;
  CZString actualKey(key, static_cast<unsigned>(cend-key), CZString::noDuplication);
  typename ObjectValues::const_iterator it = value_.map_->find(actualKey);
  if (it == value_.map_->end()) return NULL;
  return &(*it).second;
}

template<class _Alloc, class _String>
const Value<_Alloc, _String>& Value<_Alloc, _String>::operator[](const char* key) const
{
  Value const* found = find(key, key + strlen(key));
  if (!found) return nullRef;
  return *found;
}

template<class _Alloc, class _String>
template<class CharT, class Traits, class BSAllocator>
Value<_Alloc, _String> const& Value<_Alloc, _String>::operator[](std::basic_string<CharT, Traits, BSAllocator> const& key) const
{
  Value const* found = find(key.data(), key.data() + key.length());
  if (!found) return nullRef;
  return *found;
}

template<class _Alloc, class _String>
Value<_Alloc, _String>& Value<_Alloc, _String>::operator[](const char* key) {
  return resolveReference(key, key + strlen(key));
}

template<class _Alloc, class _String>
template<class CharT, class Traits, class BSAllocator>
Value<_Alloc, _String>& Value<_Alloc, _String>::operator[](const std::basic_string<CharT, Traits, BSAllocator>& key) {
  return resolveReference(key.data(), key.data() + key.length());
}

template<class _Alloc, class _String>
Value<_Alloc, _String>& Value<_Alloc, _String>::operator[](const StaticString& key) {
  return resolveReference(key.c_str());
}

#ifdef JSON_USE_CPPTL
template<class _Alloc, class _String>
Value<_Alloc, _String>& Value<_Alloc, _String>::operator[](const CppTL::ConstString& key) {
  return resolveReference(key.c_str(), key.end_c_str());
}
template<class _Alloc, class _String>
Value<_Alloc, _String> const& Value<_Alloc, _String>::operator[](CppTL::ConstString const& key) const
{
  Value const* found = find(key.c_str(), key.end_c_str());
  if (!found) return nullRef;
  return *found;
}
#endif

template<class _Alloc, class _String>
Value<_Alloc, _String>& Value<_Alloc, _String>::append(const Value<_Alloc, _String>& value) { return (*this)[size()] = value; }

template<class _Alloc, class _String>
Value<_Alloc, _String> Value<_Alloc, _String>::get(char const* key, char const* cend, Value<_Alloc, _String> const& defaultValue) const
{
  Value const* found = find(key, cend);
  return !found ? defaultValue : *found;
}

template<class _Alloc, class _String>
Value<_Alloc, _String> Value<_Alloc, _String>::get(char const* key, Value<_Alloc, _String> const& defaultValue) const
{
  return get(key, key + strlen(key), defaultValue);
}

template<class _Alloc, class _String>
template<class CharT, class Traits, class BSAllocator>
Value<_Alloc, _String> Value<_Alloc, _String>::get(std::basic_string<CharT, Traits, BSAllocator> const& key, Value<_Alloc, _String> const& defaultValue) const
{
  return get(key.data(), key.data() + key.length(), defaultValue);
}


template<class _Alloc, class _String>
bool Value<_Alloc, _String>::removeMember(const char* key, const char* cend, Value<_Alloc, _String>* removed)
{
  if (type_ != objectValue) {
    return false;
  }
  CZString actualKey(key, static_cast<unsigned>(cend-key), CZString::noDuplication);
  typename ObjectValues::iterator it = value_.map_->find(actualKey);
  if (it == value_.map_->end())
    return false;
  *removed = it->second;
  value_.map_->erase(it);
  return true;
}
template<class _Alloc, class _String>
bool Value<_Alloc, _String>::removeMember(const char* key, Value<_Alloc, _String>* removed)
{
  return removeMember(key, key + strlen(key), removed);
}
template<class _Alloc, class _String>
bool Value<_Alloc, _String>::removeMember(String const& key, Value<_Alloc, _String>* removed)
{
  return removeMember(key.data(), key.data() + key.length(), removed);
}
template<class _Alloc, class _String>
Value<_Alloc, _String> Value<_Alloc, _String>::removeMember(const char* key)
{
  JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == objectValue,
                      "in Json::Value::removeMember(): requires objectValue");
  if (type_ == nullValue)
    return nullRef;

  Value removed;  // null
  removeMember(key, key + strlen(key), &removed);
  return removed; // still null if removeMember() did nothing
}
template<class _Alloc, class _String>
Value<_Alloc, _String> Value<_Alloc, _String>::removeMember(const String& key)
{
  return removeMember(key.c_str());
}

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::removeIndex(ArrayIndex index, Value<_Alloc, _String>* removed) {
  if (type_ != arrayValue) {
    return false;
  }
  CZString key(index);
  typename ObjectValues::iterator it = value_.map_->find(key);
  if (it == value_.map_->end()) {
    return false;
  }
  *removed = it->second;
  ArrayIndex oldSize = size();
  // shift left all items left, into the place of the "removed"
  for (ArrayIndex i = index; i < (oldSize - 1); ++i){
    CZString keey(i);
    (*value_.map_)[keey] = (*this)[i + 1];
  }
  // erase the last one ("leftover")
  CZString keyLast(oldSize - 1);
  typename ObjectValues::iterator itLast = value_.map_->find(keyLast);
  value_.map_->erase(itLast);
  return true;
}

#ifdef JSON_USE_CPPTL
template<class _Alloc, class _String>
Value<_Alloc, _String> Value<_Alloc, _String>::get(const CppTL::ConstString& key,
                 const Value<_Alloc, _String>& defaultValue) const {
  return get(key.c_str(), key.end_c_str(), defaultValue);
}
#endif

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isMember(char const* key, char const* cend) const
{
  Value const* value = find(key, cend);
  return NULL != value;
}
template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isMember(char const* key) const
{
  return isMember(key, key + strlen(key));
}
template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isMember(String const& key) const
{
  return isMember(key.data(), key.data() + key.length());
}

#ifdef JSON_USE_CPPTL
template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isMember(const CppTL::ConstString& key) const {
  return isMember(key.c_str(), key.end_c_str());
}
#endif

template<class _Alloc, class _String>
typename Value<_Alloc, _String>::Members Value<_Alloc, _String>::getMemberNames() const {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == objectValue,
      "in Json::Value::getMemberNames(), value must be objectValue");
  if (type_ == nullValue)
    return Value::Members();
  Members members;
  members.reserve(value_.map_->size());
  typename ObjectValues::const_iterator it = value_.map_->begin();
  typename ObjectValues::const_iterator itEnd = value_.map_->end();
  for (; it != itEnd; ++it) {
    members.push_back(String((*it).first.data(),
                                  (*it).first.length()));
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

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isNull() const { return type_ == nullValue; }

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isBool() const { return type_ == booleanValue; }

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isInt() const {
  switch (type_) {
  case intValue:
    return value_.int_ >= minInt && value_.int_ <= maxInt;
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

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isUInt() const {
  switch (type_) {
  case intValue:
    return value_.int_ >= 0 && LargestUInt(value_.int_) <= LargestUInt(maxUInt);
  case uintValue:
    return value_.uint_ <= maxUInt;
  case realValue:
    return value_.real_ >= 0 && value_.real_ <= maxUInt &&
           IsIntegral(value_.real_);
  default:
    break;
  }
  return false;
}

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isInt64() const {
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

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isUInt64() const {
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

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isIntegral() const {
#if defined(JSON_HAS_INT64)
  return isInt64() || isUInt64();
#else
  return isInt() || isUInt();
#endif
}

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isDouble() const { return type_ == realValue || isIntegral(); }

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isNumeric() const { return isIntegral() || isDouble(); }

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isString() const { return type_ == stringValue; }

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isArray() const { return type_ == arrayValue; }

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::isObject() const { return type_ == objectValue; }

template<class _Alloc, class _String>
void Value<_Alloc, _String>::setComment(const char* comment, size_t len, CommentPlacement placement) {
  if (!comments_)
    comments_ = new CommentInfo[numberOfCommentPlacement];
  if ((len > 0) && (comment[len-1] == '\n')) {
    // Always discard trailing newline, to aid indentation.
    len -= 1;
  }
  comments_[placement].setComment(comment, len);
}

template<class _Alloc, class _String>
void Value<_Alloc, _String>::setComment(const char* comment, CommentPlacement placement) {
  setComment(comment, strlen(comment), placement);
}

template<class _Alloc, class _String>
void Value<_Alloc, _String>::setComment(const String& comment, CommentPlacement placement) {
  setComment(comment.c_str(), comment.length(), placement);
}

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::hasComment(CommentPlacement placement) const {
  return comments_ != 0 && comments_[placement].comment_.GetString() != 0;
}

template<class _Alloc, class _String>
_String Value<_Alloc, _String>::getComment(CommentPlacement placement) const {
  if (hasComment(placement))
    return comments_[placement].comment_.GetString();
  return "";
}

template<class _Alloc, class _String>
void Value<_Alloc, _String>::setOffsetStart(ptrdiff_t start) { start_ = start; }

template<class _Alloc, class _String>
void Value<_Alloc, _String>::setOffsetLimit(ptrdiff_t limit) { limit_ = limit; }

template<class _Alloc, class _String>
ptrdiff_t Value<_Alloc, _String>::getOffsetStart() const { return start_; }

template<class _Alloc, class _String>
ptrdiff_t Value<_Alloc, _String>::getOffsetLimit() const { return limit_; }

template<class _Alloc, class _String>
_String Value<_Alloc, _String>::toStyledString() const {
  StyledWriter<Value<_Alloc, _String>> writer;
  return writer.write(*this);
}

template<class _Alloc, class _String>
template<class RString>
RString Value<_Alloc, _String>::toStyledTemplateString() const {
  StyledWriter<Value<_Alloc, _String>> writer;
  auto written = writer.write(*this);
  return RString(written.begin(), written.end());
}

template<class _Alloc, class _String>
typename Value<_Alloc, _String>::const_iterator Value<_Alloc, _String>::begin() const {
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

template<class _Alloc, class _String>
typename Value<_Alloc, _String>::const_iterator Value<_Alloc, _String>::end() const {
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

template<class _Alloc, class _String>
typename Value<_Alloc, _String>::iterator Value<_Alloc, _String>::begin() {
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

template<class _Alloc, class _String>
typename Value<_Alloc, _String>::iterator Value<_Alloc, _String>::end() {
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

template<class _Alloc, class _String>
Value<_Alloc, _String>::StringValueHolder::StringValueHolder() {
  /* Not used */
}

template<class _Alloc, class _String>
Value<_Alloc, _String>::StringValueHolder::StringValueHolder(const StringValueHolder& other) {
  copy(other);
}

template<class _Alloc, class _String>
Value<_Alloc, _String>::StringValueHolder::StringValueHolder(StringValueHolder&& other) {
  swap(std::move(other));
}

template<class _Alloc, class _String>
Value<_Alloc, _String>::StringValueHolder::StringValueHolder(StringDataPtr&& value) {
  SetString(std::move(value));
}

template<class _Alloc, class _String>
Value<_Alloc, _String>::StringValueHolder::StringValueHolder(char* value) {
  SetString(value);
}

template<class _Alloc, class _String>
Value<_Alloc, _String>::StringValueHolder::~StringValueHolder() {
  /* Not used */
}

template<class _Alloc, class _String>
char* Value<_Alloc, _String>::StringValueHolder::GetString() {
  if (raw_) {
	return valueStringRaw_;
  } else if (valueStringCopy_) {
	return valueStringCopy_->data();
  } else {
	return nullptr;
  }
}

template<class _Alloc, class _String>
const char* Value<_Alloc, _String>::StringValueHolder::GetString() const {
  if (raw_) {
	return valueStringRaw_;
  } else if (valueStringCopy_) {
	return valueStringCopy_->data();
  } else {
	return nullptr;
  }
}

template<class _Alloc, class _String>
void Value<_Alloc, _String>::StringValueHolder::SetString(char* value) {
  valueStringRaw_ = value;
  raw_ = true;
}

template<class _Alloc, class _String>
void Value<_Alloc, _String>::StringValueHolder::SetString(StringDataPtr&& value) {
  std::swap(valueStringCopy_, value);
  raw_ = false;
}

template<class _Alloc, class _String>
bool Value<_Alloc, _String>::StringValueHolder::IsRaw() const {
  return raw_;
}

template<class _Alloc, class _String>
typename Value<_Alloc, _String>::StringValueHolder& Value<_Alloc, _String>::StringValueHolder::operator=(const StringValueHolder& other) {
  copy(other);
  return *this;
}

template<class _Alloc, class _String>
typename Value<_Alloc, _String>::StringValueHolder& Value<_Alloc, _String>::StringValueHolder::operator=(StringValueHolder&& other) {
  swap(std::move(other));
  return *this;
}

template<class _Alloc, class _String>
void Value<_Alloc, _String>::StringValueHolder::copy(const StringValueHolder& other) {
  valueStringRaw_ = other.valueStringRaw_;
  if (other.valueStringCopy_)
    valueStringCopy_ = StringDataPtr(new StringData(*other.valueStringCopy_));
  raw_ = other.raw_;
}

template<class _Alloc, class _String>
void Value<_Alloc, _String>::StringValueHolder::swap(StringValueHolder&& other) {
  std::swap(valueStringRaw_, other.valueStringRaw_);
  std::swap(valueStringCopy_, other.valueStringCopy_);
  std::swap(raw_, other.raw_);
}

// class PathArgument
// //////////////////////////////////////////////////////////////////

template<class _Value>
PathArgument<_Value>::PathArgument() : key_(), index_(), kind_(kindNone) {}

template<class _Value>
PathArgument<_Value>::PathArgument(ArrayIndex index)
    : key_(), index_(index), kind_(kindIndex) {}

template<class _Value>
PathArgument<_Value>::PathArgument(const char* key)
    : key_(key), index_(), kind_(kindKey) {}

template<class _Value>
PathArgument<_Value>::PathArgument(const String& key)
    : key_(key.c_str()), index_(), kind_(kindKey) {}

// class Path
// //////////////////////////////////////////////////////////////////

template<class _Value>
Path<_Value>::Path(const String& path,
           const PathArgument<_Value>& a1,
           const PathArgument<_Value>& a2,
           const PathArgument<_Value>& a3,
           const PathArgument<_Value>& a4,
           const PathArgument<_Value>& a5) {
  InArgs in;
  in.push_back(&a1);
  in.push_back(&a2);
  in.push_back(&a3);
  in.push_back(&a4);
  in.push_back(&a5);
  makePath(path, in);
}

template<class _Value>
void Path<_Value>::makePath(const String& path, const InArgs& in) {
  const char* current = path.c_str();
  const char* end = current + path.length();
  typename InArgs::const_iterator itInArg = in.begin();
  while (current != end) {
    if (*current == '[') {
      ++current;
      if (*current == '%')
        addPathInArg(path, in, itInArg, PathArgument<_Value>::kindIndex);
      else {
        ArrayIndex index = 0;
        for (; current != end && *current >= '0' && *current <= '9'; ++current)
          index = index * 10 + ArrayIndex(*current - '0');
        args_.push_back(index);
      }
      if (current == end || *current++ != ']')
        invalidPath(path, int(current - path.c_str()));
    } else if (*current == '%') {
      addPathInArg(path, in, itInArg, PathArgument<_Value>::kindKey);
      ++current;
    } else if (*current == '.') {
      ++current;
    } else {
      const char* beginName = current;
      while (current != end && !strchr("[.", *current))
        ++current;
      args_.push_back(String(beginName, current));
    }
  }
}

template<class _Value>
void Path<_Value>::addPathInArg(const String& /*path*/,
                        const InArgs& in,
                        typename InArgs::const_iterator& itInArg,
                        typename PathArgument<_Value>::Kind kind) {
  if (itInArg == in.end()) {
    // Error: missing argument %d
  } else if ((*itInArg)->kind_ != kind) {
    // Error: bad argument type
  } else {
    args_.push_back(**itInArg);
  }
}

template<class _Value>
void Path<_Value>::invalidPath(const String& /*path*/, int /*location*/) {
  // Error: invalid path.
}

template<class _Value>
const _Value& Path<_Value>::resolve(const _Value& root) const {
  const _Value* node = &root;
  for (typename Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
    const PathArgument<_Value>& arg = *it;
    if (arg.kind_ == PathArgument<_Value>::kindIndex) {
      if (!node->isArray() || !node->isValidIndex(arg.index_)) {
        // Error: unable to resolve path (array value expected at position...
      }
      node = &((*node)[arg.index_]);
    } else if (arg.kind_ == PathArgument<_Value>::kindKey) {
      if (!node->isObject()) {
        // Error: unable to resolve path (object value expected at position...)
      }
      node = &((*node)[arg.key_]);
      if (node == &_Value::nullRef) {
        // Error: unable to resolve path (object has no member named '' at
        // position...)
      }
    }
  }
  return *node;
}

template<class _Value>
_Value Path<_Value>::resolve(const _Value& root, const _Value& defaultValue) const {
  const _Value* node = &root;
  for (typename Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
    const PathArgument<_Value>& arg = *it;
    if (arg.kind_ == PathArgument<_Value>::kindIndex) {
      if (!node->isArray() || !node->isValidIndex(arg.index_))
        return defaultValue;
      node = &((*node)[arg.index_]);
    } else if (arg.kind_ == PathArgument<_Value>::kindKey) {
      if (!node->isObject())
        return defaultValue;
      node = &((*node)[arg.key_]);
      if (node == &_Value::nullRef)
        return defaultValue;
    }
  }
  return *node;
}

template<class _Value>
_Value& Path<_Value>::make(_Value& root) const {
  _Value* node = &root;
  for (typename Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
    const PathArgument<_Value>& arg = *it;
    if (arg.kind_ == PathArgument<_Value>::kindIndex) {
      if (!node->isArray()) {
        // Error: node is not an array at position ...
      }
      node = &((*node)[arg.index_]);
    } else if (arg.kind_ == PathArgument<_Value>::kindKey) {
      if (!node->isObject()) {
        // Error: node is not an object at position...
      }
      node = &((*node)[arg.key_]);
    }
  }
  return *node;
}

} // namespace detail
} // namespace Json

#endif // CPPTL_JSON_VALUE_INL_INCLUDED
