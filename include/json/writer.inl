// Copyright 2011 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSON_WRITER_INL_INCLUDED
#define JSON_WRITER_INL_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include <json/writer.h>
#include "tool.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <iomanip>
#include <memory>
#include <sstream>
#include <utility>
#include <set>
#include <cassert>
#include <cstring>
#include <cstdio>

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
#if !defined(__MINGW32__)
#define snprintf std::snprintf
#endif
#endif

#if defined(__BORLANDC__)  
#include <float.h>
#define snprintf _snprintf
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400 // VC++ 8.0
// Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif

namespace Json {
namespace detail {

bool containsControlCharacter(const char* str);
bool containsControlCharacter0(const char* str, unsigned len);
// https://github.com/upcaste/upcaste/blob/master/src/upcore/src/cstring/strnpbrk.cpp
char const* strnpbrk(char const* s, char const* accept, size_t n);

template<class _Value>
typename _Value::String valueToString(LargestInt value) {
  UIntToStringBuffer buffer;
  char* current = buffer + sizeof(buffer);
  if (value == _Value::minLargestInt) {
    uintToString(LargestUInt(_Value::maxLargestInt) + 1, current);
    *--current = '-';
  } else if (value < 0) {
    uintToString(LargestUInt(-value), current);
    *--current = '-';
  } else {
    uintToString(LargestUInt(value), current);
  }
  assert(current >= buffer);
  return current;
}

template<class _Value>
typename _Value::String valueToString(LargestUInt value) {
  UIntToStringBuffer buffer;
  char* current = buffer + sizeof(buffer);
  uintToString(value, current);
  assert(current >= buffer);
  return current;
}

#if defined(JSON_HAS_INT64)

template<class _Value>
typename _Value::String valueToString(Int value) {
  return valueToString<_Value>(LargestInt(value));
}

template<class _Value>
typename _Value::String valueToString(UInt value) {
  return valueToString<_Value>(LargestUInt(value));
}

#endif // # if defined(JSON_HAS_INT64)

template<class _Value>
typename _Value::String valueToString(double value, bool useSpecialFloats, unsigned int precision) {
  // Allocate a buffer that is more than large enough to store the 16 digits of
  // precision requested below.
  char buffer[32];
  int len = -1;

  char formatString[6];
  sprintf(formatString, "%%.%dg", precision);

  // Print into the buffer. We need not request the alternative representation
  // that always has a decimal point because JSON doesn't distingish the
  // concepts of reals and integers.
  if (jsonIsFinite(value)) {
    len = snprintf(buffer, sizeof(buffer), formatString, value);
  } else {
    // IEEE standard states that NaN values will not compare to themselves
    if (value != value) {
      len = snprintf(buffer, sizeof(buffer), useSpecialFloats ? "NaN" : "null");
    } else if (value < 0) {
      len = snprintf(buffer, sizeof(buffer), useSpecialFloats ? "-Infinity" : "-1e+9999");
    } else {
      len = snprintf(buffer, sizeof(buffer), useSpecialFloats ? "Infinity" : "1e+9999");
    }
    // For those, we do not need to call fixNumLoc, but it is fast.
  }
  assert(len >= 0);
  fixNumericLocale(buffer, buffer + len);
  return buffer;
}

template<class _Value>
typename _Value::String valueToString(double value) { return valueToString<_Value>(value, false, 17); }

template<class _Value>
typename _Value::String valueToString(bool value) { return value ? "true" : "false"; }

template<class _Value>
typename _Value::String valueToQuotedString(const char* value) {
  using String = typename _Value::String;
  if (value == NULL)
    return "";
  // Not sure how to handle unicode...
  if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL &&
      !containsControlCharacter(value))
    return String("\"") + value + "\"";
  // We have to walk value and escape any special characters.
  // Appending to std::string is not efficient, but this should be rare.
  // (Note: forward slashes are *not* rare, but I am not escaping them.)
  typename String::size_type maxsize =
      strlen(value) * 2 + 3; // allescaped+quotes+NULL
  String result;
  result.reserve(maxsize); // to avoid lots of mallocs
  result += "\"";
  for (const char* c = value; *c != 0; ++c) {
    switch (*c) {
    case '\"':
      result += "\\\"";
      break;
    case '\\':
      result += "\\\\";
      break;
    case '\b':
      result += "\\b";
      break;
    case '\f':
      result += "\\f";
      break;
    case '\n':
      result += "\\n";
      break;
    case '\r':
      result += "\\r";
      break;
    case '\t':
      result += "\\t";
      break;
    // case '/':
    // Even though \/ is considered a legal escape in JSON, a bare
    // slash is also legal, so I see no reason to escape it.
    // (I hope I am not misunderstanding something.
    // blep notes: actually escaping \/ may be useful in javascript to avoid </
    // sequence.
    // Should add a flag to allow this compatibility mode and prevent this
    // sequence from occurring.
    default:
      if (isControlCharacter(*c)) {
        std::ostringstream oss;
        oss << "\\u" << std::hex << std::uppercase << std::setfill('0')
            << std::setw(4) << static_cast<int>(*c);
        result += oss.str();
      } else {
        result += *c;
      }
      break;
    }
  }
  result += "\"";
  return result;
}

template<class _Value>
static typename _Value::String valueToQuotedStringN(const char* value, unsigned length) {
  using String = typename _Value::String;
  if (value == NULL)
    return "";
  // Not sure how to handle unicode...
  if (strnpbrk(value, "\"\\\b\f\n\r\t", length) == NULL &&
      !containsControlCharacter0(value, length))
    return String("\"") + value + "\"";
  // We have to walk value and escape any special characters.
  // Appending to std::string is not efficient, but this should be rare.
  // (Note: forward slashes are *not* rare, but I am not escaping them.)
  typename String::size_type maxsize =
      length * 2 + 3; // allescaped+quotes+NULL
  String result;
  result.reserve(maxsize); // to avoid lots of mallocs
  result += "\"";
  char const* end = value + length;
  for (const char* c = value; c != end; ++c) {
    switch (*c) {
    case '\"':
      result += "\\\"";
      break;
    case '\\':
      result += "\\\\";
      break;
    case '\b':
      result += "\\b";
      break;
    case '\f':
      result += "\\f";
      break;
    case '\n':
      result += "\\n";
      break;
    case '\r':
      result += "\\r";
      break;
    case '\t':
      result += "\\t";
      break;
    // case '/':
    // Even though \/ is considered a legal escape in JSON, a bare
    // slash is also legal, so I see no reason to escape it.
    // (I hope I am not misunderstanding something.)
    // blep notes: actually escaping \/ may be useful in javascript to avoid </
    // sequence.
    // Should add a flag to allow this compatibility mode and prevent this
    // sequence from occurring.
    default:
      if ((isControlCharacter(*c)) || (*c == 0)) {
        std::ostringstream oss;
        oss << "\\u" << std::hex << std::uppercase << std::setfill('0')
            << std::setw(4) << static_cast<int>(*c);
        result += oss.str();
      } else {
        result += *c;
      }
      break;
    }
  }
  result += "\"";
  return result;
}

// Class Writer
// //////////////////////////////////////////////////////////////////
template<class _Value>
Writer<_Value>::~Writer() {}

// Class FastWriter
// //////////////////////////////////////////////////////////////////

template<class _Value>
FastWriter<_Value>::FastWriter()
    : yamlCompatiblityEnabled_(false), dropNullPlaceholders_(false),
      omitEndingLineFeed_(false) {}

template<class _Value>
void FastWriter<_Value>::enableYAMLCompatibility() { yamlCompatiblityEnabled_ = true; }

template<class _Value>
void FastWriter<_Value>::dropNullPlaceholders() { dropNullPlaceholders_ = true; }

template<class _Value>
void FastWriter<_Value>::omitEndingLineFeed() { omitEndingLineFeed_ = true; }

template<class _Value>
typename FastWriter<_Value>::String FastWriter<_Value>::write(const _Value& root) {
  document_ = "";
  writeValue(root);
  if (!omitEndingLineFeed_)
    document_ += "\n";
  return document_;
}

template<class _Value>
void FastWriter<_Value>::writeValue(const _Value& value) {
  switch (value.type()) {
  case nullValue:
    if (!dropNullPlaceholders_)
      document_ += "null";
    break;
  case intValue:
    document_ += valueToString<_Value>(value.asLargestInt());
    break;
  case uintValue:
    document_ += valueToString<_Value>(value.asLargestUInt());
    break;
  case realValue:
    document_ += valueToString<_Value>(value.asDouble());
    break;
  case stringValue:
  {
    // Is NULL possible for value.string_?
    char const* str;
    char const* end;
    bool ok = value.getString(&str, &end);
    if (ok) document_ += valueToQuotedStringN<_Value>(str, static_cast<unsigned>(end-str));
    break;
  }
  case booleanValue:
    document_ += valueToString<_Value>(value.asBool());
    break;
  case arrayValue: {
    document_ += '[';
    ArrayIndex size = value.size();
    for (ArrayIndex index = 0; index < size; ++index) {
      if (index > 0)
        document_ += ',';
      writeValue(value[index]);
    }
    document_ += ']';
  } break;
  case objectValue: {
    typename _Value::Members members(value.getMemberNames());
    document_ += '{';
    for (typename _Value::Members::iterator it = members.begin(); it != members.end();
         ++it) {
      const String& name = *it;
      if (it != members.begin())
        document_ += ',';
      document_ += valueToQuotedStringN<_Value>(name.data(), static_cast<unsigned>(name.length()));
      document_ += yamlCompatiblityEnabled_ ? ": " : ":";
      writeValue(value[name]);
    }
    document_ += '}';
  } break;
  }
}

// Class StyledWriter
// //////////////////////////////////////////////////////////////////

template<class _Value>
StyledWriter<_Value>::StyledWriter()
    : rightMargin_(74), indentSize_(3), addChildValues_() {}

template<class _Value>
typename StyledWriter<_Value>::String StyledWriter<_Value>::write(const _Value& root) {
  document_ = "";
  addChildValues_ = false;
  indentString_ = "";
  writeCommentBeforeValue(root);
  writeValue(root);
  writeCommentAfterValueOnSameLine(root);
  document_ += "\n";
  return document_;
}

template<class _Value>
void StyledWriter<_Value>::writeValue(const _Value& value) {
  switch (value.type()) {
  case nullValue:
    pushValue("null");
    break;
  case intValue:
    pushValue(valueToString<_Value>(value.asLargestInt()));
    break;
  case uintValue:
    pushValue(valueToString<_Value>(value.asLargestUInt()));
    break;
  case realValue:
    pushValue(valueToString<_Value>(value.asDouble()));
    break;
  case stringValue:
  {
    // Is NULL possible for value.string_?
    char const* str;
    char const* end;
    bool ok = value.getString(&str, &end);
    if (ok) pushValue(valueToQuotedStringN<_Value>(str, static_cast<unsigned>(end-str)));
    else pushValue("");
    break;
  }
  case booleanValue:
    pushValue(valueToString<_Value>(value.asBool()));
    break;
  case arrayValue:
    writeArrayValue(value);
    break;
  case objectValue: {
	  typename _Value::Members members(value.getMemberNames());
    if (members.empty())
      pushValue("{}");
    else {
      writeWithIndent("{");
      indent();
      typename _Value::Members::iterator it = members.begin();
      for (;;) {
        const String& name = *it;
        const _Value& childValue = value[name];
        writeCommentBeforeValue(childValue);
        writeWithIndent(valueToQuotedString<_Value>(name.c_str()));
        document_ += " : ";
        writeValue(childValue);
        if (++it == members.end()) {
          writeCommentAfterValueOnSameLine(childValue);
          break;
        }
        document_ += ',';
        writeCommentAfterValueOnSameLine(childValue);
      }
      unindent();
      writeWithIndent("}");
    }
  } break;
  }
}

template<class _Value>
void StyledWriter<_Value>::writeArrayValue(const _Value& value) {
  unsigned size = value.size();
  if (size == 0)
    pushValue("[]");
  else {
    bool isArrayMultiLine = isMultineArray(value);
    if (isArrayMultiLine) {
      writeWithIndent("[");
      indent();
      bool hasChildValue = !childValues_.empty();
      unsigned index = 0;
      for (;;) {
        const _Value& childValue = value[index];
        writeCommentBeforeValue(childValue);
        if (hasChildValue)
          writeWithIndent(childValues_[index]);
        else {
          writeIndent();
          writeValue(childValue);
        }
        if (++index == size) {
          writeCommentAfterValueOnSameLine(childValue);
          break;
        }
        document_ += ',';
        writeCommentAfterValueOnSameLine(childValue);
      }
      unindent();
      writeWithIndent("]");
    } else // output on a single line
    {
      assert(childValues_.size() == size);
      document_ += "[ ";
      for (unsigned index = 0; index < size; ++index) {
        if (index > 0)
          document_ += ", ";
        document_ += childValues_[index];
      }
      document_ += " ]";
    }
  }
}

template<class _Value>
bool StyledWriter<_Value>::isMultineArray(const _Value& value) {
  ArrayIndex size = value.size();
  bool isMultiLine = size * 3 >= rightMargin_;
  childValues_.clear();
  for (ArrayIndex index = 0; index < size && !isMultiLine; ++index) {
    const _Value& childValue = value[index];
    isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
                        childValue.size() > 0);
  }
  if (!isMultiLine) // check if line length > max line length
  {
    childValues_.reserve(size);
    addChildValues_ = true;
    ArrayIndex lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
    for (ArrayIndex index = 0; index < size; ++index) {
      if (hasCommentForValue(value[index])) {
        isMultiLine = true;
      }
      writeValue(value[index]);
      lineLength += static_cast<ArrayIndex>(childValues_[index].length());
    }
    addChildValues_ = false;
    isMultiLine = isMultiLine || lineLength >= rightMargin_;
  }
  return isMultiLine;
}

template<class _Value>
void StyledWriter<_Value>::pushValue(const String& value) {
  if (addChildValues_)
    childValues_.push_back(value);
  else
    document_ += value;
}

template<class _Value>
void StyledWriter<_Value>::writeIndent() {
  if (!document_.empty()) {
    char last = document_[document_.length() - 1];
    if (last == ' ') // already indented
      return;
    if (last != '\n') // Comments may add new-line
      document_ += '\n';
  }
  document_ += indentString_;
}

template<class _Value>
void StyledWriter<_Value>::writeWithIndent(const String& value) {
  writeIndent();
  document_ += value;
}

template<class _Value>
void StyledWriter<_Value>::indent() { indentString_ += String(indentSize_, ' '); }

template<class _Value>
void StyledWriter<_Value>::unindent() {
  assert(int(indentString_.size()) >= indentSize_);
  indentString_.resize(indentString_.size() - indentSize_);
}

template<class _Value>
void StyledWriter<_Value>::writeCommentBeforeValue(const _Value& root) {
  if (!root.hasComment(commentBefore))
    return;

  document_ += "\n";
  writeIndent();
  const String& comment = root.getComment(commentBefore);
  typename String::const_iterator iter = comment.begin();
  while (iter != comment.end()) {
    document_ += *iter;
    if (*iter == '\n' &&
       (iter != comment.end() && *(iter + 1) == '/'))
      writeIndent();
    ++iter;
  }

  // Comments are stripped of trailing newlines, so add one here
  document_ += "\n";
}

template<class _Value>
void StyledWriter<_Value>::writeCommentAfterValueOnSameLine(const _Value& root) {
  if (root.hasComment(commentAfterOnSameLine))
    document_ += " " + root.getComment(commentAfterOnSameLine);

  if (root.hasComment(commentAfter)) {
    document_ += "\n";
    document_ += root.getComment(commentAfter);
    document_ += "\n";
  }
}

template<class _Value>
bool StyledWriter<_Value>::hasCommentForValue(const _Value& value) {
  return value.hasComment(commentBefore) ||
         value.hasComment(commentAfterOnSameLine) ||
         value.hasComment(commentAfter);
}

// Class StyledStreamWriter
// //////////////////////////////////////////////////////////////////

template<class _Value>
StyledStreamWriter<_Value>::StyledStreamWriter(String indentation)
    : document_(NULL), rightMargin_(74), indentation_(indentation),
      addChildValues_() {}

template<class _Value>
void StyledStreamWriter<_Value>::write(std::ostream& out, const _Value& root) {
  document_ = &out;
  addChildValues_ = false;
  indentString_ = "";
  indented_ = true;
  writeCommentBeforeValue(root);
  if (!indented_) writeIndent();
  indented_ = true;
  writeValue(root);
  writeCommentAfterValueOnSameLine(root);
  *document_ << "\n";
  document_ = NULL; // Forget the stream, for safety.
}

template<class _Value>
void StyledStreamWriter<_Value>::writeValue(const _Value& value) {
  switch (value.type()) {
  case nullValue:
    pushValue("null");
    break;
  case intValue:
    pushValue(valueToString<_Value>(value.asLargestInt()));
    break;
  case uintValue:
    pushValue(valueToString<_Value>(value.asLargestUInt()));
    break;
  case realValue:
    pushValue(valueToString<_Value>(value.asDouble()));
    break;
  case stringValue:
  {
    // Is NULL possible for value.string_?
    char const* str;
    char const* end;
    bool ok = value.getString(&str, &end);
    if (ok) pushValue(valueToQuotedStringN<_Value>(str, static_cast<unsigned>(end-str)));
    else pushValue("");
    break;
  }
  case booleanValue:
    pushValue(valueToString<_Value>(value.asBool()));
    break;
  case arrayValue:
    writeArrayValue(value);
    break;
  case objectValue: {
	  typename _Value::Members members(value.getMemberNames());
    if (members.empty())
      pushValue("{}");
    else {
      writeWithIndent("{");
      indent();
      typename _Value::Members::iterator it = members.begin();
      for (;;) {
        const String& name = *it;
        const _Value& childValue = value[name];
        writeCommentBeforeValue(childValue);
        writeWithIndent(valueToQuotedString<_Value>(name.c_str()));
        *document_ << " : ";
        writeValue(childValue);
        if (++it == members.end()) {
          writeCommentAfterValueOnSameLine(childValue);
          break;
        }
        *document_ << ",";
        writeCommentAfterValueOnSameLine(childValue);
      }
      unindent();
      writeWithIndent("}");
    }
  } break;
  }
}

template<class _Value>
void StyledStreamWriter<_Value>::writeArrayValue(const _Value& value) {
  unsigned size = value.size();
  if (size == 0)
    pushValue("[]");
  else {
    bool isArrayMultiLine = isMultineArray(value);
    if (isArrayMultiLine) {
      writeWithIndent("[");
      indent();
      bool hasChildValue = !childValues_.empty();
      unsigned index = 0;
      for (;;) {
        const _Value& childValue = value[index];
        writeCommentBeforeValue(childValue);
        if (hasChildValue)
          writeWithIndent(childValues_[index]);
        else {
          if (!indented_) writeIndent();
          indented_ = true;
          writeValue(childValue);
          indented_ = false;
        }
        if (++index == size) {
          writeCommentAfterValueOnSameLine(childValue);
          break;
        }
        *document_ << ",";
        writeCommentAfterValueOnSameLine(childValue);
      }
      unindent();
      writeWithIndent("]");
    } else // output on a single line
    {
      assert(childValues_.size() == size);
      *document_ << "[ ";
      for (unsigned index = 0; index < size; ++index) {
        if (index > 0)
          *document_ << ", ";
        *document_ << childValues_[index];
      }
      *document_ << " ]";
    }
  }
}

template<class _Value>
bool StyledStreamWriter<_Value>::isMultineArray(const _Value& value) {
  ArrayIndex const size = value.size();
  bool isMultiLine = size * 3 >= rightMargin_;
  childValues_.clear();
  for (ArrayIndex index = 0; index < size && !isMultiLine; ++index) {
    const _Value& childValue = value[index];
    isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
                        childValue.size() > 0);
  }
  if (!isMultiLine) // check if line length > max line length
  {
    childValues_.reserve(size);
    addChildValues_ = true;
    ArrayIndex lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
    for (ArrayIndex index = 0; index < size; ++index) {
      if (hasCommentForValue(value[index])) {
        isMultiLine = true;
      }
      writeValue(value[index]);
      lineLength += static_cast<ArrayIndex>(childValues_[index].length());
    }
    addChildValues_ = false;
    isMultiLine = isMultiLine || lineLength >= rightMargin_;
  }
  return isMultiLine;
}

template<class _Value>
void StyledStreamWriter<_Value>::pushValue(const String& value) {
  if (addChildValues_)
    childValues_.push_back(value);
  else
    *document_ << value;
}

template<class _Value>
void StyledStreamWriter<_Value>::writeIndent() {
  // blep intended this to look at the so-far-written string
  // to determine whether we are already indented, but
  // with a stream we cannot do that. So we rely on some saved state.
  // The caller checks indented_.
  *document_ << '\n' << indentString_;
}

template<class _Value>
void StyledStreamWriter<_Value>::writeWithIndent(const String& value) {
  if (!indented_) writeIndent();
  *document_ << value;
  indented_ = false;
}

template<class _Value>
void StyledStreamWriter<_Value>::indent() { indentString_ += indentation_; }

template<class _Value>
void StyledStreamWriter<_Value>::unindent() {
  assert(indentString_.size() >= indentation_.size());
  indentString_.resize(indentString_.size() - indentation_.size());
}

template<class _Value>
void StyledStreamWriter<_Value>::writeCommentBeforeValue(const _Value& root) {
  if (!root.hasComment(commentBefore))
    return;

  if (!indented_) writeIndent();
  const String& comment = root.getComment(commentBefore);
  typename String::const_iterator iter = comment.begin();
  while (iter != comment.end()) {
    *document_ << *iter;
    if (*iter == '\n' &&
       (iter != comment.end() && *(iter + 1) == '/'))
      // writeIndent();  // would include newline
      *document_ << indentString_;
    ++iter;
  }
  indented_ = false;
}

template<class _Value>
void StyledStreamWriter<_Value>::writeCommentAfterValueOnSameLine(const _Value& root) {
  if (root.hasComment(commentAfterOnSameLine))
    *document_ << ' ' << root.getComment(commentAfterOnSameLine);

  if (root.hasComment(commentAfter)) {
    writeIndent();
    *document_ << root.getComment(commentAfter);
  }
  indented_ = false;
}

template<class _Value>
bool StyledStreamWriter<_Value>::hasCommentForValue(const _Value& value) {
  return value.hasComment(commentBefore) ||
         value.hasComment(commentAfterOnSameLine) ||
         value.hasComment(commentAfter);
}

//////////////////////////
// BuiltStyledStreamWriter

/// Scoped enums are not available until C++11.
struct CommentStyle {
  /// Decide whether to write comments.
  enum Enum {
    None,  ///< Drop all comments.
    Most,  ///< Recover odd behavior of previous versions (not implemented yet).
    All  ///< Keep all comments.
  };
};

template<class _Value>
struct BuiltStyledStreamWriter : public StreamWriter<_Value>
{
  typedef typename _Value::String String;
  BuiltStyledStreamWriter(
      String const& indentation,
      CommentStyle::Enum cs,
      String const& colonSymbol,
      String const& nullSymbol,
      String const& endingLineFeedSymbol,
      bool useSpecialFloats,
      unsigned int precision);
  int write(_Value const& root, std::ostream* sout) override;
private:
  void writeValue(_Value const& value);
  void writeArrayValue(_Value const& value);
  bool isMultineArray(_Value const& value);
  void pushValue(String const& value);
  void writeIndent();
  void writeWithIndent(String const& value);
  void indent();
  void unindent();
  void writeCommentBeforeValue(_Value const& root);
  void writeCommentAfterValueOnSameLine(_Value const& root);
  static bool hasCommentForValue(const _Value& value);

  typedef std::vector<String> ChildValues;

  ChildValues childValues_;
  String indentString_;
  int rightMargin_;
  String indentation_;
  CommentStyle::Enum cs_;
  String colonSymbol_;
  String nullSymbol_;
  String endingLineFeedSymbol_;
  bool addChildValues_ : 1;
  bool indented_ : 1;
  bool useSpecialFloats_ : 1;
  unsigned int precision_;
};
template<class _Value>
BuiltStyledStreamWriter<_Value>::BuiltStyledStreamWriter(
      String const& indentation,
      CommentStyle::Enum cs,
      String const& colonSymbol,
      String const& nullSymbol,
      String const& endingLineFeedSymbol,
      bool useSpecialFloats,
      unsigned int precision)
  : rightMargin_(74)
  , indentation_(indentation)
  , cs_(cs)
  , colonSymbol_(colonSymbol)
  , nullSymbol_(nullSymbol)
  , endingLineFeedSymbol_(endingLineFeedSymbol)
  , addChildValues_(false)
  , indented_(false)
  , useSpecialFloats_(useSpecialFloats)
  , precision_(precision)
{
}
template<class _Value>
int BuiltStyledStreamWriter<_Value>::write(_Value const& root, std::ostream* sout)
{
  StreamWriter<_Value>::sout_ = sout;
  addChildValues_ = false;
  indented_ = true;
  indentString_ = "";
  writeCommentBeforeValue(root);
  if (!indented_) writeIndent();
  indented_ = true;
  writeValue(root);
  writeCommentAfterValueOnSameLine(root);
  *(StreamWriter<_Value>::sout_) << endingLineFeedSymbol_;
  StreamWriter<_Value>::sout_ = NULL;
  return 0;
}
template<class _Value>
void BuiltStyledStreamWriter<_Value>::writeValue(_Value const& value) {
  switch (value.type()) {
  case nullValue:
    pushValue(nullSymbol_);
    break;
  case intValue:
    pushValue(valueToString<_Value>(value.asLargestInt()));
    break;
  case uintValue:
    pushValue(valueToString<_Value>(value.asLargestUInt()));
    break;
  case realValue:
    pushValue(valueToString<_Value>(value.asDouble(), useSpecialFloats_, precision_));
    break;
  case stringValue:
  {
    // Is NULL is possible for value.string_?
    char const* str;
    char const* end;
    bool ok = value.getString(&str, &end);
    if (ok) pushValue(valueToQuotedStringN<_Value>(str, static_cast<unsigned>(end-str)));
    else pushValue("");
    break;
  }
  case booleanValue:
    pushValue(valueToString<_Value>(value.asBool()));
    break;
  case arrayValue:
    writeArrayValue(value);
    break;
  case objectValue: {
    typename _Value::Members members(value.getMemberNames());
    if (members.empty())
      pushValue("{}");
    else {
      writeWithIndent("{");
      indent();
      typename _Value::Members::iterator it = members.begin();
      for (;;) {
        String const& name = *it;
        _Value const& childValue = value[name];
        writeCommentBeforeValue(childValue);
        writeWithIndent(valueToQuotedStringN<_Value>(name.data(), static_cast<unsigned>(name.length())));
        *(StreamWriter<_Value>::sout_) << colonSymbol_;
        writeValue(childValue);
        if (++it == members.end()) {
          writeCommentAfterValueOnSameLine(childValue);
          break;
        }
        *(StreamWriter<_Value>::sout_) << ",";
        writeCommentAfterValueOnSameLine(childValue);
      }
      unindent();
      writeWithIndent("}");
    }
  } break;
  }
}

template<class _Value>
void BuiltStyledStreamWriter<_Value>::writeArrayValue(_Value const& value) {
  unsigned size = value.size();
  if (size == 0)
    pushValue("[]");
  else {
    bool isMultiLine = (cs_ == CommentStyle::All) || isMultineArray(value);
    if (isMultiLine) {
      writeWithIndent("[");
      indent();
      bool hasChildValue = !childValues_.empty();
      unsigned index = 0;
      for (;;) {
        _Value const& childValue = value[index];
        writeCommentBeforeValue(childValue);
        if (hasChildValue)
          writeWithIndent(childValues_[index]);
        else {
          if (!indented_) writeIndent();
          indented_ = true;
          writeValue(childValue);
          indented_ = false;
        }
        if (++index == size) {
          writeCommentAfterValueOnSameLine(childValue);
          break;
        }
        *(StreamWriter<_Value>::sout_) << ",";
        writeCommentAfterValueOnSameLine(childValue);
      }
      unindent();
      writeWithIndent("]");
    } else // output on a single line
    {
      assert(childValues_.size() == size);
      *(StreamWriter<_Value>::sout_) << "[";
      if (!indentation_.empty())   *(StreamWriter<_Value>::sout_) << " ";
      for (unsigned index = 0; index < size; ++index) {
        if (index > 0)
        	  *(StreamWriter<_Value>::sout_) << ", ";
        *(StreamWriter<_Value>::sout_) << childValues_[index];
      }
      if (!indentation_.empty())   *(StreamWriter<_Value>::sout_) << " ";
      *(StreamWriter<_Value>::sout_) << "]";
    }
  }
}

template<class _Value>
bool BuiltStyledStreamWriter<_Value>::isMultineArray(_Value const& value) {
  ArrayIndex const size = value.size();
  bool isMultiLine = size * 3 >= rightMargin_;
  childValues_.clear();
  for (ArrayIndex index = 0; index < size && !isMultiLine; ++index) {
    _Value const& childValue = value[index];
    isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
                        childValue.size() > 0);
  }
  if (!isMultiLine) // check if line length > max line length
  {
    childValues_.reserve(size);
    addChildValues_ = true;
    ArrayIndex lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
    for (ArrayIndex index = 0; index < size; ++index) {
      if (hasCommentForValue(value[index])) {
        isMultiLine = true;
      }
      writeValue(value[index]);
      lineLength += static_cast<ArrayIndex>(childValues_[index].length());
    }
    addChildValues_ = false;
    isMultiLine = isMultiLine || lineLength >= rightMargin_;
  }
  return isMultiLine;
}

template<class _Value>
void BuiltStyledStreamWriter<_Value>::pushValue(String const& value) {
  if (addChildValues_)
    childValues_.push_back(value);
  else
	*(StreamWriter<_Value>::sout_) << value;
}

template<class _Value>
void BuiltStyledStreamWriter<_Value>::writeIndent() {
  // blep intended this to look at the so-far-written string
  // to determine whether we are already indented, but
  // with a stream we cannot do that. So we rely on some saved state.
  // The caller checks indented_.

  if (!indentation_.empty()) {
    // In this case, drop newlines too.
	*(StreamWriter<_Value>::sout_) << '\n' << indentString_;
  }
}

template<class _Value>
void BuiltStyledStreamWriter<_Value>::writeWithIndent(String const& value) {
  if (!indented_) writeIndent();
  *(StreamWriter<_Value>::sout_) << value;
  indented_ = false;
}

template<class _Value>
void BuiltStyledStreamWriter<_Value>::indent() { indentString_ += indentation_; }

template<class _Value>
void BuiltStyledStreamWriter<_Value>::unindent() {
  assert(indentString_.size() >= indentation_.size());
  indentString_.resize(indentString_.size() - indentation_.size());
}

template<class _Value>
void BuiltStyledStreamWriter<_Value>::writeCommentBeforeValue(_Value const& root) {
  if (cs_ == CommentStyle::None) return;
  if (!root.hasComment(commentBefore))
    return;

  if (!indented_) writeIndent();
  const String& comment = root.getComment(commentBefore);
  typename String::const_iterator iter = comment.begin();
  while (iter != comment.end()) {
	*(StreamWriter<_Value>::sout_) << *iter;
    if (*iter == '\n' &&
       (iter != comment.end() && *(iter + 1) == '/'))
      // writeIndent();  // would write extra newline
      *(StreamWriter<_Value>::sout_) << indentString_;
    ++iter;
  }
  indented_ = false;
}

template<class _Value>
void BuiltStyledStreamWriter<_Value>::writeCommentAfterValueOnSameLine(_Value const& root) {
  if (cs_ == CommentStyle::None) return;
  if (root.hasComment(commentAfterOnSameLine))
	*(StreamWriter<_Value>::sout_) << " " + root.getComment(commentAfterOnSameLine);

  if (root.hasComment(commentAfter)) {
    writeIndent();
    *(StreamWriter<_Value>::sout_) << root.getComment(commentAfter);
  }
}

// static
template<class _Value>
bool BuiltStyledStreamWriter<_Value>::hasCommentForValue(const _Value& value) {
  return value.hasComment(commentBefore) ||
         value.hasComment(commentAfterOnSameLine) ||
         value.hasComment(commentAfter);
}

///////////////
// StreamWriter

template<class _Value>
StreamWriter<_Value>::StreamWriter()
    : sout_(NULL)
{
}
template<class _Value>
StreamWriter<_Value>::~StreamWriter()
{
}
template<class _Value>
StreamWriter<_Value>::Factory::~Factory()
{}
template<class _Value>
StreamWriterBuilder<_Value>::StreamWriterBuilder()
{
  setDefaults(&settings_);
}
template<class _Value>
StreamWriterBuilder<_Value>::~StreamWriterBuilder()
{}
template<class _Value>
StreamWriter<_Value>* StreamWriterBuilder<_Value>::newStreamWriter() const
{
  String indentation = settings_["indentation"].asString();
  String cs_str = settings_["commentStyle"].asString();
  bool eyc = settings_["enableYAMLCompatibility"].asBool();
  bool dnp = settings_["dropNullPlaceholders"].asBool();
  bool usf = settings_["useSpecialFloats"].asBool(); 
  unsigned int pre = settings_["precision"].asUInt();
  CommentStyle::Enum cs = CommentStyle::All;
  if (cs_str == "All") {
    cs = CommentStyle::All;
  } else if (cs_str == "None") {
    cs = CommentStyle::None;
  } else {
    throwRuntimeError("commentStyle must be 'All' or 'None'");
  }
  String colonSymbol = " : ";
  if (eyc) {
    colonSymbol = ": ";
  } else if (indentation.empty()) {
    colonSymbol = ":";
  }
  String nullSymbol = "null";
  if (dnp) {
    nullSymbol = "";
  }
  if (pre > 17) pre = 17;
  String endingLineFeedSymbol = "";
  return new BuiltStyledStreamWriter<_Value>(
      indentation, cs,
      colonSymbol, nullSymbol, endingLineFeedSymbol, usf, pre);
}
template<class _Value>
static void getValidWriterKeys(std::set<typename _Value::String>* valid_keys)
{
  valid_keys->clear();
  valid_keys->insert("indentation");
  valid_keys->insert("commentStyle");
  valid_keys->insert("enableYAMLCompatibility");
  valid_keys->insert("dropNullPlaceholders");
  valid_keys->insert("useSpecialFloats");
  valid_keys->insert("precision");
}
template<class _Value>
bool StreamWriterBuilder<_Value>::validate(_Value* invalid) const
{
  _Value my_invalid;
  if (!invalid) invalid = &my_invalid;  // so we do not need to test for NULL
  _Value& inv = *invalid;
  std::set<String> valid_keys;
  getValidWriterKeys<_Value>(&valid_keys);
  typename _Value::Members keys = settings_.getMemberNames();
  size_t n = keys.size();
  for (size_t i = 0; i < n; ++i) {
    String const& key = keys[i];
    if (valid_keys.find(key) == valid_keys.end()) {
      inv[key] = settings_[key];
    }
  }
  return 0u == inv.size();
}
template<class _Value>
_Value& StreamWriterBuilder<_Value>::operator[](String key)
{
  return settings_[key];
}
// static
template<class _Value>
void StreamWriterBuilder<_Value>::setDefaults(_Value* settings)
{
  //! [StreamWriterBuilderDefaults]
  (*settings)["commentStyle"] = "All";
  (*settings)["indentation"] = "\t";
  (*settings)["enableYAMLCompatibility"] = false;
  (*settings)["dropNullPlaceholders"] = false;
  (*settings)["useSpecialFloats"] = false;
  (*settings)["precision"] = 17;
  //! [StreamWriterBuilderDefaults]
}

template<class _Value>
typename _Value::String writeString(typename StreamWriter<_Value>::Factory const& builder, _Value const& root) {
#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
typedef std::unique_ptr<StreamWriter<_Value>> StreamWriterPtr;
#else
typedef std::auto_ptr<StreamWriter<_Value>>   StreamWriterPtr;
#endif

  std::ostringstream sout;
  StreamWriterPtr const writer(builder.newStreamWriter());
  writer->write(root, &sout);
  return sout.str();
}

/// \brief Output using the StyledStreamWriter.
/// \see Json::operator>>()
template<class _Alloc, class _String>
std::ostream& operator<<(std::ostream& sout, Value<_Alloc, _String> const& root) {
#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
typedef std::unique_ptr<StreamWriter<Value<_Alloc, _String>>> StreamWriterPtr;
#else
typedef std::auto_ptr<StreamWriter<Value<_Alloc, _String>>>   StreamWriterPtr;
#endif

  StreamWriterBuilder<Value<_Alloc, _String>> builder;
  StreamWriterPtr const writer(builder.newStreamWriter());
  writer->write(root, &sout);
  return sout;
}

} // namespace detail
} // namespace Json

#endif // JSON_WRITER_INL_INCLUDED
