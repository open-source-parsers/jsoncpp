// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef CPPTL_JSON_H_INCLUDED
#define CPPTL_JSON_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "value_declaration.h"
#endif // if !defined(JSON_IS_AMALGAMATION)

/** \brief JSON (JavaScript Object Notation).
 */
namespace Json {

template<typename _Traits, typename _Alloc>
const unsigned char ALIGNAS(8) Value<_Traits, _Alloc>::kNull[2048] = {0}; //FIXME sizeof(Value<_Traits, _Alloc>) cannot be determined
template<typename _Traits, typename _Alloc>
const Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::null = reinterpret_cast<const Value<_Traits, _Alloc>&>(kNull[0]);
template<typename _Traits, typename _Alloc>
const Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::nullRef = null;

template<typename _Traits, typename _Alloc>
const Int Value<_Traits, _Alloc>::minInt = Int(~(UInt(-1) / 2));
template<typename _Traits, typename _Alloc>
const Int Value<_Traits, _Alloc>::maxInt = Int(UInt(-1) / 2);
template<typename _Traits, typename _Alloc>
const UInt Value<_Traits, _Alloc>::maxUInt = UInt(-1);
#if defined(JSON_HAS_INT64)
template<typename _Traits, typename _Alloc>
const Int64 Value<_Traits, _Alloc>::minInt64 = Int64(~(UInt64(-1) / 2));
template<typename _Traits, typename _Alloc>
const Int64 Value<_Traits, _Alloc>::maxInt64 = Int64(UInt64(-1) / 2);
template<typename _Traits, typename _Alloc>
const UInt64 Value<_Traits, _Alloc>::maxUInt64 = UInt64(-1);
// The constant is hard-coded because some compiler have trouble
// converting Value<_Traits, _Alloc>::maxUInt64 to a double correctly (AIX/xlC).
// Assumes that UInt64 is a 64 bits integer.
template<typename _Traits, typename _Alloc>
const double Value<_Traits, _Alloc>::maxUInt64AsDouble = 18446744073709551615.0;
#endif // defined(JSON_HAS_INT64)
template<typename _Traits, typename _Alloc>
const LargestInt Value<_Traits, _Alloc>::minLargestInt = LargestInt(~(LargestUInt(-1) / 2));
template<typename _Traits, typename _Alloc>
const LargestInt Value<_Traits, _Alloc>::maxLargestInt = LargestInt(LargestUInt(-1) / 2);
template<typename _Traits, typename _Alloc>
const LargestUInt Value<_Traits, _Alloc>::maxLargestUInt = LargestUInt(-1);

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Value::CommentInfo
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::CommentInfo::CommentInfo() {}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::CommentInfo::~CommentInfo() {}

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::CommentInfo::setComment(const char* text, size_t len) {
  if (comment_) {
    comment_ = string_data_type();
  }
  JSON_ASSERT(text != 0);
  JSON_ASSERT_MESSAGE(
      text[0] == '\0' || text[0] == '/',
      "in Json::Value<_Traits, _Alloc>::setComment(): Comments must start with /");
  // It seems that /**/ style comments are acceptable as well.
  comment_ = duplicateStringValue(text, len);
}

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::CommentInfo::setComment(const string_data_type& text, size_t len) {
  if (comment_) {
    comment_ = string_data_type();
  }
  JSON_ASSERT(text);
  JSON_ASSERT_MESSAGE(
      text->data()[0] == '\0' || text->data()[0] == '/',
      "in Json::Value<_Traits, _Alloc>::setComment(): Comments must start with /");
  // It seems that /**/ style comments are acceptable as well.
  comment_ = duplicateStringValue(text->data(), len);
}

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Value<_Traits, _Alloc>::CZString
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

// Notes: policy_ indicates if the string was allocated when
// a string is stored.

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::CZString::CZString(ArrayIndex aindex) : cstrNoDup_(nullptr), index_(aindex) {}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::CZString::CZString(char const* str, unsigned ulength, DuplicationPolicy allocate)
    : cstrNoDup_(str) {
  // allocate != duplicate
  storage_.policy_ = allocate & 0x3;
  storage_.length_ = ulength & 0x3FFFFFFF;
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::CZString::CZString(const CZString& other) : cstrNoDup_(nullptr) {
  switch (other.storage_.policy_) {
	case noDuplication:
	  cstrNoDup_ = other.cstrNoDup_;
	  break;
	case duplicate:
	  if (other.cstr_)
		cstr_ = duplicateStringValue(other.cstr_->data(), other.storage_.length_);
	  break;
	case duplicateOnCopy:
	  if (other.cstrNoDup_)
		cstr_ = duplicateStringValue(other.cstrNoDup_, other.storage_.length_);
	  break;
  }

  storage_.policy_ = (other.cstr_
                 ? (static_cast<DuplicationPolicy>(other.storage_.policy_) == noDuplication
                     ? noDuplication : duplicate)
                 : static_cast<DuplicationPolicy>(other.storage_.policy_));
  if (other.cstrNoDup_ && storage_.policy_ == duplicateOnCopy) //We've been copied, now the policy is just duplicate.
	  storage_.policy_ = duplicate;
  storage_.length_ = other.storage_.length_;
}

#if JSON_HAS_RVALUE_REFERENCES
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::CZString::CZString(CZString&& other)
  : cstrNoDup_(other.cstrNoDup_), index_(other.index_) {
  std::swap(cstr_, other.cstr_);
  other.cstrNoDup_ = nullptr;
}
#endif

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::CZString::~CZString() {
  /* Not Used */
}

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::CZString::swap(CZString& other) {
  std::swap(cstr_, other.cstr_);
  std::swap(cstrNoDup_, other.cstrNoDup_);
  std::swap(index_, other.index_);
}

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::CZString& Value<_Traits, _Alloc>::CZString::operator=(CZString other) {
  swap(other);
  return *this;
}

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::CZString::operator<(const CZString& other) const {
  if (storage_.policy_ == duplicate) {
	if (!cstr_) return index_ < other.index_;
  } else {
  	if (!cstrNoDup_) return index_ < other.index_;
  }
  //return strcmp(cstr_, other.cstr_) < 0;
  // Assume both are strings.
  unsigned this_len = this->storage_.length_;
  unsigned other_len = other.storage_.length_;
  unsigned min_len = std::min(this_len, other_len);
  const char* thisData = (storage_.policy_ == duplicate) ? this->cstr_->data() : this->cstrNoDup_;
  const char* otherData = (other.storage_.policy_ == duplicate) ? other.cstr_->data() : other.cstrNoDup_;
  int comp = memcmp(thisData, otherData, min_len);
  if (comp < 0) return true;
  if (comp > 0) return false;
  return (this_len < other_len);
}

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::CZString::operator==(const CZString& other) const {
  if (storage_.policy_ == duplicate) {
    if (!cstr_) return index_ == other.index_;
  } else {
    if (!cstrNoDup_) return index_ == other.index_;
  }

  //return strcmp(cstr_, other.cstr_) == 0;
  // Assume both are strings.
  unsigned this_len = this->storage_.length_;
  unsigned other_len = other.storage_.length_;
  if (this_len != other_len) return false;
  const char* thisData = (storage_.policy_ == duplicate) ? this->cstr_->data() : this->cstrNoDup_;
  const char* otherData = (other.storage_.policy_ == duplicate) ? other.cstr_->data() : other.cstrNoDup_;
  int comp = memcmp(thisData, otherData, this_len);

  return comp == 0;
}

template<typename _Traits, typename _Alloc>
ArrayIndex Value<_Traits, _Alloc>::CZString::index() const { return index_; }

//const char* Value<_Traits, _Alloc>::CZString::c_str() const { return cstr_; }
template<typename _Traits, typename _Alloc>
const char* Value<_Traits, _Alloc>::CZString::data() const {
  if (storage_.policy_ == duplicate) {
	if (!cstr_)
		return nullptr;
    return cstr_->data();
  } else {
    return cstrNoDup_;
  }
}

template<typename _Traits, typename _Alloc>
unsigned Value<_Traits, _Alloc>::CZString::length() const { return storage_.length_; }
template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::CZString::isStaticString() const { return storage_.policy_ == noDuplication; }

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Value<_Traits, _Alloc>::Value
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

/*! \internal Default constructor initialization must be equivalent to:
 * memset( this, 0, sizeof(Value<_Traits, _Alloc>) )
 * This optimization is used in ValueInternalMap fast allocator.
 */
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(ValueType vtype) {
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
    value_.stringRaw_ = nullptr;
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

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(Int value) {
  initBasic(intValue);
  value_.int_ = value;
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(UInt value) {
  initBasic(uintValue);
  value_.uint_ = value;
}
#if defined(JSON_HAS_INT64)
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(Int64 value) {
  initBasic(intValue);
  value_.int_ = value;
}
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(UInt64 value) {
  initBasic(uintValue);
  value_.uint_ = value;
}
#endif // defined(JSON_HAS_INT64)

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(double value) {
  initBasic(realValue);
  value_.real_ = value;
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(const char* value) {
  initBasic(stringValue, true);
  value_.stringDuplicate_ = duplicateAndPrefixStringValue(value, static_cast<unsigned>(strlen(value)));
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(const char* beginValue, const char* endValue) {
  initBasic(stringValue, true);
  value_.stringDuplicate_ =
      duplicateAndPrefixStringValue(beginValue, static_cast<unsigned>(endValue - beginValue));
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(const std::basic_string<typename Value<_Traits, _Alloc>::value_type, typename Value<_Traits, _Alloc>::traits_type, typename Value<_Traits, _Alloc>::allocator_type>& value) {
  initBasic(stringValue, true);
  value_.stringDuplicate_ =
      duplicateAndPrefixStringValue(value.data(), static_cast<unsigned>(value.length()));
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(const StaticString& value) {
  initBasic(stringValue);
  value_.stringRaw_ = const_cast<char*>(value.c_str());
}

#ifdef JSON_USE_CPPTL
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(const CppTL::ConstString& value) {
  initBasic(stringValue, true);
  value_.string_ = duplicateAndPrefixStringValue(value, static_cast<unsigned>(value.length()));
}
#endif

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(bool value) {
  initBasic(booleanValue);
  value_.bool_ = value;
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(Value<_Traits, _Alloc> const& other)
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
    if (other.value_.stringDuplicate_ && other.allocated_) {
      unsigned len;
      char const* str;
	  decodePrefixedString(other.allocated_, other.value_.stringDuplicate_->data(), &len, &str);
      value_.stringDuplicate_ = duplicateAndPrefixStringValue(str, len);
      allocated_ = true;
    } else {
      value_.stringRaw_ = other.value_.stringRaw_;
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
      if (otherComment.comment_)
        comments_[comment].setComment(
            otherComment.comment_, strlen(otherComment.comment_->data()));
    }
  }
}

#if JSON_HAS_RVALUE_REFERENCES
// Move constructor
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::Value(Value<_Traits, _Alloc>&& other) {
  initBasic(nullValue);
  swap(other);
}
#endif

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>::~Value() {
  switch (type_) {
  case nullValue:
  case intValue:
  case uintValue:
  case realValue:
  case booleanValue:
    break;
  case stringValue:
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

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::operator=(Value<_Traits, _Alloc> other) {
  swap(other);
  return *this;
}

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::swapPayload(Value<_Traits, _Alloc>& other) {
  ValueType temp = type_;
  type_ = other.type_;
  other.type_ = temp;
  std::swap(value_, other.value_);
  int temp2 = allocated_;
  allocated_ = other.allocated_;
  other.allocated_ = temp2 & 0x1;
}

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::swap(Value<_Traits, _Alloc>& other) {
  swapPayload(other);
  std::swap(comments_, other.comments_);
  std::swap(start_, other.start_);
  std::swap(limit_, other.limit_);
}

template<typename _Traits, typename _Alloc>
ValueType Value<_Traits, _Alloc>::type() const { return type_; }

template<typename _Traits, typename _Alloc>
int Value<_Traits, _Alloc>::compare(const Value<_Traits, _Alloc>& other) const {
  if (*this < other)
    return -1;
  if (*this > other)
    return 1;
  return 0;
}

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::operator<(const Value<_Traits, _Alloc>& other) const {
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
	if (allocated_) {
	  if (!(value_.stringDuplicate_) || !(other.value_.stringDuplicate_)) {
		if (other.value_.stringDuplicate_) return true;
		else return false;
	  }
	} else {
	  if ((value_.stringRaw_ == nullptr) || (other.value_.stringRaw_ == nullptr)) {
	    if (other.value_.stringRaw_) return true;
	    else return false;
	  }
	}
    unsigned this_len;
    unsigned other_len;
    char const* this_str;
    char const* other_str;
    if (allocated_) {
      decodePrefixedString(this->allocated_, this->value_.stringDuplicate_->data(), &this_len, &this_str);
    } else {
      decodePrefixedString(this->allocated_, this->value_.stringRaw_, &this_len, &this_str);
    }
    if (other.allocated_) {
      decodePrefixedString(other.allocated_, other.value_.stringDuplicate_->data(), &other_len, &other_str);
    } else {
      decodePrefixedString(other.allocated_, other.value_.stringRaw_, &other_len, &other_str);
    }
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

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::operator<=(const Value<_Traits, _Alloc>& other) const { return !(other < *this); }

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::operator>=(const Value<_Traits, _Alloc>& other) const { return !(*this < other); }

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::operator>(const Value<_Traits, _Alloc>& other) const { return other < *this; }

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::operator==(const Value<_Traits, _Alloc>& other) const {
  // if ( type_ != other.type_ )
  // GCC 2.95.3 says:
  // attempt to take address of bit-field structure member `Json::Value<_Traits, _Alloc>::type_'
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
    if (allocated_) {
	  if (!(value_.stringDuplicate_) || !(other.value_.stringDuplicate_)) {
	    return (!(value_.stringDuplicate_) && !(other.value_.stringDuplicate_));
	  }
	} else {
	  if ((value_.stringRaw_ == nullptr) || (other.value_.stringRaw_ == nullptr)) {
		return value_.stringRaw_ == other.value_.stringRaw_;
	  }
	}
    unsigned this_len;
    unsigned other_len;
    char const* this_str;
    char const* other_str;
    if (allocated_) {
	  decodePrefixedString(this->allocated_, this->value_.stringDuplicate_->data(), &this_len, &this_str);
	} else {
	  decodePrefixedString(this->allocated_, this->value_.stringRaw_, &this_len, &this_str);
	}
	if (other.allocated_) {
	  decodePrefixedString(other.allocated_, other.value_.stringDuplicate_->data(), &other_len, &other_str);
	} else {
	  decodePrefixedString(other.allocated_, other.value_.stringRaw_, &other_len, &other_str);
	}
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

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::operator!=(const Value<_Traits, _Alloc>& other) const { return !(*this == other); }

template<typename _Traits, typename _Alloc>
const char* Value<_Traits, _Alloc>::asCString() const {
  JSON_ASSERT_MESSAGE(type_ == stringValue,
                      "in Json::Value<_Traits, _Alloc>::asCString(): requires stringValue");
  if (allocated_) {
    if (!(value_.stringDuplicate_)) {
      return nullptr;
    }
  } else {
    if (value_.stringRaw_ == nullptr) {
 	 return nullptr;
    }
  }
  unsigned this_len;
  char const* this_str;
  if (allocated_) {
  	decodePrefixedString(this->allocated_, this->value_.stringDuplicate_->data(), &this_len, &this_str);
  } else {
  	decodePrefixedString(this->allocated_, this->value_.stringRaw_, &this_len, &this_str);
  }
  return this_str;
}

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::getString(char const** str, char const** cend) const {
  if (type_ != stringValue) return false;
  if (allocated_) {
	if (!(value_.stringDuplicate_)) return false;
  } else {
	if (value_.stringRaw_ == nullptr) return false;
  }
  unsigned length;
  if (allocated_) {
    decodePrefixedString(this->allocated_, this->value_.stringDuplicate_->data(), &length, str);
  } else {
    decodePrefixedString(this->allocated_, this->value_.stringRaw_, &length, str);
  }
  *cend = *str + length;
  return true;
}

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::string_type Value<_Traits, _Alloc>::asString() const {
  switch (type_) {
  case nullValue:
    return string_type();
  case stringValue:
  {
    if (allocated_) {
	  if (!(value_.stringDuplicate_)) return string_type();
	} else {
	  if (value_.stringRaw_ == nullptr) return string_type();
	}
    unsigned this_len;
    char const* this_str;
    if (allocated_) {
      decodePrefixedString(this->allocated_, this->value_.stringDuplicate_->data(), &this_len, &this_str);
    } else {
      decodePrefixedString(this->allocated_, this->value_.stringRaw_, &this_len, &this_str);
    }
    return string_type(this_str, this_len);
  }
  case booleanValue:
    return value_.bool_ ? "true" : "false";
  case intValue:
    return valueToString<_Traits, _Alloc>(value_.int_);
  case uintValue:
    return valueToString<_Traits, _Alloc>(value_.uint_);
  case realValue:
    return valueToString<_Traits, _Alloc>(value_.real_);
  default:
    JSON_FAIL_MESSAGE("Type is not convertible to string");
  }
}

#ifdef JSON_USE_CPPTL
CppTL::ConstString Value<_Traits, _Alloc>::asConstString() const {
  unsigned len;
  char const* str;
  decodePrefixedString(allocated_, value_.string_,
      &len, &str);
  return CppTL::ConstString(str, len);
}
#endif

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::Int Value<_Traits, _Alloc>::asInt() const {
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
  JSON_FAIL_MESSAGE("Value<_Traits, _Alloc> is not convertible to Int.");
}

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::UInt Value<_Traits, _Alloc>::asUInt() const {
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
  JSON_FAIL_MESSAGE("Value<_Traits, _Alloc> is not convertible to UInt.");
}

#if defined(JSON_HAS_INT64)

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::Int64 Value<_Traits, _Alloc>::asInt64() const {
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
  JSON_FAIL_MESSAGE("Value<_Traits, _Alloc> is not convertible to Int64.");
}

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::UInt64 Value<_Traits, _Alloc>::asUInt64() const {
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
  JSON_FAIL_MESSAGE("Value<_Traits, _Alloc> is not convertible to UInt64.");
}
#endif // if defined(JSON_HAS_INT64)

template<typename _Traits, typename _Alloc>
LargestInt Value<_Traits, _Alloc>::asLargestInt() const {
#if defined(JSON_NO_INT64)
  return asInt();
#else
  return asInt64();
#endif
}

template<typename _Traits, typename _Alloc>
LargestUInt Value<_Traits, _Alloc>::asLargestUInt() const {
#if defined(JSON_NO_INT64)
  return asUInt();
#else
  return asUInt64();
#endif
}

template<typename _Traits, typename _Alloc>
double Value<_Traits, _Alloc>::asDouble() const {
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
  JSON_FAIL_MESSAGE("Value<_Traits, _Alloc> is not convertible to double.");
}

template<typename _Traits, typename _Alloc>
float Value<_Traits, _Alloc>::asFloat() const {
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
  JSON_FAIL_MESSAGE("Value<_Traits, _Alloc> is not convertible to float.");
}

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::asBool() const {
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
  JSON_FAIL_MESSAGE("Value<_Traits, _Alloc> is not convertible to bool.");
}

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isConvertibleTo(ValueType other) const {
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
template<typename _Traits, typename _Alloc>
ArrayIndex Value<_Traits, _Alloc>::size() const {
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

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::empty() const {
  if (isNull() || isArray() || isObject())
    return size() == 0u;
  else
    return false;
}

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::operator!() const { return isNull(); }

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::clear() {
  JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == arrayValue ||
                          type_ == objectValue,
                      "in Json::Value<_Traits, _Alloc>::clear(): requires complex value");
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

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::resize(ArrayIndex newSize) {
  JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == arrayValue,
                      "in Json::Value<_Traits, _Alloc>::resize(): requires arrayValue");
  if (type_ == nullValue)
    *this = Value<_Traits, _Alloc>(arrayValue);
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

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::operator[](ArrayIndex index) {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == arrayValue,
      "in Json::Value<_Traits, _Alloc>::operator[](ArrayIndex): requires arrayValue");
  if (type_ == nullValue)
    *this = Value<_Traits, _Alloc>(arrayValue);
  CZString key(index);
  typename ObjectValues::iterator it = value_.map_->lower_bound(key);
  if (it != value_.map_->end() && (*it).first == key)
    return (*it).second;

  typename ObjectValues::value_type defaultValue(key, nullRef);
  it = value_.map_->insert(it, defaultValue);
  return (*it).second;
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::operator[](int index) {
  JSON_ASSERT_MESSAGE(
      index >= 0,
      "in Json::Value<_Traits, _Alloc>::operator[](int index): index cannot be negative");
  return (*this)[ArrayIndex(index)];
}

template<typename _Traits, typename _Alloc>
const Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::operator[](ArrayIndex index) const {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == arrayValue,
      "in Json::Value<_Traits, _Alloc>::operator[](ArrayIndex)const: requires arrayValue");
  if (type_ == nullValue)
    return nullRef;
  CZString key(index);
  typename ObjectValues::const_iterator it = value_.map_->find(key);
  if (it == value_.map_->end())
    return nullRef;
  return (*it).second;
}

template<typename _Traits, typename _Alloc>
const Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::operator[](int index) const {
  JSON_ASSERT_MESSAGE(
      index >= 0,
      "in Json::Value<_Traits, _Alloc>::operator[](int index) const: index cannot be negative");
  return (*this)[ArrayIndex(index)];
}

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::initBasic(ValueType vtype, bool allocated) {
  type_ = vtype;
  allocated_ = allocated;
  comments_ = 0;
  start_ = 0;
  limit_ = 0;
}

// Access an object value by name, create a null member if it does not exist.
// @pre Type of '*this' is object or null.
// @param key is null-terminated.
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::resolveReference(const char* key) {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == objectValue,
      "in Json::Value<_Traits, _Alloc>::resolveReference(): requires objectValue");
  if (type_ == nullValue)
    *this = Value<_Traits, _Alloc>(objectValue);
  CZString actualKey(
      key, static_cast<unsigned>(strlen(key)), CZString::noDuplication); // NOTE!
  typename ObjectValues::iterator it = value_.map_->lower_bound(actualKey);
  if (it != value_.map_->end() && (*it).first == actualKey)
    return (*it).second;

  typename ObjectValues::value_type defaultValue(actualKey, nullRef);
  it = value_.map_->insert(it, defaultValue);
  Value<_Traits, _Alloc>& value = (*it).second;
  return value;
}

// @param key is not null-terminated.
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::resolveReference(char const* key, char const* cend)
{
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == objectValue,
      "in Json::Value<_Traits, _Alloc>::resolveReference(key, end): requires objectValue");
  if (type_ == nullValue)
    *this = Value<_Traits, _Alloc>(objectValue);
  CZString actualKey(
      key, static_cast<unsigned>(cend-key), CZString::duplicateOnCopy);
  typename ObjectValues::iterator it = value_.map_->lower_bound(actualKey);
  if (it != value_.map_->end() && (*it).first == actualKey)
    return (*it).second;

  typename ObjectValues::value_type defaultValue(actualKey, nullRef);
  it = value_.map_->insert(it, defaultValue);
  Value<_Traits, _Alloc>& value = (*it).second;
  return value;
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc> Value<_Traits, _Alloc>::get(ArrayIndex index, const Value<_Traits, _Alloc>& defaultValue) const {
  const Value<_Traits, _Alloc>* value = &((*this)[index]);
  return value == &nullRef ? defaultValue : *value;
}

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isValidIndex(ArrayIndex index) const { return index < size(); }

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc> const* Value<_Traits, _Alloc>::find(char const* key, char const* cend) const
{
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == objectValue,
      "in Json::Value<_Traits, _Alloc>::find(key, end, found): requires objectValue or nullValue");
  if (type_ == nullValue) return NULL;
  CZString actualKey(key, static_cast<unsigned>(cend-key), CZString::noDuplication);
  typename ObjectValues::const_iterator it = value_.map_->find(actualKey);
  if (it == value_.map_->end()) return NULL;
  return &(*it).second;
}
template<typename _Traits, typename _Alloc>
const Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::operator[](const char* key) const
{
  Value<_Traits, _Alloc> const* found = find(key, key + strlen(key));
  if (!found) return nullRef;
  return *found;
}
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc> const& Value<_Traits, _Alloc>::operator[](typename Value<_Traits, _Alloc>::string_type const& key) const
{
  Value<_Traits, _Alloc> const* found = find(key.data(), key.data() + key.length());
  if (!found) return nullRef;
  return *found;
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::operator[](const char* key) {
  return resolveReference(key, key + strlen(key));
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::operator[](const typename Value<_Traits, _Alloc>::string_type& key) {
  return resolveReference(key.data(), key.data() + key.length());
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::operator[](const StaticString& key) {
  return resolveReference(key.c_str());
}

#ifdef JSON_USE_CPPTL
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::operator[](const CppTL::ConstString& key) {
  return resolveReference(key.c_str(), key.end_c_str());
}
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc> const& Value<_Traits, _Alloc>::operator[](CppTL::ConstString const& key) const
{
  Value<_Traits, _Alloc> const* found = find(key.c_str(), key.end_c_str());
  if (!found) return nullRef;
  return *found;
}
#endif

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& Value<_Traits, _Alloc>::append(const Value<_Traits, _Alloc>& value) { return (*this)[size()] = value; }

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc> Value<_Traits, _Alloc>::get(char const* key, char const* cend, Value<_Traits, _Alloc> const& defaultValue) const
{
  Value<_Traits, _Alloc> const* found = find(key, cend);
  return !found ? defaultValue : *found;
}
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc> Value<_Traits, _Alloc>::get(char const* key, Value<_Traits, _Alloc> const& defaultValue) const
{
  return get(key, key + strlen(key), defaultValue);
}
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc> Value<_Traits, _Alloc>::get(typename Value<_Traits, _Alloc>::string_type const& key, Value<_Traits, _Alloc> const& defaultValue) const
{
  return get(key.data(), key.data() + key.length(), defaultValue);
}


template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::removeMember(const char* key, const char* cend, Value<_Traits, _Alloc>* removed)
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
template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::removeMember(const char* key, Value<_Traits, _Alloc>* removed)
{
  return removeMember(key, key + strlen(key), removed);
}
template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::removeMember(typename Value<_Traits, _Alloc>::string_type const& key, Value<_Traits, _Alloc>* removed)
{
  return removeMember(key.data(), key.data() + key.length(), removed);
}
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc> Value<_Traits, _Alloc>::removeMember(const char* key)
{
  JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == objectValue,
                      "in Json::Value<_Traits, _Alloc>::removeMember(): requires objectValue");
  if (type_ == nullValue)
    return nullRef;

  Value<_Traits, _Alloc> removed;  // null
  removeMember(key, key + strlen(key), &removed);
  return removed; // still null if removeMember() did nothing
}
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc> Value<_Traits, _Alloc>::removeMember(const typename Value<_Traits, _Alloc>::string_type& key)
{
  return removeMember(key.c_str());
}

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::removeIndex(ArrayIndex index, Value<_Traits, _Alloc>* removed) {
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
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc> Value<_Traits, _Alloc>::get(const CppTL::ConstString& key,
                 const Value<_Traits, _Alloc>& defaultValue) const {
  return get(key.c_str(), key.end_c_str(), defaultValue);
}
#endif

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isMember(char const* key, char const* cend) const
{
  Value<_Traits, _Alloc> const* value = find(key, cend);
  return NULL != value;
}
template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isMember(char const* key) const
{
  return isMember(key, key + strlen(key));
}
template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isMember(typename Value<_Traits, _Alloc>::string_type const& key) const
{
  return isMember(key.data(), key.data() + key.length());
}

#ifdef JSON_USE_CPPTL
template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isMember(const CppTL::ConstString& key) const {
  return isMember(key.c_str(), key.end_c_str());
}
#endif

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::Members Value<_Traits, _Alloc>::getMemberNames() const {
  JSON_ASSERT_MESSAGE(
      type_ == nullValue || type_ == objectValue,
      "in Json::Value<_Traits, _Alloc>::getMemberNames(), value must be objectValue");
  if (type_ == nullValue)
    return Value<_Traits, _Alloc>::Members();
  Members members;
  members.reserve(value_.map_->size());
  typename ObjectValues::const_iterator it = value_.map_->begin();
  typename ObjectValues::const_iterator itEnd = value_.map_->end();
  for (; it != itEnd; ++it) {
    members.push_back(string_type((*it).first.data(),
                                  (*it).first.length()));
  }
  return members;
}
//
//# ifdef JSON_USE_CPPTL
// EnumMemberNames
// Value<_Traits, _Alloc>::enumMemberNames() const
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
// Value<_Traits, _Alloc>::enumValues() const
//{
//   if ( type_ == objectValue  ||  type_ == arrayValue )
//      return CppTL::Enum::anyValues( *(value_.map_),
//                                     CppTL::Type<const Value<_Traits, _Alloc> &>() );
//   return EnumValues();
//}
//
//# endif

static bool IsIntegral(double d) {
  double integral_part;
  return modf(d, &integral_part) == 0.0;
}

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isNull() const { return type_ == nullValue; }

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isBool() const { return type_ == booleanValue; }

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isInt() const {
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

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isUInt() const {
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

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isInt64() const {
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

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isUInt64() const {
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

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isIntegral() const {
#if defined(JSON_HAS_INT64)
  return isInt64() || isUInt64();
#else
  return isInt() || isUInt();
#endif
}

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isDouble() const { return type_ == realValue || isIntegral(); }

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isNumeric() const { return isIntegral() || isDouble(); }

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isString() const { return type_ == stringValue; }

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isArray() const { return type_ == arrayValue; }

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::isObject() const { return type_ == objectValue; }

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::setComment(const char* comment, size_t len, CommentPlacement placement) {
  if (!comments_)
    comments_ = new CommentInfo[numberOfCommentPlacement];
  if ((len > 0) && (comment[len-1] == '\n')) {
    // Always discard trailing newline, to aid indentation.
    len -= 1;
  }
  comments_[placement].setComment(comment, len);
}

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::setComment(const char* comment, CommentPlacement placement) {
  setComment(comment, strlen(comment), placement);
}

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::setComment(const string_type& comment, CommentPlacement placement) {
  setComment(comment.c_str(), comment.length(), placement);
}

template<typename _Traits, typename _Alloc>
bool Value<_Traits, _Alloc>::hasComment(CommentPlacement placement) const {
  return comments_ != 0 && comments_[placement].comment_;
}

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::string_type Value<_Traits, _Alloc>::getComment(CommentPlacement placement) const {
  if (hasComment(placement))
    return Value<_Traits, _Alloc>::string_type(comments_[placement].comment_->begin(), comments_[placement].comment_->end());
  return "";
}

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::setOffsetStart(size_t start) { start_ = start; }

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::setOffsetLimit(size_t limit) { limit_ = limit; }

template<typename _Traits, typename _Alloc>
size_t Value<_Traits, _Alloc>::getOffsetStart() const { return start_; }

template<typename _Traits, typename _Alloc>
size_t Value<_Traits, _Alloc>::getOffsetLimit() const { return limit_; }

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::string_type Value<_Traits, _Alloc>::toStyledString() const {
  StyledWriter<_Traits, _Alloc> writer;
  return writer.write(*this);
}

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::const_iterator Value<_Traits, _Alloc>::begin() const {
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

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::const_iterator Value<_Traits, _Alloc>::end() const {
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

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::iterator Value<_Traits, _Alloc>::begin() {
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

template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::iterator Value<_Traits, _Alloc>::end() {
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

/** Duplicates the specified string value.
 * @param value Pointer to the string to duplicate. Must be zero-terminated if
 *              length is "unknown".
 * @param length Length of the value. if equals to unknown, then it will be
 *               computed using strlen(value).
 * @return Pointer on the duplicate instance of string.
 */
template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::string_data_type Value<_Traits, _Alloc>::duplicateStringValue(const char* value, size_t length) {
  // Avoid an integer overflow in the call to malloc below by limiting length
  // to a sane value.
  if (length >= (size_t)Value::maxInt)
    length = Value::maxInt - 1;

  Value<_Traits, _Alloc>::string_data_type newString(new string_data_type_in_ptr(value, value + length));
  newString->push_back(0);
  return newString;
}

/* Record the length as a prefix.
 */
template<typename _Traits, typename _Alloc>
typename Value<_Traits, _Alloc>::string_data_type Value<_Traits, _Alloc>::duplicateAndPrefixStringValue(
    const char* value,
    unsigned int length)
{
  // Avoid an integer overflow in the call to malloc below by limiting length
  // to a sane value.
  JSON_ASSERT_MESSAGE(length <= (unsigned)Value::maxInt - sizeof(unsigned) - 1U,
                      "in Json::Value::duplicateAndPrefixStringValue(): "
                      "length too big for prefixing");
  unsigned actualLength = length + static_cast<unsigned>(sizeof(unsigned)) + 1U;
  string_data_type newString = string_data_type(new string_data_type_in_ptr());
  try {
	  newString->reserve(actualLength);
  } catch (...) {
	  throwRuntimeError(
		  "in Json::Value::duplicateAndPrefixStringValue(): "
		  "Failed to allocate string value buffer");
  }
  newString->data()[actualLength-1] = 0; //Prevent overflow later.
  for (unsigned int i=0; i<sizeof(unsigned); i++)
	  newString->push_back(reinterpret_cast<char*>(&length)[i]);
  newString->insert(newString->end(), value, value+length);
  newString->push_back(static_cast<value_type>(0));

  return newString;
}

template<typename _Traits, typename _Alloc>
void Value<_Traits, _Alloc>::decodePrefixedString(
    bool isPrefixed, const char* prefixed,
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

// class ValueHolder
// //////////////////////////////////////////////////////////////////

template<typename _Traits, typename _Alloc>
ValueHolder<_Traits, _Alloc>::ValueHolder() { /* Not used */ } //Now required for strings in the union
template<typename _Traits, typename _Alloc>
ValueHolder<_Traits, _Alloc>::ValueHolder(const ValueHolder& ref) {
  copy(ref);
}
template<typename _Traits, typename _Alloc>
ValueHolder<_Traits, _Alloc>::ValueHolder(ValueHolder&& ref) {
  swap(std::move(ref));
}
template<typename _Traits, typename _Alloc>
ValueHolder<_Traits, _Alloc>::~ValueHolder() { /* Not used */ }

template<typename _Traits, typename _Alloc>
ValueHolder<_Traits, _Alloc>& ValueHolder<_Traits, _Alloc>::operator=(const ValueHolder<_Traits, _Alloc>& value) {
  copy(value);
  return *this;
}

template<typename _Traits, typename _Alloc>
ValueHolder<_Traits, _Alloc>& ValueHolder<_Traits, _Alloc>::operator=(ValueHolder<_Traits, _Alloc>&& value) {
  swap(std::move(value));
  return *this;
}

template<typename _Traits, typename _Alloc>
void ValueHolder<_Traits, _Alloc>::copy(const ValueHolder& value) {
  int_=value.int_;
  uint_=value.uint_;
  real_=value.real_;
  bool_=value.bool_;
  stringRaw_=value.stringRaw_;
  if (value.stringDuplicate_) {
    stringDuplicate_=typename Value<_Traits, _Alloc>::string_data_type(new typename Value<_Traits, _Alloc>::string_data_type_in_ptr(*(value.stringDuplicate_)));
  }
  map_=value.map_;
}

template<typename _Traits, typename _Alloc>
void ValueHolder<_Traits, _Alloc>::swap(ValueHolder&& value) {
  int_=value.int_;
  value.int_=0;
  uint_=value.uint_;
  value.uint_=0;
  real_=value.real_;
  value.real_=0;
  bool_=value.bool_;
  value.bool_=false;
  stringRaw_=value.stringRaw_;
  value.stringRaw_=nullptr;
  std::swap(stringDuplicate_, value.stringDuplicate_);
  map_=value.map_;
  value.map_=nullptr;
}

// class PathArgument
// //////////////////////////////////////////////////////////////////

template<typename _Traits, typename _Alloc>
PathArgument<_Traits, _Alloc>::PathArgument() : key_(), index_(), kind_(kindNone) {}

template<typename _Traits, typename _Alloc>
PathArgument<_Traits, _Alloc>::PathArgument(ArrayIndex index)
    : key_(), index_(index), kind_(kindIndex) {}

template<typename _Traits, typename _Alloc>
PathArgument<_Traits, _Alloc>::PathArgument(const char* key)
    : key_(key), index_(), kind_(kindKey) {}

template<typename _Traits, typename _Alloc>
PathArgument<_Traits, _Alloc>::PathArgument(const std::string& key)
    : key_(key.c_str()), index_(), kind_(kindKey) {}


// class Path
// //////////////////////////////////////////////////////////////////

template<typename _Traits, typename _Alloc>
Path<_Traits, _Alloc>::Path(const std::string& path,
           const PathArgument<_Traits, _Alloc>& a1,
           const PathArgument<_Traits, _Alloc>& a2,
           const PathArgument<_Traits, _Alloc>& a3,
           const PathArgument<_Traits, _Alloc>& a4,
           const PathArgument<_Traits, _Alloc>& a5) {
  InArgs in;
  in.push_back(&a1);
  in.push_back(&a2);
  in.push_back(&a3);
  in.push_back(&a4);
  in.push_back(&a5);
  makePath(path, in);
}

template<typename _Traits, typename _Alloc>
void Path<_Traits, _Alloc>::makePath(const std::string& path, const InArgs& in) {
  const char* current = path.c_str();
  const char* end = current + path.length();
  typename InArgs::const_iterator itInArg = in.begin();
  while (current != end) {
    if (*current == '[') {
      ++current;
      if (*current == '%')
        addPathInArg(path, in, itInArg, PathArgument<_Traits, _Alloc>::kindIndex);
      else {
        ArrayIndex index = 0;
        for (; current != end && *current >= '0' && *current <= '9'; ++current)
          index = index * 10 + ArrayIndex(*current - '0');
        args_.push_back(index);
      }
      if (current == end || *current++ != ']')
        invalidPath(path, int(current - path.c_str()));
    } else if (*current == '%') {
      addPathInArg(path, in, itInArg, PathArgument<_Traits, _Alloc>::kindKey);
      ++current;
    } else if (*current == '.') {
      ++current;
    } else {
      const char* beginName = current;
      while (current != end && !strchr("[.", *current))
        ++current;
      args_.push_back(std::string(beginName, current));
    }
  }
}

template<typename _Traits, typename _Alloc>
void Path<_Traits, _Alloc>::addPathInArg(const std::string& /*path*/,
                        const InArgs& in,
						typename InArgs::const_iterator& itInArg,
						typename PathArgument<_Traits, _Alloc>::Kind kind) {
  if (itInArg == in.end()) {
    // Error: missing argument %d
  } else if ((*itInArg)->kind_ != kind) {
    // Error: bad argument type
  } else {
    args_.push_back(**itInArg);
  }
}

template<typename _Traits, typename _Alloc>
void Path<_Traits, _Alloc>::invalidPath(const std::string& /*path*/, int /*location*/) {
  // Error: invalid path.
}

template<typename _Traits, typename _Alloc>
const Value<_Traits, _Alloc>& Path<_Traits, _Alloc>::resolve(const Value<_Traits, _Alloc>& root) const {
  const Value<_Traits, _Alloc>* node = &root;
  for (typename Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
    const PathArgument<_Traits, _Alloc>& arg = *it;
    if (arg.kind_ == PathArgument<_Traits, _Alloc>::kindIndex) {
      if (!node->isArray() || !node->isValidIndex(arg.index_)) {
        // Error: unable to resolve path (array value expected at position...
      }
      node = &((*node)[arg.index_]);
    } else if (arg.kind_ == PathArgument<_Traits, _Alloc>::kindKey) {
      if (!node->isObject()) {
        // Error: unable to resolve path (object value expected at position...)
      }
      node = &((*node)[arg.key_]);
      if (node == &Value<_Traits, _Alloc>::nullRef) {
        // Error: unable to resolve path (object has no member named '' at
        // position...)
      }
    }
  }
  return *node;
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc> Path<_Traits, _Alloc>::resolve(const Value<_Traits, _Alloc>& root, const Value<_Traits, _Alloc>& defaultValue) const {
  const Value<_Traits, _Alloc>* node = &root;
  for (typename Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
    const PathArgument<_Traits, _Alloc>& arg = *it;
    if (arg.kind_ == PathArgument<_Traits, _Alloc>::kindIndex) {
      if (!node->isArray() || !node->isValidIndex(arg.index_))
        return defaultValue;
      node = &((*node)[arg.index_]);
    } else if (arg.kind_ == PathArgument<_Traits, _Alloc>::kindKey) {
      if (!node->isObject())
        return defaultValue;
      node = &((*node)[arg.key_]);
      if (node == &Value<_Traits, _Alloc>::nullRef)
        return defaultValue;
    }
  }
  return *node;
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& Path<_Traits, _Alloc>::make(Value<_Traits, _Alloc>& root) const {
  Value<_Traits, _Alloc>* node = &root;
  for (typename Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
    const PathArgument<_Traits, _Alloc>& arg = *it;
    if (arg.kind_ == PathArgument<_Traits, _Alloc>::kindIndex) {
      if (!node->isArray()) {
        // Error: node is not an array at position ...
      }
      node = &((*node)[arg.index_]);
    } else if (arg.kind_ == PathArgument<_Traits, _Alloc>::kindKey) {
      if (!node->isObject()) {
        // Error: node is not an object at position...
      }
      node = &((*node)[arg.key_]);
    }
  }
  return *node;
}

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueIteratorBase
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

template<typename _Traits, typename _Alloc>
ValueIteratorBase<_Traits, _Alloc>::ValueIteratorBase()
    : current_(), isNull_(true) {
}

template<typename _Traits, typename _Alloc>
ValueIteratorBase<_Traits, _Alloc>::ValueIteratorBase(
    const typename Value<_Traits, _Alloc>::ObjectValues::iterator& current)
    : current_(current), isNull_(false) {}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& ValueIteratorBase<_Traits, _Alloc>::deref() const {
  return current_->second;
}

template<typename _Traits, typename _Alloc>
void ValueIteratorBase<_Traits, _Alloc>::increment() {
  ++current_;
}

template<typename _Traits, typename _Alloc>
void ValueIteratorBase<_Traits, _Alloc>::decrement() {
  --current_;
}

template<typename _Traits, typename _Alloc>
typename ValueIteratorBase<_Traits, _Alloc>::difference_type
ValueIteratorBase<_Traits, _Alloc>::computeDistance(const SelfType& other) const {
#ifdef JSON_USE_CPPTL_SMALLMAP
  return other.current_ - current_;
#else
  // Iterator for null value are initialized using the default
  // constructor, which initialize current_ to the default
  // std::map::iterator. As begin() and end() are two instance
  // of the default std::map::iterator, they can not be compared.
  // To allow this, we handle this comparison specifically.
  if (isNull_ && other.isNull_) {
    return 0;
  }

  // Usage of std::distance is not portable (does not compile with Sun Studio 12
  // RogueWave STL,
  // which is the one used by default).
  // Using a portable hand-made version for non random iterator instead:
  //   return difference_type( std::distance( current_, other.current_ ) );
  difference_type myDistance = 0;
  for (typename Value<_Traits, _Alloc>::ObjectValues::iterator it = current_; it != other.current_;
       ++it) {
    ++myDistance;
  }
  return myDistance;
#endif
}

template<typename _Traits, typename _Alloc>
bool ValueIteratorBase<_Traits, _Alloc>::isEqual(const SelfType& other) const {
  if (isNull_) {
    return other.isNull_;
  }
  return current_ == other.current_;
}

template<typename _Traits, typename _Alloc>
void ValueIteratorBase<_Traits, _Alloc>::copy(const SelfType& other) {
  current_ = other.current_;
  isNull_ = other.isNull_;
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc> ValueIteratorBase<_Traits, _Alloc>::key() const {
  const typename Value<_Traits, _Alloc>::CZString czstring = (*current_).first;
  if (czstring.data()) {
    if (czstring.isStaticString())
      return Value<_Traits, _Alloc>(StaticString(czstring.data()));
    return Value<_Traits, _Alloc>(czstring.data(), czstring.data() + czstring.length());
  }
  return Value<_Traits, _Alloc>(czstring.index());
}

template<typename _Traits, typename _Alloc>
UInt ValueIteratorBase<_Traits, _Alloc>::index() const {
  const typename Value<_Traits, _Alloc>::CZString czstring = (*current_).first;
  if (!czstring.data())
    return czstring.index();
  return typename Value<_Traits, _Alloc>::UInt(-1);
}

template<typename _Traits, typename _Alloc>
std::string ValueIteratorBase<_Traits, _Alloc>::name() const {
  char const* keey;
  char const* end;
  keey = memberName(&end);
  if (!keey) return std::string();
  return std::string(keey, end);
}

template<typename _Traits, typename _Alloc>
char const* ValueIteratorBase<_Traits, _Alloc>::memberName() const {
  const char* cname = (*current_).first.data();
  return cname ? cname : "";
}

template<typename _Traits, typename _Alloc>
char const* ValueIteratorBase<_Traits, _Alloc>::memberName(char const** end) const {
  const char* cname = (*current_).first.data();
  if (!cname) {
    *end = NULL;
    return NULL;
  }
  *end = cname + (*current_).first.length();
  return cname;
}

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueConstIterator
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

template<typename _Traits, typename _Alloc>
ValueConstIterator<_Traits, _Alloc>::ValueConstIterator() {}

template<typename _Traits, typename _Alloc>
ValueConstIterator<_Traits, _Alloc>::ValueConstIterator(
    const typename Value<_Traits, _Alloc>::ObjectValues::iterator& current)
    : ValueIteratorBase<_Traits, _Alloc>(current) {}

template<typename _Traits, typename _Alloc>
ValueConstIterator<_Traits, _Alloc>::ValueConstIterator(ValueIterator<_Traits, _Alloc> const& other)
    : ValueIteratorBase<_Traits, _Alloc>(other) {}

template<typename _Traits, typename _Alloc>
ValueConstIterator<_Traits, _Alloc>& ValueConstIterator<_Traits, _Alloc>::
operator=(const ValueIteratorBase<_Traits, _Alloc>& other) {
  copy(other);
  return *this;
}

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueIterator
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

template<typename _Traits, typename _Alloc>
ValueIterator<_Traits, _Alloc>::ValueIterator() {}

template<typename _Traits, typename _Alloc>
ValueIterator<_Traits, _Alloc>::ValueIterator(const typename Value<_Traits, _Alloc>::ObjectValues::iterator& current)
    : ValueIteratorBase<_Traits, _Alloc>(current) {}

template<typename _Traits, typename _Alloc>
ValueIterator<_Traits, _Alloc>::ValueIterator(const ValueConstIterator<_Traits, _Alloc>& other)
    : ValueIteratorBase<_Traits, _Alloc>(other) {
  throwRuntimeError("ConstIterator to Iterator should never be allowed.");
}

template<typename _Traits, typename _Alloc>
ValueIterator<_Traits, _Alloc>::ValueIterator(const ValueIterator<_Traits, _Alloc>& other)
    : ValueIteratorBase<_Traits, _Alloc>(other) {}

template<typename _Traits, typename _Alloc>
ValueIterator<_Traits, _Alloc>& ValueIterator<_Traits, _Alloc>::operator=(const SelfType& other) {
  copy(other);
  return *this;
}

} // namespace Json


namespace std {
/// Specialize std::swap() for Json::Value.
template<typename _Traits, typename _Alloc>
inline void swap(Json::Value<_Traits, _Alloc>& a, Json::Value<_Traits, _Alloc>& b) { a.swap(b); }
}


#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(pop)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

#endif // CPPTL_JSON_H_INCLUDED
