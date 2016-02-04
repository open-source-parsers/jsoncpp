// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSON_WRITER_H_INCLUDED
#define JSON_WRITER_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "writer_declaration.h"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace Json {

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToQuotedStringN(const char* value, unsigned length) {
  if (value == NULL)
    return "";
  // Not sure how to handle unicode...
  if (WriterUtils::strnpbrk(value, "\"\\\b\f\n\r\t", length) == NULL &&
      !WriterUtils::containsControlCharacter0(value, length))
    return std::basic_string<char, _Traits, _Alloc>("\"") + value + "\"";
  // We have to walk value and escape any special characters.
  // Appending to std::basic_string<char, _Traits, _Alloc> is not efficient, but this should be rare.
  // (Note: forward slashes are *not* rare, but I am not escaping them.)
  typename std::basic_string<char, _Traits, _Alloc>::size_type maxsize =
      length * 2 + 3; // allescaped+quotes+NULL
  std::basic_string<char, _Traits, _Alloc> result;
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
      if ((WriterUtils::isControlCharacter(*c)) || (*c == 0)) {
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

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(LargestInt value) {
  WriterUtils::UIntToStringBuffer buffer;
  char* current = buffer + sizeof(buffer);
  if (value == Value<_Traits, _Alloc>::minLargestInt) {
	WriterUtils::uintToString(LargestUInt(Value<_Traits, _Alloc>::maxLargestInt) + 1, current);
    *--current = '-';
  } else if (value < 0) {
	WriterUtils::uintToString(LargestUInt(-value), current);
    *--current = '-';
  } else {
	WriterUtils::uintToString(LargestUInt(value), current);
  }
  assert(current >= buffer);
  return current;
}

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(LargestUInt value) {
  WriterUtils::UIntToStringBuffer buffer;
  char* current = buffer + sizeof(buffer);
  WriterUtils::uintToString(value, current);
  assert(current >= buffer);
  return current;
}

#if defined(JSON_HAS_INT64)

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(Int value) {
  return valueToString<_Traits, _Alloc>(LargestInt(value));
}

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(UInt value) {
  return valueToString<_Traits, _Alloc>(LargestUInt(value));
}
#endif // # if defined(JSON_HAS_INT64)

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(double value, bool useSpecialFloats, unsigned int precision) {
  // Allocate a buffer that is more than large enough to store the 16 digits of
  // precision requested below.
  char buffer[32];
  int len = -1;

  char formatString[6];
  sprintf(formatString, "%%.%dg", precision);

  // Print into the buffer. We need not request the alternative representation
  // that always has a decimal point because JSON doesn't distingish the
  // concepts of reals and integers.
  if (isfinite(value)) {
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
  WriterUtils::fixNumericLocale(buffer, buffer + len);
  return buffer;
}

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(double value) { return valueToString<_Traits, _Alloc>(value, false, 17); }

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToString(bool value) { return value ? "true" : "false"; }

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> valueToQuotedString(const char* value) {
  if (value == NULL)
    return "";
  // Not sure how to handle unicode...
  if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL &&
      !WriterUtils::containsControlCharacter(value))
    return std::basic_string<char, _Traits, _Alloc>("\"") + value + "\"";
  // We have to walk value and escape any special characters.
  // Appending to std::basic_string<char, _Traits, _Alloc> is not efficient, but this should be rare.
  // (Note: forward slashes are *not* rare, but I am not escaping them.)
  typename std::basic_string<char, _Traits, _Alloc>::size_type maxsize =
      strlen(value) * 2 + 3; // allescaped+quotes+NULL
  std::basic_string<char, _Traits, _Alloc> result;
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
      if (WriterUtils::isControlCharacter(*c)) {
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

///////////////
// StreamWriter

template<typename _Traits, typename _Alloc>
StreamWriter<_Traits, _Alloc>::StreamWriter()
    : sout_(NULL)
{
}
template<typename _Traits, typename _Alloc>
StreamWriter<_Traits, _Alloc>::~StreamWriter()
{
}
template<typename _Traits, typename _Alloc>
StreamWriter<_Traits, _Alloc>::Factory::~Factory()
{}

/** \brief Write into stringstream, then return string, for convenience.
 * A StreamWriter will be created from the factory, used, and then deleted.
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::basic_string<char, _Traits, _Alloc> JSON_API writeString(typename StreamWriter<_Traits, _Alloc>::Factory const& builder, Value<_Traits, _Alloc> const& root) {
#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
  typedef std::unique_ptr<StreamWriter<_Traits, _Alloc>> StreamWriterPtr;
#else
  typedef std::auto_ptr<StreamWriter<_Traits, _Alloc>>   StreamWriterPtr;
#endif
  std::ostringstream sout;
  StreamWriterPtr const writer(builder.newStreamWriter());
  writer->write(root, &sout);
  return sout.str();
}

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
std::ostream& operator<<(std::ostream& sout, Value<_Traits, _Alloc> const& root) {
#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
  typedef std::unique_ptr<StreamWriter<_Traits, _Alloc>> StreamWriterPtr;
#else
  typedef std::auto_ptr<StreamWriter<_Traits, _Alloc>>   StreamWriterPtr;
#endif
  StreamWriterBuilder<_Traits, _Alloc> builder;
  StreamWriterPtr const writer(builder.newStreamWriter());
  writer->write(root, &sout);
  return sout;
}

/// Scoped enums are not available until C++11.
struct CommentStyle {
  /// Decide whether to write comments.
  enum Enum {
    None,  ///< Drop all comments.
    Most,  ///< Recover odd behavior of previous versions (not implemented yet).
    All  ///< Keep all comments.
  };
};


template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
struct BuiltStyledStreamWriter : public StreamWriter<_Traits, _Alloc>
{
  BuiltStyledStreamWriter(
      std::basic_string<char, _Traits, _Alloc> const& indentation,
      CommentStyle::Enum cs,
      std::basic_string<char, _Traits, _Alloc> const& colonSymbol,
      std::basic_string<char, _Traits, _Alloc> const& nullSymbol,
      std::basic_string<char, _Traits, _Alloc> const& endingLineFeedSymbol,
      bool useSpecialFloats,
      unsigned int precision);
  int write(Value<_Traits, _Alloc> const& root, std::ostream* sout) override;
private:
  void writeValue(Value<_Traits, _Alloc> const& value);
  void writeArrayValue(Value<_Traits, _Alloc> const& value);
  bool isMultineArray(Value<_Traits, _Alloc> const& value);
  void pushValue(std::basic_string<char, _Traits, _Alloc> const& value);
  void writeIndent();
  void writeWithIndent(std::basic_string<char, _Traits, _Alloc> const& value);
  void indent();
  void unindent();
  void writeCommentBeforeValue(Value<_Traits, _Alloc> const& root);
  void writeCommentAfterValueOnSameLine(Value<_Traits, _Alloc> const& root);
  static bool hasCommentForValue(const Value<_Traits, _Alloc>& value);

  typedef std::vector<std::basic_string<char, _Traits, _Alloc>> ChildValues;

  ChildValues childValues_;
  std::basic_string<char, _Traits, _Alloc> indentString_;
  int rightMargin_;
  std::basic_string<char, _Traits, _Alloc> indentation_;
  CommentStyle::Enum cs_;
  std::basic_string<char, _Traits, _Alloc> colonSymbol_;
  std::basic_string<char, _Traits, _Alloc> nullSymbol_;
  std::basic_string<char, _Traits, _Alloc> endingLineFeedSymbol_;
  bool addChildValues_ : 1;
  bool indented_ : 1;
  bool useSpecialFloats_ : 1;
  unsigned int precision_;
};

template<typename _Traits, typename _Alloc>
BuiltStyledStreamWriter<_Traits, _Alloc>::BuiltStyledStreamWriter(
      std::basic_string<char, _Traits, _Alloc> const& indentation,
      CommentStyle::Enum cs,
      std::basic_string<char, _Traits, _Alloc> const& colonSymbol,
      std::basic_string<char, _Traits, _Alloc> const& nullSymbol,
      std::basic_string<char, _Traits, _Alloc> const& endingLineFeedSymbol,
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
template<typename _Traits, typename _Alloc>
int BuiltStyledStreamWriter<_Traits, _Alloc>::write(Value<_Traits, _Alloc> const& root, std::ostream* sout)
{
  StreamWriter<_Traits, _Alloc>::sout_ = sout;
  addChildValues_ = false;
  indented_ = true;
  indentString_ = "";
  writeCommentBeforeValue(root);
  if (!indented_) writeIndent();
  indented_ = true;
  writeValue(root);
  writeCommentAfterValueOnSameLine(root);
  *StreamWriter<_Traits, _Alloc>::sout_ << endingLineFeedSymbol_;
  StreamWriter<_Traits, _Alloc>::sout_ = NULL;
  return 0;
}
template<typename _Traits, typename _Alloc>
void BuiltStyledStreamWriter<_Traits, _Alloc>::writeValue(Value<_Traits, _Alloc> const& value) {
  switch (value.type()) {
  case nullValue:
    pushValue(nullSymbol_);
    break;
  case intValue:
    pushValue(valueToString<_Traits, _Alloc>(value.asLargestInt()));
    break;
  case uintValue:
    pushValue(valueToString<_Traits, _Alloc>(value.asLargestUInt()));
    break;
  case realValue:
    pushValue(valueToString<_Traits, _Alloc>(value.asDouble(), useSpecialFloats_, precision_));
    break;
  case stringValue:
  {
    // Is NULL is possible for value.string_?
    char const* str;
    char const* end;
    bool ok = value.getString(&str, &end);
    if (ok) pushValue(valueToQuotedStringN<_Traits, _Alloc>(str, static_cast<unsigned>(end-str)));
    else pushValue("");
    break;
  }
  case booleanValue:
    pushValue(valueToString<_Traits, _Alloc>(value.asBool()));
    break;
  case arrayValue:
    writeArrayValue(value);
    break;
  case objectValue: {
    typename Value<_Traits, _Alloc>::Members members(value.getMemberNames());
    if (members.empty())
      pushValue("{}");
    else {
      writeWithIndent("{");
      indent();
      typename Value<_Traits, _Alloc>::Members::iterator it = members.begin();
      for (;;) {
        std::basic_string<char, _Traits, _Alloc> const& name = *it;
        Value<_Traits, _Alloc> const& childValue = value[name];
        writeCommentBeforeValue(childValue);
        writeWithIndent(valueToQuotedStringN<_Traits, _Alloc>(name.data(), static_cast<unsigned>(name.length())));
        *StreamWriter<_Traits, _Alloc>::sout_ << colonSymbol_;
        writeValue(childValue);
        if (++it == members.end()) {
          writeCommentAfterValueOnSameLine(childValue);
          break;
        }
        *StreamWriter<_Traits, _Alloc>::sout_ << ",";
        writeCommentAfterValueOnSameLine(childValue);
      }
      unindent();
      writeWithIndent("}");
    }
  } break;
  }
}

template<typename _Traits, typename _Alloc>
void BuiltStyledStreamWriter<_Traits, _Alloc>::writeArrayValue(Value<_Traits, _Alloc> const& value) {
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
        Value<_Traits, _Alloc> const& childValue = value[index];
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
        *StreamWriter<_Traits, _Alloc>::sout_ << ",";
        writeCommentAfterValueOnSameLine(childValue);
      }
      unindent();
      writeWithIndent("]");
    } else // output on a single line
    {
      assert(childValues_.size() == size);
      *StreamWriter<_Traits, _Alloc>::sout_ << "[";
      if (!indentation_.empty()) *StreamWriter<_Traits, _Alloc>::sout_ << " ";
      for (unsigned index = 0; index < size; ++index) {
        if (index > 0)
          *StreamWriter<_Traits, _Alloc>::sout_ << ", ";
        *StreamWriter<_Traits, _Alloc>::sout_ << childValues_[index];
      }
      if (!indentation_.empty()) *StreamWriter<_Traits, _Alloc>::sout_ << " ";
      *StreamWriter<_Traits, _Alloc>::sout_ << "]";
    }
  }
}

template<typename _Traits, typename _Alloc>
bool BuiltStyledStreamWriter<_Traits, _Alloc>::isMultineArray(Value<_Traits, _Alloc> const& value) {
  int size = value.size();
  bool isMultiLine = size * 3 >= rightMargin_;
  childValues_.clear();
  for (int index = 0; index < size && !isMultiLine; ++index) {
    Value<_Traits, _Alloc> const& childValue = value[index];
    isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
                        childValue.size() > 0);
  }
  if (!isMultiLine) // check if line length > max line length
  {
    childValues_.reserve(size);
    addChildValues_ = true;
    int lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
    for (int index = 0; index < size; ++index) {
      if (hasCommentForValue(value[index])) {
        isMultiLine = true;
      }
      writeValue(value[index]);
      lineLength += int(childValues_[index].length());
    }
    addChildValues_ = false;
    isMultiLine = isMultiLine || lineLength >= rightMargin_;
  }
  return isMultiLine;
}

template<typename _Traits, typename _Alloc>
void BuiltStyledStreamWriter<_Traits, _Alloc>::pushValue(std::basic_string<char, _Traits, _Alloc> const& value) {
  if (addChildValues_)
    childValues_.push_back(value);
  else
    *StreamWriter<_Traits, _Alloc>::sout_ << value;
}

template<typename _Traits, typename _Alloc>
void BuiltStyledStreamWriter<_Traits, _Alloc>::writeIndent() {
  // blep intended this to look at the so-far-written string
  // to determine whether we are already indented, but
  // with a stream we cannot do that. So we rely on some saved state.
  // The caller checks indented_.

  if (!indentation_.empty()) {
    // In this case, drop newlines too.
    *StreamWriter<_Traits, _Alloc>::sout_ << '\n' << indentString_;
  }
}

template<typename _Traits, typename _Alloc>
void BuiltStyledStreamWriter<_Traits, _Alloc>::writeWithIndent(std::basic_string<char, _Traits, _Alloc> const& value) {
  if (!indented_) writeIndent();
  *StreamWriter<_Traits, _Alloc>::sout_ << value;
  indented_ = false;
}

template<typename _Traits, typename _Alloc>
void BuiltStyledStreamWriter<_Traits, _Alloc>::indent() { indentString_ += indentation_; }

template<typename _Traits, typename _Alloc>
void BuiltStyledStreamWriter<_Traits, _Alloc>::unindent() {
  assert(indentString_.size() >= indentation_.size());
  indentString_.resize(indentString_.size() - indentation_.size());
}

template<typename _Traits, typename _Alloc>
void BuiltStyledStreamWriter<_Traits, _Alloc>::writeCommentBeforeValue(Value<_Traits, _Alloc> const& root) {
  if (cs_ == CommentStyle::None) return;
  if (!root.hasComment(commentBefore))
    return;

  if (!indented_) writeIndent();
  const std::basic_string<char, _Traits, _Alloc>& comment = root.getComment(commentBefore);
  typename std::basic_string<char, _Traits, _Alloc>::const_iterator iter = comment.begin();
  while (iter != comment.end()) {
	if (*iter != 0)
      *StreamWriter<_Traits, _Alloc>::sout_ << *iter;
    if (*iter == '\n' &&
       (iter != comment.end() && *(iter + 1) == '/'))
      // writeIndent();  // would write extra newline
      *StreamWriter<_Traits, _Alloc>::sout_ << indentString_;
    ++iter;
  }
  indented_ = false;
}

template<typename _Traits, typename _Alloc>
void BuiltStyledStreamWriter<_Traits, _Alloc>::writeCommentAfterValueOnSameLine(Value<_Traits, _Alloc> const& root) {
  if (cs_ == CommentStyle::None) return;
  if (root.hasComment(commentAfterOnSameLine))
    *StreamWriter<_Traits, _Alloc>::sout_ << " " + root.getComment(commentAfterOnSameLine);

  if (root.hasComment(commentAfter)) {
    writeIndent();
    *StreamWriter<_Traits, _Alloc>::sout_ << root.getComment(commentAfter);
  }
}

// static
template<typename _Traits, typename _Alloc>
bool BuiltStyledStreamWriter<_Traits, _Alloc>::hasCommentForValue(const Value<_Traits, _Alloc>& value) {
  return value.hasComment(commentBefore) ||
         value.hasComment(commentAfterOnSameLine) ||
         value.hasComment(commentAfter);
}

template<typename _Traits, typename _Alloc>
StreamWriterBuilder<_Traits, _Alloc>::StreamWriterBuilder()
{
  setDefaults(&settings_);
}
template<typename _Traits, typename _Alloc>
StreamWriterBuilder<_Traits, _Alloc>::~StreamWriterBuilder()
{}
template<typename _Traits, typename _Alloc>
StreamWriter<_Traits, _Alloc>* StreamWriterBuilder<_Traits, _Alloc>::newStreamWriter() const
{
  std::basic_string<char, _Traits, _Alloc> indentation = settings_["indentation"].asString();
  std::basic_string<char, _Traits, _Alloc> cs_str = settings_["commentStyle"].asString();
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
  std::basic_string<char, _Traits, _Alloc> colonSymbol = " : ";
  if (eyc) {
    colonSymbol = ": ";
  } else if (indentation.empty()) {
    colonSymbol = ":";
  }
  std::basic_string<char, _Traits, _Alloc> nullSymbol = "null";
  if (dnp) {
    nullSymbol = "";
  }
  if (pre > 17) pre = 17;
  std::basic_string<char, _Traits, _Alloc> endingLineFeedSymbol = "";
  return new BuiltStyledStreamWriter<_Traits, _Alloc>(
      indentation, cs,
      colonSymbol, nullSymbol, endingLineFeedSymbol, usf, pre);
}
template<typename _Traits, typename _Alloc>
void StreamWriterBuilder<_Traits, _Alloc>::getValidWriterKeys(std::set<std::basic_string<char, _Traits, _Alloc>>* valid_keys)
{
  valid_keys->clear();
  valid_keys->insert("indentation");
  valid_keys->insert("commentStyle");
  valid_keys->insert("enableYAMLCompatibility");
  valid_keys->insert("dropNullPlaceholders");
  valid_keys->insert("useSpecialFloats");
  valid_keys->insert("precision");
}
template<typename _Traits, typename _Alloc>
bool StreamWriterBuilder<_Traits, _Alloc>::validate(Json::Value<_Traits, _Alloc>* invalid) const
{
  Json::Value<_Traits, _Alloc> my_invalid;
  if (!invalid) invalid = &my_invalid;  // so we do not need to test for NULL
  Json::Value<_Traits, _Alloc>& inv = *invalid;
  std::set<std::basic_string<char, _Traits, _Alloc>> valid_keys;
  getValidWriterKeys(&valid_keys);
  typename Value<_Traits, _Alloc>::Members keys = settings_.getMemberNames();
  size_t n = keys.size();
  for (size_t i = 0; i < n; ++i) {
    std::basic_string<char, _Traits, _Alloc> const& key = keys[i];
    if (valid_keys.find(key) == valid_keys.end()) {
      inv[key] = settings_[key];
    }
  }
  return 0u == inv.size();
}
template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& StreamWriterBuilder<_Traits, _Alloc>::operator[](std::basic_string<char, _Traits, _Alloc> key)
{
  return settings_[key];
}
// static
template<typename _Traits, typename _Alloc>
void StreamWriterBuilder<_Traits, _Alloc>::setDefaults(Json::Value<_Traits, _Alloc>* settings)
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

// Class Writer
// //////////////////////////////////////////////////////////////////
template<typename _Traits, typename _Alloc>
Writer<_Traits, _Alloc>::~Writer() {}

// Class FastWriter
// //////////////////////////////////////////////////////////////////

template<typename _Traits, typename _Alloc>
FastWriter<_Traits, _Alloc>::FastWriter()
    : yamlCompatiblityEnabled_(false), dropNullPlaceholders_(false),
      omitEndingLineFeed_(false) {}

template<typename _Traits, typename _Alloc>
void FastWriter<_Traits, _Alloc>::enableYAMLCompatibility() { yamlCompatiblityEnabled_ = true; }

template<typename _Traits, typename _Alloc>
void FastWriter<_Traits, _Alloc>::dropNullPlaceholders() { dropNullPlaceholders_ = true; }

template<typename _Traits, typename _Alloc>
void FastWriter<_Traits, _Alloc>::omitEndingLineFeed() { omitEndingLineFeed_ = true; }

template<typename _Traits, typename _Alloc>
std::basic_string<char, _Traits, _Alloc> FastWriter<_Traits, _Alloc>::write(const Value<_Traits, _Alloc>& root) {
  document_ = "";
  writeValue(root);
  if (!omitEndingLineFeed_)
    document_ += "\n";
  return document_;
}

template<typename _Traits, typename _Alloc>
void FastWriter<_Traits, _Alloc>::writeValue(const Value<_Traits, _Alloc>& value) {
  switch (value.type()) {
  case ValueType::nullValue:
    if (!dropNullPlaceholders_)
      document_ += "null";
    break;
  case ValueType::intValue:
    document_ += valueToString<_Traits, _Alloc>(value.asLargestInt());
    break;
  case ValueType::uintValue:
    document_ += valueToString<_Traits, _Alloc>(value.asLargestUInt());
    break;
  case ValueType::realValue:
    document_ += valueToString<_Traits, _Alloc>(value.asDouble());
    break;
  case ValueType::stringValue:
  {
    // Is NULL possible for value.string_?
    char const* str;
    char const* end;
    bool ok = value.getString(&str, &end);
    if (ok) document_ += valueToQuotedStringN<_Traits, _Alloc>(str, static_cast<unsigned>(end-str));
    break;
  }
  case ValueType::booleanValue:
    document_ += valueToString<_Traits, _Alloc>(value.asBool());
    break;
  case ValueType::arrayValue: {
    document_ += '[';
    int size = value.size();
    for (int index = 0; index < size; ++index) {
      if (index > 0)
        document_ += ',';
      writeValue(value[index]);
    }
    document_ += ']';
  } break;
  case ValueType::objectValue: {
    typename Value<_Traits, _Alloc>::Members members(value.getMemberNames());
    document_ += '{';
    for (typename Value<_Traits, _Alloc>::Members::iterator it = members.begin(); it != members.end();
         ++it) {
      const std::basic_string<char, _Traits, _Alloc>& name = *it;
      if (it != members.begin())
        document_ += ',';
      document_ += valueToQuotedStringN<_Traits, _Alloc>(name.data(), static_cast<unsigned>(name.length()));
      document_ += yamlCompatiblityEnabled_ ? ": " : ":";
      writeValue(value[name]);
    }
    document_ += '}';
  } break;
  }
}

// Class StyledWriter
// //////////////////////////////////////////////////////////////////
template<typename _Traits, typename _Alloc>
StyledWriter<_Traits, _Alloc>::StyledWriter()
    : rightMargin_(74), indentSize_(3), addChildValues_() {}

template<typename _Traits, typename _Alloc>
std::basic_string<char, _Traits, _Alloc> StyledWriter<_Traits, _Alloc>::write(const Value<_Traits, _Alloc>& root) {
  document_ = "";
  addChildValues_ = false;
  indentString_ = "";
  writeCommentBeforeValue(root);
  writeValue(root);
  writeCommentAfterValueOnSameLine(root);
  document_ += "\n";
  return document_;
}

template<typename _Traits, typename _Alloc>
void StyledWriter<_Traits, _Alloc>::writeValue(const Value<_Traits, _Alloc>& value) {
  switch (value.type()) {
  case nullValue:
    pushValue("null");
    break;
  case intValue:
    pushValue(valueToString<_Traits, _Alloc>(value.asLargestInt()));
    break;
  case uintValue:
    pushValue(valueToString<_Traits, _Alloc>(value.asLargestUInt()));
    break;
  case realValue:
    pushValue(valueToString<_Traits, _Alloc>(value.asDouble()));
    break;
  case stringValue:
  {
    // Is NULL possible for value.string_?
    char const* str;
    char const* end;
    bool ok = value.getString(&str, &end);
    if (ok) pushValue(valueToQuotedStringN<_Traits, _Alloc>(str, static_cast<unsigned>(end-str)));
    else pushValue("");
    break;
  }
  case booleanValue:
    pushValue(valueToString<_Traits, _Alloc>(value.asBool()));
    break;
  case arrayValue:
    writeArrayValue(value);
    break;
  case objectValue: {
	  typename Value<_Traits, _Alloc>::Members members(value.getMemberNames());
    if (members.empty())
      pushValue("{}");
    else {
      writeWithIndent("{");
      indent();
      typename Value<_Traits, _Alloc>::Members::iterator it = members.begin();
      for (;;) {
        const std::basic_string<char, _Traits, _Alloc>& name = *it;
        const Value<_Traits, _Alloc>& childValue = value[name];
        writeCommentBeforeValue(childValue);
        writeWithIndent(valueToQuotedString<_Traits, _Alloc>(name.c_str()));
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

template<typename _Traits, typename _Alloc>
void StyledWriter<_Traits, _Alloc>::writeArrayValue(const Value<_Traits, _Alloc>& value) {
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
        const Value<_Traits, _Alloc>& childValue = value[index];
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

template<typename _Traits, typename _Alloc>
bool StyledWriter<_Traits, _Alloc>::isMultineArray(const Value<_Traits, _Alloc>& value) {
  int size = value.size();
  bool isMultiLine = size * 3 >= rightMargin_;
  childValues_.clear();
  for (int index = 0; index < size && !isMultiLine; ++index) {
    const Value<_Traits, _Alloc>& childValue = value[index];
    isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
                        childValue.size() > 0);
  }
  if (!isMultiLine) // check if line length > max line length
  {
    childValues_.reserve(size);
    addChildValues_ = true;
    int lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
    for (int index = 0; index < size; ++index) {
      if (hasCommentForValue(value[index])) {
        isMultiLine = true;
      }
      writeValue(value[index]);
      lineLength += int(childValues_[index].length());
    }
    addChildValues_ = false;
    isMultiLine = isMultiLine || lineLength >= rightMargin_;
  }
  return isMultiLine;
}

template<typename _Traits, typename _Alloc>
void StyledWriter<_Traits, _Alloc>::pushValue(const std::basic_string<char, _Traits, _Alloc>& value) {
  if (addChildValues_)
    childValues_.push_back(value);
  else
    document_ += value;
}

template<typename _Traits, typename _Alloc>
void StyledWriter<_Traits, _Alloc>::writeIndent() {
  if (!document_.empty()) {
    char last = document_[document_.length() - 1];
    if (last == ' ') // already indented
      return;
    if (last != '\n') // Comments may add new-line
      document_ += '\n';
  }
  document_ += indentString_;
}

template<typename _Traits, typename _Alloc>
void StyledWriter<_Traits, _Alloc>::writeWithIndent(const std::basic_string<char, _Traits, _Alloc>& value) {
  writeIndent();
  document_ += value;
}

template<typename _Traits, typename _Alloc>
void StyledWriter<_Traits, _Alloc>::indent() { indentString_ += std::basic_string<char, _Traits, _Alloc>(indentSize_, ' '); }

template<typename _Traits, typename _Alloc>
void StyledWriter<_Traits, _Alloc>::unindent() {
  assert(int(indentString_.size()) >= indentSize_);
  indentString_.resize(indentString_.size() - indentSize_);
}

template<typename _Traits, typename _Alloc>
void StyledWriter<_Traits, _Alloc>::writeCommentBeforeValue(const Value<_Traits, _Alloc>& root) {
  if (!root.hasComment(commentBefore))
    return;

  document_ += "\n";
  writeIndent();
  const std::basic_string<char, _Traits, _Alloc>& comment = root.getComment(commentBefore);
  typename std::basic_string<char, _Traits, _Alloc>::const_iterator iter = comment.begin();
  while (iter != comment.end()) {
	if (*iter != 0)
      document_ += *iter;
    if (*iter == '\n' &&
       (iter != comment.end() && *(iter + 1) == '/'))
      writeIndent();
    ++iter;
  }

  // Comments are stripped of trailing newlines, so add one here
  document_ += "\n";
}

template<typename _Traits, typename _Alloc>
void StyledWriter<_Traits, _Alloc>::writeCommentAfterValueOnSameLine(const Value<_Traits, _Alloc>& root) {
  if (root.hasComment(commentAfterOnSameLine))
    document_ += " " + root.getComment(commentAfterOnSameLine);

  if (root.hasComment(commentAfter)) {
    document_ += "\n";
    document_ += root.getComment(commentAfter);
    document_ += "\n";
  }
}

template<typename _Traits, typename _Alloc>
bool StyledWriter<_Traits, _Alloc>::hasCommentForValue(const Value<_Traits, _Alloc>& value) {
  return value.hasComment(commentBefore) ||
         value.hasComment(commentAfterOnSameLine) ||
         value.hasComment(commentAfter);
}

// Class StyledStreamWriter
// //////////////////////////////////////////////////////////////////

template<typename _Traits, typename _Alloc>
StyledStreamWriter<_Traits, _Alloc>::StyledStreamWriter(std::basic_string<char, _Traits, _Alloc> indentation)
    : document_(NULL), rightMargin_(74), indentation_(indentation),
      addChildValues_() {}

template<typename _Traits, typename _Alloc>
void StyledStreamWriter<_Traits, _Alloc>::write(std::ostream& out, const Value<_Traits, _Alloc>& root) {
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

template<typename _Traits, typename _Alloc>
void StyledStreamWriter<_Traits, _Alloc>::writeValue(const Value<_Traits, _Alloc>& value) {
  switch (value.type()) {
  case nullValue:
    pushValue("null");
    break;
  case intValue:
    pushValue(valueToString<_Traits, _Alloc>(value.asLargestInt()));
    break;
  case uintValue:
    pushValue(valueToString<_Traits, _Alloc>(value.asLargestUInt()));
    break;
  case realValue:
    pushValue(valueToString<_Traits, _Alloc>(value.asDouble()));
    break;
  case stringValue:
  {
    // Is NULL possible for value.string_?
    char const* str;
    char const* end;
    bool ok = value.getString(&str, &end);
    if (ok) pushValue(valueToQuotedStringN<_Traits, _Alloc>(str, static_cast<unsigned>(end-str)));
    else pushValue("");
    break;
  }
  case booleanValue:
    pushValue(valueToString<_Traits, _Alloc>(value.asBool()));
    break;
  case arrayValue:
    writeArrayValue(value);
    break;
  case objectValue: {
    typename Value<_Traits, _Alloc>::Members members(value.getMemberNames());
    if (members.empty())
      pushValue("{}");
    else {
      writeWithIndent("{");
      indent();
      typename Value<_Traits, _Alloc>::Members::iterator it = members.begin();
      for (;;) {
        const std::basic_string<char, _Traits, _Alloc>& name = *it;
        const Value<_Traits, _Alloc>& childValue = value[name];
        writeCommentBeforeValue(childValue);
        writeWithIndent(valueToQuotedString<_Traits, _Alloc>(name.c_str()));
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

template<typename _Traits, typename _Alloc>
void StyledStreamWriter<_Traits, _Alloc>::writeArrayValue(const Value<_Traits, _Alloc>& value) {
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
        const Value<_Traits, _Alloc>& childValue = value[index];
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

template<typename _Traits, typename _Alloc>
bool StyledStreamWriter<_Traits, _Alloc>::isMultineArray(const Value<_Traits, _Alloc>& value) {
  int size = value.size();
  bool isMultiLine = size * 3 >= rightMargin_;
  childValues_.clear();
  for (int index = 0; index < size && !isMultiLine; ++index) {
    const Value<_Traits, _Alloc>& childValue = value[index];
    isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
                        childValue.size() > 0);
  }
  if (!isMultiLine) // check if line length > max line length
  {
    childValues_.reserve(size);
    addChildValues_ = true;
    int lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
    for (int index = 0; index < size; ++index) {
      if (hasCommentForValue(value[index])) {
        isMultiLine = true;
      }
      writeValue(value[index]);
      lineLength += int(childValues_[index].length());
    }
    addChildValues_ = false;
    isMultiLine = isMultiLine || lineLength >= rightMargin_;
  }
  return isMultiLine;
}

template<typename _Traits, typename _Alloc>
void StyledStreamWriter<_Traits, _Alloc>::pushValue(const std::basic_string<char, _Traits, _Alloc>& value) {
  if (addChildValues_)
    childValues_.push_back(value);
  else
    *document_ << value;
}

template<typename _Traits, typename _Alloc>
void StyledStreamWriter<_Traits, _Alloc>::writeIndent() {
  // blep intended this to look at the so-far-written string
  // to determine whether we are already indented, but
  // with a stream we cannot do that. So we rely on some saved state.
  // The caller checks indented_.
  *document_ << '\n' << indentString_;
}

template<typename _Traits, typename _Alloc>
void StyledStreamWriter<_Traits, _Alloc>::writeWithIndent(const std::basic_string<char, _Traits, _Alloc>& value) {
  if (!indented_) writeIndent();
  *document_ << value;
  indented_ = false;
}

template<typename _Traits, typename _Alloc>
void StyledStreamWriter<_Traits, _Alloc>::indent() { indentString_ += indentation_; }

template<typename _Traits, typename _Alloc>
void StyledStreamWriter<_Traits, _Alloc>::unindent() {
  assert(indentString_.size() >= indentation_.size());
  indentString_.resize(indentString_.size() - indentation_.size());
}

template<typename _Traits, typename _Alloc>
void StyledStreamWriter<_Traits, _Alloc>::writeCommentBeforeValue(const Value<_Traits, _Alloc>& root) {
  if (!root.hasComment(commentBefore))
    return;

  if (!indented_) writeIndent();
  const std::basic_string<char, _Traits, _Alloc>& comment = root.getComment(commentBefore);
  typename std::basic_string<char, _Traits, _Alloc>::const_iterator iter = comment.begin();
  while (iter != comment.end()) {
	if (*iter != 0)
	  *document_ << *iter;
    if (*iter == '\n' &&
       (iter != comment.end() && *(iter + 1) == '/'))
      // writeIndent();  // would include newline
      *document_ << indentString_;
    ++iter;
  }
  indented_ = false;
}

template<typename _Traits, typename _Alloc>
void StyledStreamWriter<_Traits, _Alloc>::writeCommentAfterValueOnSameLine(const Value<_Traits, _Alloc>& root) {
  if (root.hasComment(commentAfterOnSameLine))
    *document_ << ' ' << root.getComment(commentAfterOnSameLine);

  if (root.hasComment(commentAfter)) {
    writeIndent();
    *document_ << root.getComment(commentAfter);
  }
  indented_ = false;
}

template<typename _Traits, typename _Alloc>
bool StyledStreamWriter<_Traits, _Alloc>::hasCommentForValue(const Value<_Traits, _Alloc>& value) {
  return value.hasComment(commentBefore) ||
         value.hasComment(commentAfterOnSameLine) ||
         value.hasComment(commentAfter);
}

} // namespace Json

#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(pop)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

#endif // JSON_WRITER_H_INCLUDED
