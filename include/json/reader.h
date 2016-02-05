// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef CPPTL_JSON_READER_H_INCLUDED
#define CPPTL_JSON_READER_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "reader_declaration.h"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace Json {

// Implementation of class Reader
// ////////////////////////////////

template<typename _Traits, typename _Alloc>
int const Reader<_Traits, _Alloc>::stackLimit_g = 1000;
template<typename _Traits, typename _Alloc>
int Reader<_Traits, _Alloc>::stackDepth_g = 0;

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::containsNewLine(typename Reader<_Traits, _Alloc>::Location begin, typename Reader<_Traits, _Alloc>::Location end) {
  for (; begin < end; ++begin)
    if (*begin == '\n' || *begin == '\r')
      return true;
  return false;
}

// Class Reader
// //////////////////////////////////////////////////////////////////
template<typename _Traits, typename _Alloc>
Reader<_Traits, _Alloc>::Reader()
    : errors_(), document_(), begin_(), end_(), current_(), lastValueEnd_(),
      lastValue_(), commentsBefore_(), features_(Features::all()),
      collectComments_() {}

template<typename _Traits, typename _Alloc>
Reader<_Traits, _Alloc>::Reader(const Features& features)
    : errors_(), document_(), begin_(), end_(), current_(), lastValueEnd_(),
      lastValue_(), commentsBefore_(), features_(features), collectComments_() {
}

template<typename _Traits, typename _Alloc>
bool
Reader<_Traits, _Alloc>::parse(const std::basic_string<char, _Traits, _Alloc>& document, Value<_Traits, _Alloc>& root, bool collectComments) {
  document_ = document;
  const char* begin = document_.c_str();
  const char* end = begin + document_.length();
  return parse(begin, end, root, collectComments);
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::parse(std::istream& sin, Value<_Traits, _Alloc>& root, bool collectComments) {
  // std::istream_iterator<char> begin(sin);
  // std::istream_iterator<char> end;
  // Those would allow streamed input from a file, if parse() were a
  // template function.

  // Since std::basic_string<char, _Traits, _Alloc> is reference-counted, this at least does not
  // create an extra copy.
  std::basic_string<char, _Traits, _Alloc> doc;
  std::getline(sin, doc, (char)EOF);
  return parse(doc, root, collectComments);
}


template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::parse(const char* beginDoc,
                   const char* endDoc,
                   Value<_Traits, _Alloc>& root,
                   bool collectComments) {
  if (!features_.allowComments_) {
    collectComments = false;
  }

  begin_ = beginDoc;
  end_ = endDoc;
  collectComments_ = collectComments;
  current_ = begin_;
  lastValueEnd_ = 0;
  lastValue_ = 0;
  commentsBefore_ = "";
  errors_.clear();
  while (!nodes_.empty())
    nodes_.pop();
  nodes_.push(&root);

  stackDepth_g = 0;  // Yes, this is bad coding, but options are limited.
  bool successful = readValue();
  Token token;
  skipCommentTokens(token);
  if (collectComments_ && !commentsBefore_.empty())
    root.setComment(commentsBefore_, commentAfter);
  if (features_.strictRoot_) {
    if (!root.isArray() && !root.isObject()) {
      // Set error location to start of doc, ideally should be first token found
      // in doc
      token.type_ = tokenError;
      token.start_ = beginDoc;
      token.end_ = endDoc;
      addError(
          "A valid JSON document must be either an array or an object value.",
          token);
      return false;
    }
  }
  return successful;
}


template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::readValue() {
  // This is a non-reentrant way to support a stackLimit. Terrible!
  // But this deprecated class has a security problem: Bad input can
  // cause a seg-fault. This seems like a fair, binary-compatible way
  // to prevent the problem.
  if (stackDepth_g >= stackLimit_g) throwRuntimeError("Exceeded stackLimit in readValue().");
  ++stackDepth_g;

  Token token;
  skipCommentTokens(token);
  bool successful = true;

  if (collectComments_ && !commentsBefore_.empty()) {
    currentValue().setComment(commentsBefore_, commentBefore);
    commentsBefore_ = "";
  }

  switch (token.type_) {
  case tokenObjectBegin:
    successful = readObject(token);
    currentValue().setOffsetLimit(current_ - begin_);
    break;
  case tokenArrayBegin:
    successful = readArray(token);
    currentValue().setOffsetLimit(current_ - begin_);
    break;
  case tokenNumber:
    successful = decodeNumber(token);
    break;
  case tokenString:
    successful = decodeString(token);
    break;
  case tokenTrue:
    {
    Value<_Traits, _Alloc> v(true);
    currentValue().swapPayload(v);
    currentValue().setOffsetStart(token.start_ - begin_);
    currentValue().setOffsetLimit(token.end_ - begin_);
    }
    break;
  case tokenFalse:
    {
    Value<_Traits, _Alloc> v(false);
    currentValue().swapPayload(v);
    currentValue().setOffsetStart(token.start_ - begin_);
    currentValue().setOffsetLimit(token.end_ - begin_);
    }
    break;
  case tokenNull:
    {
    Value<_Traits, _Alloc> v;
    currentValue().swapPayload(v);
    currentValue().setOffsetStart(token.start_ - begin_);
    currentValue().setOffsetLimit(token.end_ - begin_);
    }
    break;
  case tokenArraySeparator:
  case tokenObjectEnd:
  case tokenArrayEnd:
    if (features_.allowDroppedNullPlaceholders_) {
      // "Un-read" the current token and mark the current value as a null
      // token.
      current_--;
      Value<_Traits, _Alloc> v;
      currentValue().swapPayload(v);
      currentValue().setOffsetStart(current_ - begin_ - 1);
      currentValue().setOffsetLimit(current_ - begin_);
    } // Else, fall through...
    break;
  default:
    currentValue().setOffsetStart(token.start_ - begin_);
    currentValue().setOffsetLimit(token.end_ - begin_);
    return addError("Syntax error: value, object or array expected.", token);
  }

  if (collectComments_) {
    lastValueEnd_ = current_;
    lastValue_ = &currentValue();
  }

  --stackDepth_g;
  return successful;
}

template<typename _Traits, typename _Alloc>
void Reader<_Traits, _Alloc>::skipCommentTokens(Token& token) {
  if (features_.allowComments_) {
    do {
      readToken(token);
    } while (token.type_ == tokenComment);
  } else {
    readToken(token);
  }
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::readToken(Token& token) {
  skipSpaces();
  token.start_ = current_;
  Char c = getNextChar();
  bool ok = true;
  switch (c) {
  case '{':
    token.type_ = tokenObjectBegin;
    break;
  case '}':
    token.type_ = tokenObjectEnd;
    break;
  case '[':
    token.type_ = tokenArrayBegin;
    break;
  case ']':
    token.type_ = tokenArrayEnd;
    break;
  case '"':
    token.type_ = tokenString;
    ok = readString();
    break;
  case '/':
    token.type_ = tokenComment;
    ok = readComment();
    break;
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '-':
    token.type_ = tokenNumber;
    readNumber();
    break;
  case 't':
    token.type_ = tokenTrue;
    ok = match("rue", 3);
    break;
  case 'f':
    token.type_ = tokenFalse;
    ok = match("alse", 4);
    break;
  case 'n':
    token.type_ = tokenNull;
    ok = match("ull", 3);
    break;
  case ',':
    token.type_ = tokenArraySeparator;
    break;
  case ':':
    token.type_ = tokenMemberSeparator;
    break;
  case 0:
    token.type_ = tokenEndOfStream;
    break;
  default:
    ok = false;
    break;
  }
  if (!ok)
    token.type_ = tokenError;
  token.end_ = current_;
  return true;
}

template<typename _Traits, typename _Alloc>
void Reader<_Traits, _Alloc>::skipSpaces() {
  while (current_ != end_) {
    Char c = *current_;
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
      ++current_;
    else
      break;
  }
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::match(Location pattern, int patternLength) {
  if (end_ - current_ < patternLength)
    return false;
  int index = patternLength;
  while (index--)
    if (current_[index] != pattern[index])
      return false;
  current_ += patternLength;
  return true;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::readComment() {
  Location commentBegin = current_ - 1;
  Char c = getNextChar();
  bool successful = false;
  if (c == '*')
    successful = readCStyleComment();
  else if (c == '/')
    successful = readCppStyleComment();
  if (!successful)
    return false;

  if (collectComments_) {
    CommentPlacement placement = commentBefore;
    if (lastValueEnd_ && !containsNewLine(lastValueEnd_, commentBegin)) {
      if (c != '*' || !containsNewLine(commentBegin, current_))
        placement = commentAfterOnSameLine;
    }

    addComment(commentBegin, current_, placement);
  }
  return true;
}

template<typename _Traits, typename _Alloc>
std::basic_string<char, _Traits, _Alloc> Reader<_Traits, _Alloc>::normalizeEOL(typename Reader<_Traits, _Alloc>::Location begin, typename Reader<_Traits, _Alloc>::Location end) {
  std::basic_string<char, _Traits, _Alloc> normalized;
  normalized.reserve(end - begin);
  Reader<_Traits, _Alloc>::Location current = begin;
  while (current != end) {
    char c = *current++;
    if (c == '\r') {
      if (current != end && *current == '\n')
         // convert dos EOL
         ++current;
      // convert Mac EOL
      normalized += '\n';
    } else {
      normalized += c;
    }
  }
  return normalized;
}

template<typename _Traits, typename _Alloc>
void
Reader<_Traits, _Alloc>::addComment(Location begin, Location end, CommentPlacement placement) {
  assert(collectComments_);
  const std::basic_string<char, _Traits, _Alloc>& normalized = normalizeEOL(begin, end);
  if (placement == commentAfterOnSameLine) {
    assert(lastValue_ != 0);
    lastValue_->setComment(normalized, placement);
  } else {
    commentsBefore_ += normalized;
  }
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::readCStyleComment() {
  while (current_ != end_) {
    Char c = getNextChar();
    if (c == '*' && *current_ == '/')
      break;
  }
  return getNextChar() == '/';
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::readCppStyleComment() {
  while (current_ != end_) {
    Char c = getNextChar();
    if (c == '\n')
      break;
    if (c == '\r') {
      // Consume DOS EOL. It will be normalized in addComment.
      if (current_ != end_ && *current_ == '\n')
        getNextChar();
      // Break on Moc OS 9 EOL.
      break;
    }
  }
  return true;
}

template<typename _Traits, typename _Alloc>
void Reader<_Traits, _Alloc>::readNumber() {
  const char *p = current_;
  char c = '0'; // stopgap for already consumed character
  // integral part
  while (c >= '0' && c <= '9')
    c = (current_ = p) < end_ ? *p++ : 0;
  // fractional part
  if (c == '.') {
    c = (current_ = p) < end_ ? *p++ : 0;
    while (c >= '0' && c <= '9')
      c = (current_ = p) < end_ ? *p++ : 0;
  }
  // exponential part
  if (c == 'e' || c == 'E') {
    c = (current_ = p) < end_ ? *p++ : 0;
    if (c == '+' || c == '-')
      c = (current_ = p) < end_ ? *p++ : 0;
    while (c >= '0' && c <= '9')
      c = (current_ = p) < end_ ? *p++ : 0;
  }
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::readString() {
  Char c = 0;
  while (current_ != end_) {
    c = getNextChar();
    if (c == '\\')
      getNextChar();
    else if (c == '"')
      break;
  }
  return c == '"';
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::readObject(Token& tokenStart) {
  Token tokenName;
  std::basic_string<char, _Traits, _Alloc> name;
  Value<_Traits, _Alloc> init(objectValue);
  currentValue().swapPayload(init);
  currentValue().setOffsetStart(tokenStart.start_ - begin_);
  while (readToken(tokenName)) {
    bool initialTokenOk = true;
    while (tokenName.type_ == tokenComment && initialTokenOk)
      initialTokenOk = readToken(tokenName);
    if (!initialTokenOk)
      break;
    if (tokenName.type_ == tokenObjectEnd && name.empty()) // empty object
      return true;
    name = "";
    if (tokenName.type_ == tokenString) {
      if (!decodeString(tokenName, name))
        return recoverFromError(tokenObjectEnd);
    } else if (tokenName.type_ == tokenNumber && features_.allowNumericKeys_) {
      Value<_Traits, _Alloc> numberName;
      if (!decodeNumber(tokenName, numberName))
        return recoverFromError(tokenObjectEnd);
      name = numberName.asString();
    } else {
      break;
    }

    Token colon;
    if (!readToken(colon) || colon.type_ != tokenMemberSeparator) {
      return addErrorAndRecover(
          "Missing ':' after object member name", colon, tokenObjectEnd);
    }
    Value<_Traits, _Alloc>& value = currentValue()[name];
    nodes_.push(&value);
    bool ok = readValue();
    nodes_.pop();
    if (!ok) // error already set
      return recoverFromError(tokenObjectEnd);

    Token comma;
    if (!readToken(comma) ||
        (comma.type_ != tokenObjectEnd && comma.type_ != tokenArraySeparator &&
         comma.type_ != tokenComment)) {
      return addErrorAndRecover(
          "Missing ',' or '}' in object declaration", comma, tokenObjectEnd);
    }
    bool finalizeTokenOk = true;
    while (comma.type_ == tokenComment && finalizeTokenOk)
      finalizeTokenOk = readToken(comma);
    if (comma.type_ == tokenObjectEnd)
      return true;
  }
  return addErrorAndRecover(
      "Missing '}' or object member name", tokenName, tokenObjectEnd);
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::readArray(Token& tokenStart) {
  Value<_Traits, _Alloc> init(arrayValue);
  currentValue().swapPayload(init);
  currentValue().setOffsetStart(tokenStart.start_ - begin_);
  skipSpaces();
  if (*current_ == ']') // empty array
  {
    Token endArray;
    readToken(endArray);
    return true;
  }
  int index = 0;
  for (;;) {
    Value<_Traits, _Alloc>& value = currentValue()[index++];
    nodes_.push(&value);
    bool ok = readValue();
    nodes_.pop();
    if (!ok) // error already set
      return recoverFromError(tokenArrayEnd);

    Token token;
    // Accept Comment after last item in the array.
    ok = readToken(token);
    while (token.type_ == tokenComment && ok) {
      ok = readToken(token);
    }
    bool badTokenType =
        (token.type_ != tokenArraySeparator && token.type_ != tokenArrayEnd);
    if (!ok || badTokenType) {
      return addErrorAndRecover(
          "Missing ',' or ']' in array declaration", token, tokenArrayEnd);
    }
    if (token.type_ == tokenArrayEnd)
      break;
  }
  return true;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::decodeNumber(Token& token) {
  Value<_Traits, _Alloc> decoded;
  if (!decodeNumber(token, decoded))
    return false;
  currentValue().swapPayload(decoded);
  currentValue().setOffsetStart(token.start_ - begin_);
  currentValue().setOffsetLimit(token.end_ - begin_);
  return true;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::decodeNumber(Token& token, Value<_Traits, _Alloc>& decoded) {
  // Attempts to parse the number as an integer. If the number is
  // larger than the maximum supported value of an integer then
  // we decode the number as a double.
  Location current = token.start_;
  bool isNegative = *current == '-';
  if (isNegative)
    ++current;
  // TODO: Help the compiler do the div and mod at compile time or get rid of them.
  typename Value<_Traits, _Alloc>::LargestUInt maxIntegerValue =
      isNegative ? typename Value<_Traits, _Alloc>::LargestUInt(Value<_Traits, _Alloc>::maxLargestInt) + 1
                 : Value<_Traits, _Alloc>::maxLargestUInt;
  typename Value<_Traits, _Alloc>::LargestUInt threshold = maxIntegerValue / 10;
  typename Value<_Traits, _Alloc>::LargestUInt value = 0;
  while (current < token.end_) {
    Char c = *current++;
    if (c < '0' || c > '9')
      return decodeDouble(token, decoded);
    typename Value<_Traits, _Alloc>::UInt digit(c - '0');
    if (value >= threshold) {
      // We've hit or exceeded the max value divided by 10 (rounded down). If
      // a) we've only just touched the limit, b) this is the last digit, and
      // c) it's small enough to fit in that rounding delta, we're okay.
      // Otherwise treat this number as a double to avoid overflow.
      if (value > threshold || current != token.end_ ||
          digit > maxIntegerValue % 10) {
        return decodeDouble(token, decoded);
      }
    }
    value = value * 10 + digit;
  }
  if (isNegative && value == maxIntegerValue)
    decoded = Value<_Traits, _Alloc>::minLargestInt;
  else if (isNegative)
    decoded = - typename Value<_Traits, _Alloc>::LargestInt(value);
  else if (value <= typename Value<_Traits, _Alloc>::LargestUInt(Value<_Traits, _Alloc>::maxInt))
    decoded = typename Value<_Traits, _Alloc>::LargestInt(value);
  else
    decoded = value;
  return true;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::decodeDouble(Token& token) {
  Value<_Traits, _Alloc> decoded;
  if (!decodeDouble(token, decoded))
    return false;
  currentValue().swapPayload(decoded);
  currentValue().setOffsetStart(token.start_ - begin_);
  currentValue().setOffsetLimit(token.end_ - begin_);
  return true;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::decodeDouble(Token& token, Value<_Traits, _Alloc>& decoded) {
  double value = 0;
  std::basic_string<char, _Traits, _Alloc> buffer(token.start_, token.end_);
  std::istringstream is(buffer);
  if (!(is >> value))
    return addError("'" + std::basic_string<char, _Traits, _Alloc>(token.start_, token.end_) +
                        "' is not a number.",
                    token);
  decoded = value;
  return true;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::decodeString(Token& token) {
  std::basic_string<char, _Traits, _Alloc> decoded_string;
  if (!decodeString(token, decoded_string))
    return false;
  Value<_Traits, _Alloc> decoded(decoded_string);
  currentValue().swapPayload(decoded);
  currentValue().setOffsetStart(token.start_ - begin_);
  currentValue().setOffsetLimit(token.end_ - begin_);
  return true;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::decodeString(Token& token, std::basic_string<char, _Traits, _Alloc>& decoded) {
  decoded.reserve(token.end_ - token.start_ - 2);
  Location current = token.start_ + 1; // skip '"'
  Location end = token.end_ - 1;       // do not include '"'
  while (current != end) {
    Char c = *current++;
    if (c == '"')
      break;
    else if (c == '\\') {
      if (current == end)
        return addError("Empty escape sequence in string", token, current);
      Char escape = *current++;
      switch (escape) {
      case '"':
        decoded += '"';
        break;
      case '/':
        decoded += '/';
        break;
      case '\\':
        decoded += '\\';
        break;
      case 'b':
        decoded += '\b';
        break;
      case 'f':
        decoded += '\f';
        break;
      case 'n':
        decoded += '\n';
        break;
      case 'r':
        decoded += '\r';
        break;
      case 't':
        decoded += '\t';
        break;
      case 'u': {
        unsigned int unicode;
        if (!decodeUnicodeCodePoint(token, current, end, unicode))
          return false;
        decoded += WriterUtils::codePointToUTF8(unicode);
      } break;
      default:
        return addError("Bad escape sequence in string", token, current);
      }
    } else {
      decoded += c;
    }
  }
  return true;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::decodeUnicodeCodePoint(Token& token,
                                    Location& current,
                                    Location end,
                                    unsigned int& unicode) {

  if (!decodeUnicodeEscapeSequence(token, current, end, unicode))
    return false;
  if (unicode >= 0xD800 && unicode <= 0xDBFF) {
    // surrogate pairs
    if (end - current < 6)
      return addError(
          "additional six characters expected to parse unicode surrogate pair.",
          token,
          current);
    unsigned int surrogatePair;
    if (*(current++) == '\\' && *(current++) == 'u') {
      if (decodeUnicodeEscapeSequence(token, current, end, surrogatePair)) {
        unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
      } else
        return false;
    } else
      return addError("expecting another \\u token to begin the second half of "
                      "a unicode surrogate pair",
                      token,
                      current);
  }
  return true;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::decodeUnicodeEscapeSequence(Token& token,
                                         Location& current,
                                         Location end,
                                         unsigned int& unicode) {
  if (end - current < 4)
    return addError(
        "Bad unicode escape sequence in string: four digits expected.",
        token,
        current);
  unicode = 0;
  for (int index = 0; index < 4; ++index) {
    Char c = *current++;
    unicode *= 16;
    if (c >= '0' && c <= '9')
      unicode += c - '0';
    else if (c >= 'a' && c <= 'f')
      unicode += c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      unicode += c - 'A' + 10;
    else
      return addError(
          "Bad unicode escape sequence in string: hexadecimal digit expected.",
          token,
          current);
  }
  return true;
}

template<typename _Traits, typename _Alloc>
bool
Reader<_Traits, _Alloc>::addError(const std::basic_string<char, _Traits, _Alloc>& message, Token& token, Location extra) {
  ErrorInfo info;
  info.token_ = token;
  info.message_ = message;
  info.extra_ = extra;
  errors_.push_back(info);
  return false;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::recoverFromError(TokenType skipUntilToken) {
  int errorCount = int(errors_.size());
  Token skip;
  for (;;) {
    if (!readToken(skip))
      errors_.resize(errorCount); // discard errors caused by recovery
    if (skip.type_ == skipUntilToken || skip.type_ == tokenEndOfStream)
      break;
  }
  errors_.resize(errorCount);
  return false;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::addErrorAndRecover(const std::basic_string<char, _Traits, _Alloc>& message,
                                Token& token,
                                TokenType skipUntilToken) {
  addError(message, token);
  return recoverFromError(skipUntilToken);
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& Reader<_Traits, _Alloc>::currentValue() { return *(nodes_.top()); }

template<typename _Traits, typename _Alloc>
typename Reader<_Traits, _Alloc>::Char Reader<_Traits, _Alloc>::getNextChar() {
  if (current_ == end_)
    return 0;
  return *current_++;
}

template<typename _Traits, typename _Alloc>
void Reader<_Traits, _Alloc>::getLocationLineAndColumn(Location location,
                                      int& line,
                                      int& column) const {
  Location current = begin_;
  Location lastLineStart = current;
  line = 0;
  while (current < location && current != end_) {
    Char c = *current++;
    if (c == '\r') {
      if (*current == '\n')
        ++current;
      lastLineStart = current;
      ++line;
    } else if (c == '\n') {
      lastLineStart = current;
      ++line;
    }
  }
  // column & line start at 1
  column = int(location - lastLineStart) + 1;
  ++line;
}

template<typename _Traits, typename _Alloc>
std::basic_string<char, _Traits, _Alloc> Reader<_Traits, _Alloc>::getLocationLineAndColumn(Location location) const {
  int line, column;
  getLocationLineAndColumn(location, line, column);
  char buffer[18 + 16 + 16 + 1];
  snprintf(buffer, sizeof(buffer), "Line %d, Column %d", line, column);
  return buffer;
}

// Deprecated. Preserved for backward compatibility
template<typename _Traits, typename _Alloc>
std::basic_string<char, _Traits, _Alloc> Reader<_Traits, _Alloc>::getFormatedErrorMessages() const {
  return getFormattedErrorMessages();
}

template<typename _Traits, typename _Alloc>
std::basic_string<char, _Traits, _Alloc> Reader<_Traits, _Alloc>::getFormattedErrorMessages() const {
  std::basic_string<char, _Traits, _Alloc> formattedMessage;
  for (typename Errors::const_iterator itError = errors_.begin();
       itError != errors_.end();
       ++itError) {
    const ErrorInfo& error = *itError;
    formattedMessage +=
        "* " + getLocationLineAndColumn(error.token_.start_) + "\n";
    formattedMessage += "  " + error.message_ + "\n";
    if (error.extra_)
      formattedMessage +=
          "See " + getLocationLineAndColumn(error.extra_) + " for detail.\n";
  }
  return formattedMessage;
}

template<typename _Traits, typename _Alloc>
std::vector<typename Reader<_Traits, _Alloc>::StructuredError> Reader<_Traits, _Alloc>::getStructuredErrors() const {
  std::vector<Reader<_Traits, _Alloc>::StructuredError> allErrors;
  for (typename Errors::const_iterator itError = errors_.begin();
       itError != errors_.end();
       ++itError) {
    const ErrorInfo& error = *itError;
    Reader<_Traits, _Alloc>::StructuredError structured;
    structured.offset_start = error.token_.start_ - begin_;
    structured.offset_limit = error.token_.end_ - begin_;
    structured.message = error.message_;
    allErrors.push_back(structured);
  }
  return allErrors;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::pushError(const Value<_Traits, _Alloc>& value, const std::basic_string<char, _Traits, _Alloc>& message) {
  size_t length = end_ - begin_;
  if(value.getOffsetStart() > length
    || value.getOffsetLimit() > length)
    return false;
  Token token;
  token.type_ = tokenError;
  token.start_ = begin_ + value.getOffsetStart();
  token.end_ = end_ + value.getOffsetLimit();
  ErrorInfo info;
  info.token_ = token;
  info.message_ = message;
  info.extra_ = 0;
  errors_.push_back(info);
  return true;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::pushError(const Value<_Traits, _Alloc>& value, const std::basic_string<char, _Traits, _Alloc>& message, const Value<_Traits, _Alloc>& extra) {
  size_t length = end_ - begin_;
  if(value.getOffsetStart() > length
    || value.getOffsetLimit() > length
    || extra.getOffsetLimit() > length)
    return false;
  Token token;
  token.type_ = tokenError;
  token.start_ = begin_ + value.getOffsetStart();
  token.end_ = begin_ + value.getOffsetLimit();
  ErrorInfo info;
  info.token_ = token;
  info.message_ = message;
  info.extra_ = begin_ + extra.getOffsetStart();
  errors_.push_back(info);
  return true;
}

template<typename _Traits, typename _Alloc>
bool Reader<_Traits, _Alloc>::good() const {
  return !errors_.size();
}

// complete copy of Read impl, for OurReader<_Traits, _Alloc>

template<typename _Traits, typename _Alloc>
OurReader<_Traits, _Alloc>::OurReader(OurFeatures const& features)
    : errors_(), document_(), begin_(), end_(), current_(), lastValueEnd_(),
      lastValue_(), commentsBefore_(),
      stackDepth_(0),
      features_(features), collectComments_() {
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::parse(const char* beginDoc,
                   const char* endDoc,
                   Value<_Traits, _Alloc>& root,
                   bool collectComments) {
  if (!features_.allowComments_) {
    collectComments = false;
  }

  begin_ = beginDoc;
  end_ = endDoc;
  collectComments_ = collectComments;
  current_ = begin_;
  lastValueEnd_ = 0;
  lastValue_ = 0;
  commentsBefore_ = "";
  errors_.clear();
  while (!nodes_.empty())
    nodes_.pop();
  nodes_.push(&root);

  stackDepth_ = 0;
  bool successful = readValue();
  Token token;
  skipCommentTokens(token);
  if (features_.failIfExtra_) {
    if (token.type_ != tokenError && token.type_ != tokenEndOfStream) {
      addError("Extra non-whitespace after JSON value.", token);
      return false;
    }
  }
  if (collectComments_ && !commentsBefore_.empty())
    root.setComment(commentsBefore_, commentAfter);
  if (features_.strictRoot_) {
    if (!root.isArray() && !root.isObject()) {
      // Set error location to start of doc, ideally should be first token found
      // in doc
      token.type_ = tokenError;
      token.start_ = beginDoc;
      token.end_ = endDoc;
      addError(
          "A valid JSON document must be either an array or an object value.",
          token);
      return false;
    }
  }
  return successful;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::readValue() {
  if (stackDepth_ >= features_.stackLimit_) throwRuntimeError("Exceeded stackLimit in readValue().");
  ++stackDepth_;
  Token token;
  skipCommentTokens(token);
  bool successful = true;

  if (collectComments_ && !commentsBefore_.empty()) {
    currentValue().setComment(commentsBefore_, commentBefore);
    commentsBefore_ = "";
  }

  switch (token.type_) {
  case tokenObjectBegin:
    successful = readObject(token);
    currentValue().setOffsetLimit(current_ - begin_);
    break;
  case tokenArrayBegin:
    successful = readArray(token);
    currentValue().setOffsetLimit(current_ - begin_);
    break;
  case tokenNumber:
    successful = decodeNumber(token);
    break;
  case tokenString:
    successful = decodeString(token);
    break;
  case tokenTrue:
    {
    Value<_Traits, _Alloc> v(true);
    currentValue().swapPayload(v);
    currentValue().setOffsetStart(token.start_ - begin_);
    currentValue().setOffsetLimit(token.end_ - begin_);
    }
    break;
  case tokenFalse:
    {
    Value<_Traits, _Alloc> v(false);
    currentValue().swapPayload(v);
    currentValue().setOffsetStart(token.start_ - begin_);
    currentValue().setOffsetLimit(token.end_ - begin_);
    }
    break;
  case tokenNull:
    {
    Value<_Traits, _Alloc> v;
    currentValue().swapPayload(v);
    currentValue().setOffsetStart(token.start_ - begin_);
    currentValue().setOffsetLimit(token.end_ - begin_);
    }
    break;
  case tokenNaN:
    {
    Value<_Traits, _Alloc> v(std::numeric_limits<double>::quiet_NaN());
    currentValue().swapPayload(v);
    currentValue().setOffsetStart(token.start_ - begin_);
    currentValue().setOffsetLimit(token.end_ - begin_);
    }
    break;
  case tokenPosInf:
    {
    Value<_Traits, _Alloc> v(std::numeric_limits<double>::infinity());
    currentValue().swapPayload(v);
    currentValue().setOffsetStart(token.start_ - begin_);
    currentValue().setOffsetLimit(token.end_ - begin_);
    }
    break;
  case tokenNegInf:
    {
    Value<_Traits, _Alloc> v(-std::numeric_limits<double>::infinity());
    currentValue().swapPayload(v);
    currentValue().setOffsetStart(token.start_ - begin_);
    currentValue().setOffsetLimit(token.end_ - begin_);
    }
    break;
  case tokenArraySeparator:
  case tokenObjectEnd:
  case tokenArrayEnd:
    if (features_.allowDroppedNullPlaceholders_) {
      // "Un-read" the current token and mark the current value as a null
      // token.
      current_--;
      Value<_Traits, _Alloc> v;
      currentValue().swapPayload(v);
      currentValue().setOffsetStart(current_ - begin_ - 1);
      currentValue().setOffsetLimit(current_ - begin_);
      break;
    } // else, fall through ...
  default:
    currentValue().setOffsetStart(token.start_ - begin_);
    currentValue().setOffsetLimit(token.end_ - begin_);
    return addError("Syntax error: value, object or array expected.", token);
  }

  if (collectComments_) {
    lastValueEnd_ = current_;
    lastValue_ = &currentValue();
  }

  --stackDepth_;
  return successful;
}

template<typename _Traits, typename _Alloc>
void OurReader<_Traits, _Alloc>::skipCommentTokens(Token& token) {
  if (features_.allowComments_) {
    do {
      readToken(token);
    } while (token.type_ == tokenComment);
  } else {
    readToken(token);
  }
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::readToken(Token& token) {
  skipSpaces();
  token.start_ = current_;
  Char c = getNextChar();
  bool ok = true;
  switch (c) {
  case '{':
    token.type_ = tokenObjectBegin;
    break;
  case '}':
    token.type_ = tokenObjectEnd;
    break;
  case '[':
    token.type_ = tokenArrayBegin;
    break;
  case ']':
    token.type_ = tokenArrayEnd;
    break;
  case '"':
    token.type_ = tokenString;
    ok = readString();
    break;
  case '\'':
    if (features_.allowSingleQuotes_) {
    token.type_ = tokenString;
    ok = readStringSingleQuote();
    break;
    } // else continue
  case '/':
    token.type_ = tokenComment;
    ok = readComment();
    break;
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    token.type_ = tokenNumber;
    readNumber(false);
    break;
  case '-':
    if (readNumber(true)) {
      token.type_ = tokenNumber;
    } else {
      token.type_ = tokenNegInf;
      ok = features_.allowSpecialFloats_ && match("nfinity", 7);
    }
    break;
  case 't':
    token.type_ = tokenTrue;
    ok = match("rue", 3);
    break;
  case 'f':
    token.type_ = tokenFalse;
    ok = match("alse", 4);
    break;
  case 'n':
    token.type_ = tokenNull;
    ok = match("ull", 3);
    break;
  case 'N':
    if (features_.allowSpecialFloats_) {
      token.type_ = tokenNaN;
      ok = match("aN", 2);
    } else {
      ok = false;
    }
    break;
  case 'I':
    if (features_.allowSpecialFloats_) {
      token.type_ = tokenPosInf;
      ok = match("nfinity", 7);
    } else {
      ok = false;
    }
    break;
  case ',':
    token.type_ = tokenArraySeparator;
    break;
  case ':':
    token.type_ = tokenMemberSeparator;
    break;
  case 0:
    token.type_ = tokenEndOfStream;
    break;
  default:
    ok = false;
    break;
  }
  if (!ok)
    token.type_ = tokenError;
  token.end_ = current_;
  return true;
}

template<typename _Traits, typename _Alloc>
void OurReader<_Traits, _Alloc>::skipSpaces() {
  while (current_ != end_) {
    Char c = *current_;
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
      ++current_;
    else
      break;
  }
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::match(Location pattern, int patternLength) {
  if (end_ - current_ < patternLength)
    return false;
  int index = patternLength;
  while (index--)
    if (current_[index] != pattern[index])
      return false;
  current_ += patternLength;
  return true;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::readComment() {
  Location commentBegin = current_ - 1;
  Char c = getNextChar();
  bool successful = false;
  if (c == '*')
    successful = readCStyleComment();
  else if (c == '/')
    successful = readCppStyleComment();
  if (!successful)
    return false;

  if (collectComments_) {
    CommentPlacement placement = commentBefore;
    if (lastValueEnd_ && !Reader<_Traits, _Alloc>::containsNewLine(lastValueEnd_, commentBegin)) {
      if (c != '*' || !Reader<_Traits, _Alloc>::containsNewLine(commentBegin, current_))
        placement = commentAfterOnSameLine;
    }

    addComment(commentBegin, current_, placement);
  }
  return true;
}

template<typename _Traits, typename _Alloc>
void
OurReader<_Traits, _Alloc>::addComment(Location begin, Location end, CommentPlacement placement) {
  assert(collectComments_);
  const std::basic_string<char, _Traits, _Alloc>& normalized = Reader<_Traits, _Alloc>::normalizeEOL(begin, end);
  if (placement == commentAfterOnSameLine) {
    assert(lastValue_ != 0);
    lastValue_->setComment(normalized, placement);
  } else {
    commentsBefore_ += normalized;
  }
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::readCStyleComment() {
  while (current_ != end_) {
    Char c = getNextChar();
    if (c == '*' && *current_ == '/')
      break;
  }
  return getNextChar() == '/';
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::readCppStyleComment() {
  while (current_ != end_) {
    Char c = getNextChar();
    if (c == '\n')
      break;
    if (c == '\r') {
      // Consume DOS EOL. It will be normalized in addComment.
      if (current_ != end_ && *current_ == '\n')
        getNextChar();
      // Break on Moc OS 9 EOL.
      break;
    }
  }
  return true;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::readNumber(bool checkInf) {
  const char *p = current_;
  if (checkInf && p != end_ && *p == 'I') {
    current_ = ++p;
    return false;
  }
  char c = '0'; // stopgap for already consumed character
  // integral part
  while (c >= '0' && c <= '9')
    c = (current_ = p) < end_ ? *p++ : 0;
  // fractional part
  if (c == '.') {
    c = (current_ = p) < end_ ? *p++ : 0;
    while (c >= '0' && c <= '9')
      c = (current_ = p) < end_ ? *p++ : 0;
  }
  // exponential part
  if (c == 'e' || c == 'E') {
    c = (current_ = p) < end_ ? *p++ : 0;
    if (c == '+' || c == '-')
      c = (current_ = p) < end_ ? *p++ : 0;
    while (c >= '0' && c <= '9')
      c = (current_ = p) < end_ ? *p++ : 0;
  }
  return true;
}
template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::readString() {
  Char c = 0;
  while (current_ != end_) {
    c = getNextChar();
    if (c == '\\')
      getNextChar();
    else if (c == '"')
      break;
  }
  return c == '"';
}


template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::readStringSingleQuote() {
  Char c = 0;
  while (current_ != end_) {
    c = getNextChar();
    if (c == '\\')
      getNextChar();
    else if (c == '\'')
      break;
  }
  return c == '\'';
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::readObject(Token& tokenStart) {
  Token tokenName;
  std::basic_string<char, _Traits, _Alloc> name;
  Value<_Traits, _Alloc> init(objectValue);
  currentValue().swapPayload(init);
  currentValue().setOffsetStart(tokenStart.start_ - begin_);
  while (readToken(tokenName)) {
    bool initialTokenOk = true;
    while (tokenName.type_ == tokenComment && initialTokenOk)
      initialTokenOk = readToken(tokenName);
    if (!initialTokenOk)
      break;
    if (tokenName.type_ == tokenObjectEnd && name.empty()) // empty object
      return true;
    name = "";
    if (tokenName.type_ == tokenString) {
      if (!decodeString(tokenName, name))
        return recoverFromError(tokenObjectEnd);
    } else if (tokenName.type_ == tokenNumber && features_.allowNumericKeys_) {
      Value<_Traits, _Alloc> numberName;
      if (!decodeNumber(tokenName, numberName))
        return recoverFromError(tokenObjectEnd);
      name = numberName.asString();
    } else {
      break;
    }

    Token colon;
    if (!readToken(colon) || colon.type_ != tokenMemberSeparator) {
      return addErrorAndRecover(
          "Missing ':' after object member name", colon, tokenObjectEnd);
    }
    if (name.length() >= (1U<<30)) throwRuntimeError("keylength >= 2^30");
    if (features_.rejectDupKeys_ && currentValue().isMember(name)) {
      std::basic_string<char, _Traits, _Alloc> msg = "Duplicate key: '" + name + "'";
      return addErrorAndRecover(
          msg, tokenName, tokenObjectEnd);
    }
    Value<_Traits, _Alloc>& value = currentValue()[name];
    nodes_.push(&value);
    bool ok = readValue();
    nodes_.pop();
    if (!ok) // error already set
      return recoverFromError(tokenObjectEnd);

    Token comma;
    if (!readToken(comma) ||
        (comma.type_ != tokenObjectEnd && comma.type_ != tokenArraySeparator &&
         comma.type_ != tokenComment)) {
      return addErrorAndRecover(
          "Missing ',' or '}' in object declaration", comma, tokenObjectEnd);
    }
    bool finalizeTokenOk = true;
    while (comma.type_ == tokenComment && finalizeTokenOk)
      finalizeTokenOk = readToken(comma);
    if (comma.type_ == tokenObjectEnd)
      return true;
  }
  return addErrorAndRecover(
      "Missing '}' or object member name", tokenName, tokenObjectEnd);
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::readArray(Token& tokenStart) {
  Value<_Traits, _Alloc> init(arrayValue);
  currentValue().swapPayload(init);
  currentValue().setOffsetStart(tokenStart.start_ - begin_);
  skipSpaces();
  if (*current_ == ']') // empty array
  {
    Token endArray;
    readToken(endArray);
    return true;
  }
  int index = 0;
  for (;;) {
    Value<_Traits, _Alloc>& value = currentValue()[index++];
    nodes_.push(&value);
    bool ok = readValue();
    nodes_.pop();
    if (!ok) // error already set
      return recoverFromError(tokenArrayEnd);

    Token token;
    // Accept Comment after last item in the array.
    ok = readToken(token);
    while (token.type_ == tokenComment && ok) {
      ok = readToken(token);
    }
    bool badTokenType =
        (token.type_ != tokenArraySeparator && token.type_ != tokenArrayEnd);
    if (!ok || badTokenType) {
      return addErrorAndRecover(
          "Missing ',' or ']' in array declaration", token, tokenArrayEnd);
    }
    if (token.type_ == tokenArrayEnd)
      break;
  }
  return true;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::decodeNumber(Token& token) {
  Value<_Traits, _Alloc> decoded;
  if (!decodeNumber(token, decoded))
    return false;
  currentValue().swapPayload(decoded);
  currentValue().setOffsetStart(token.start_ - begin_);
  currentValue().setOffsetLimit(token.end_ - begin_);
  return true;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::decodeNumber(Token& token, Value<_Traits, _Alloc>& decoded) {
  // Attempts to parse the number as an integer. If the number is
  // larger than the maximum supported value of an integer then
  // we decode the number as a double.
  Location current = token.start_;
  bool isNegative = *current == '-';
  if (isNegative)
    ++current;
  // TODO: Help the compiler do the div and mod at compile time or get rid of them.
  typename Value<_Traits, _Alloc>::LargestUInt maxIntegerValue =
      isNegative ? typename Value<_Traits, _Alloc>::LargestUInt(-Value<_Traits, _Alloc>::minLargestInt)
                 : Value<_Traits, _Alloc>::maxLargestUInt;
  typename Value<_Traits, _Alloc>::LargestUInt threshold = maxIntegerValue / 10;
  typename Value<_Traits, _Alloc>::LargestUInt value = 0;
  while (current < token.end_) {
    Char c = *current++;
    if (c < '0' || c > '9')
      return decodeDouble(token, decoded);
    typename Value<_Traits, _Alloc>::UInt digit(c - '0');
    if (value >= threshold) {
      // We've hit or exceeded the max value divided by 10 (rounded down). If
      // a) we've only just touched the limit, b) this is the last digit, and
      // c) it's small enough to fit in that rounding delta, we're okay.
      // Otherwise treat this number as a double to avoid overflow.
      if (value > threshold || current != token.end_ ||
          digit > maxIntegerValue % 10) {
        return decodeDouble(token, decoded);
      }
    }
    value = value * 10 + digit;
  }
  if (isNegative)
    decoded = - typename Value<_Traits, _Alloc>::LargestInt(value);
  else if (value <= typename Value<_Traits, _Alloc>::LargestUInt(Value<_Traits, _Alloc>::maxInt))
    decoded = typename Value<_Traits, _Alloc>::LargestInt(value);
  else
    decoded = value;
  return true;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::decodeDouble(Token& token) {
  Value<_Traits, _Alloc> decoded;
  if (!decodeDouble(token, decoded))
    return false;
  currentValue().swapPayload(decoded);
  currentValue().setOffsetStart(token.start_ - begin_);
  currentValue().setOffsetLimit(token.end_ - begin_);
  return true;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::decodeDouble(Token& token, Value<_Traits, _Alloc>& decoded) {
  double value = 0;
  const int bufferSize = 32;
  int count;
  int length = int(token.end_ - token.start_);

  // Sanity check to avoid buffer overflow exploits.
  if (length < 0) {
    return addError("Unable to parse token length", token);
  }

  // Avoid using a string constant for the format control string given to
  // sscanf, as this can cause hard to debug crashes on OS X. See here for more
  // info:
  //
  //     http://developer.apple.com/library/mac/#DOCUMENTATION/DeveloperTools/gcc-4.0.1/gcc/Incompatibilities.html
  char format[] = "%lf";

  if (length <= bufferSize) {
    Char buffer[bufferSize + 1];
    memcpy(buffer, token.start_, length);
    buffer[length] = 0;
    count = sscanf(buffer, format, &value);
  } else {
    std::basic_string<char, _Traits, _Alloc> buffer(token.start_, token.end_);
    count = sscanf(buffer.c_str(), format, &value);
  }

  if (count != 1)
    return addError("'" + std::basic_string<char, _Traits, _Alloc>(token.start_, token.end_) +
                        "' is not a number.",
                    token);
  decoded = value;
  return true;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::decodeString(Token& token) {
  std::basic_string<char, _Traits, _Alloc> decoded_string;
  if (!decodeString(token, decoded_string))
    return false;
  Value<_Traits, _Alloc> decoded(decoded_string);
  currentValue().swapPayload(decoded);
  currentValue().setOffsetStart(token.start_ - begin_);
  currentValue().setOffsetLimit(token.end_ - begin_);
  return true;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::decodeString(Token& token, std::basic_string<char, _Traits, _Alloc>& decoded) {
  decoded.reserve(token.end_ - token.start_ - 2);
  Location current = token.start_ + 1; // skip '"'
  Location end = token.end_ - 1;       // do not include '"'
  while (current != end) {
    Char c = *current++;
    if (c == '"')
      break;
    else if (c == '\\') {
      if (current == end)
        return addError("Empty escape sequence in string", token, current);
      Char escape = *current++;
      switch (escape) {
      case '"':
        decoded += '"';
        break;
      case '/':
        decoded += '/';
        break;
      case '\\':
        decoded += '\\';
        break;
      case 'b':
        decoded += '\b';
        break;
      case 'f':
        decoded += '\f';
        break;
      case 'n':
        decoded += '\n';
        break;
      case 'r':
        decoded += '\r';
        break;
      case 't':
        decoded += '\t';
        break;
      case 'u': {
        unsigned int unicode;
        if (!decodeUnicodeCodePoint(token, current, end, unicode))
          return false;
        decoded += WriterUtils::codePointToUTF8(unicode);
      } break;
      default:
        return addError("Bad escape sequence in string", token, current);
      }
    } else {
      decoded += c;
    }
  }
  return true;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::decodeUnicodeCodePoint(Token& token,
                                    Location& current,
                                    Location end,
                                    unsigned int& unicode) {

  if (!decodeUnicodeEscapeSequence(token, current, end, unicode))
    return false;
  if (unicode >= 0xD800 && unicode <= 0xDBFF) {
    // surrogate pairs
    if (end - current < 6)
      return addError(
          "additional six characters expected to parse unicode surrogate pair.",
          token,
          current);
    unsigned int surrogatePair;
    if (*(current++) == '\\' && *(current++) == 'u') {
      if (decodeUnicodeEscapeSequence(token, current, end, surrogatePair)) {
        unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
      } else
        return false;
    } else
      return addError("expecting another \\u token to begin the second half of "
                      "a unicode surrogate pair",
                      token,
                      current);
  }
  return true;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::decodeUnicodeEscapeSequence(Token& token,
                                         Location& current,
                                         Location end,
                                         unsigned int& unicode) {
  if (end - current < 4)
    return addError(
        "Bad unicode escape sequence in string: four digits expected.",
        token,
        current);
  unicode = 0;
  for (int index = 0; index < 4; ++index) {
    Char c = *current++;
    unicode *= 16;
    if (c >= '0' && c <= '9')
      unicode += c - '0';
    else if (c >= 'a' && c <= 'f')
      unicode += c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      unicode += c - 'A' + 10;
    else
      return addError(
          "Bad unicode escape sequence in string: hexadecimal digit expected.",
          token,
          current);
  }
  return true;
}

template<typename _Traits, typename _Alloc>
bool
OurReader<_Traits, _Alloc>::addError(const std::basic_string<char, _Traits, _Alloc>& message, Token& token, Location extra) {
  ErrorInfo info;
  info.token_ = token;
  info.message_ = message;
  info.extra_ = extra;
  errors_.push_back(info);
  return false;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::recoverFromError(TokenType skipUntilToken) {
  int errorCount = int(errors_.size());
  Token skip;
  for (;;) {
    if (!readToken(skip))
      errors_.resize(errorCount); // discard errors caused by recovery
    if (skip.type_ == skipUntilToken || skip.type_ == tokenEndOfStream)
      break;
  }
  errors_.resize(errorCount);
  return false;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::addErrorAndRecover(const std::basic_string<char, _Traits, _Alloc>& message,
                                Token& token,
                                TokenType skipUntilToken) {
  addError(message, token);
  return recoverFromError(skipUntilToken);
}

template<typename _Traits, typename _Alloc>
Value<_Traits, _Alloc>& OurReader<_Traits, _Alloc>::currentValue() { return *(nodes_.top()); }

template<typename _Traits, typename _Alloc>
typename OurReader<_Traits, _Alloc>::Char OurReader<_Traits, _Alloc>::getNextChar() {
  if (current_ == end_)
    return 0;
  return *current_++;
}

template<typename _Traits, typename _Alloc>
void OurReader<_Traits, _Alloc>::getLocationLineAndColumn(Location location,
                                      int& line,
                                      int& column) const {
  Location current = begin_;
  Location lastLineStart = current;
  line = 0;
  while (current < location && current != end_) {
    Char c = *current++;
    if (c == '\r') {
      if (*current == '\n')
        ++current;
      lastLineStart = current;
      ++line;
    } else if (c == '\n') {
      lastLineStart = current;
      ++line;
    }
  }
  // column & line start at 1
  column = int(location - lastLineStart) + 1;
  ++line;
}

template<typename _Traits, typename _Alloc>
std::basic_string<char, _Traits, _Alloc> OurReader<_Traits, _Alloc>::getLocationLineAndColumn(Location location) const {
  int line, column;
  getLocationLineAndColumn(location, line, column);
  char buffer[18 + 16 + 16 + 1];
  snprintf(buffer, sizeof(buffer), "Line %d, Column %d", line, column);
  return buffer;
}

template<typename _Traits, typename _Alloc>
std::basic_string<char, _Traits, _Alloc> OurReader<_Traits, _Alloc>::getFormattedErrorMessages() const {
  std::basic_string<char, _Traits, _Alloc> formattedMessage;
  for (typename Errors::const_iterator itError = errors_.begin();
       itError != errors_.end();
       ++itError) {
    const ErrorInfo& error = *itError;
    formattedMessage +=
        "* " + getLocationLineAndColumn(error.token_.start_) + "\n";
    formattedMessage += "  " + error.message_ + "\n";
    if (error.extra_)
      formattedMessage +=
          "See " + getLocationLineAndColumn(error.extra_) + " for detail.\n";
  }
  return formattedMessage;
}

template<typename _Traits, typename _Alloc>
std::vector<typename OurReader<_Traits, _Alloc>::StructuredError> OurReader<_Traits, _Alloc>::getStructuredErrors() const {
  std::vector<OurReader<_Traits, _Alloc>::StructuredError> allErrors;
  for (typename Errors::const_iterator itError = errors_.begin();
       itError != errors_.end();
       ++itError) {
    const ErrorInfo& error = *itError;
    OurReader<_Traits, _Alloc>::StructuredError structured;
    structured.offset_start = error.token_.start_ - begin_;
    structured.offset_limit = error.token_.end_ - begin_;
    structured.message = error.message_;
    allErrors.push_back(structured);
  }
  return allErrors;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::pushError(const Value<_Traits, _Alloc>& value, const std::basic_string<char, _Traits, _Alloc>& message) {
  size_t length = end_ - begin_;
  if(value.getOffsetStart() > length
    || value.getOffsetLimit() > length)
    return false;
  Token token;
  token.type_ = tokenError;
  token.start_ = begin_ + value.getOffsetStart();
  token.end_ = end_ + value.getOffsetLimit();
  ErrorInfo info;
  info.token_ = token;
  info.message_ = message;
  info.extra_ = 0;
  errors_.push_back(info);
  return true;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::pushError(const Value<_Traits, _Alloc>& value, const std::basic_string<char, _Traits, _Alloc>& message, const Value<_Traits, _Alloc>& extra) {
  size_t length = end_ - begin_;
  if(value.getOffsetStart() > length
    || value.getOffsetLimit() > length
    || extra.getOffsetLimit() > length)
    return false;
  Token token;
  token.type_ = tokenError;
  token.start_ = begin_ + value.getOffsetStart();
  token.end_ = begin_ + value.getOffsetLimit();
  ErrorInfo info;
  info.token_ = token;
  info.message_ = message;
  info.extra_ = begin_ + extra.getOffsetStart();
  errors_.push_back(info);
  return true;
}

template<typename _Traits, typename _Alloc>
bool OurReader<_Traits, _Alloc>::good() const {
  return !errors_.size();
}

template<typename _Traits, typename _Alloc>
CharReaderBuilder<_Traits, _Alloc>::CharReaderBuilder()
{
  setDefaults(&settings_);
}
template<typename _Traits, typename _Alloc>
CharReaderBuilder<_Traits, _Alloc>::~CharReaderBuilder()
{}
template<typename _Traits, typename _Alloc>
CharReader<_Traits, _Alloc>* CharReaderBuilder<_Traits, _Alloc>::newCharReader() const
{
  bool collectComments = settings_["collectComments"].asBool();
  OurFeatures features = OurFeatures::all();
  features.allowComments_ = settings_["allowComments"].asBool();
  features.strictRoot_ = settings_["strictRoot"].asBool();
  features.allowDroppedNullPlaceholders_ = settings_["allowDroppedNullPlaceholders"].asBool();
  features.allowNumericKeys_ = settings_["allowNumericKeys"].asBool();
  features.allowSingleQuotes_ = settings_["allowSingleQuotes"].asBool();
  features.stackLimit_ = settings_["stackLimit"].asInt();
  features.failIfExtra_ = settings_["failIfExtra"].asBool();
  features.rejectDupKeys_ = settings_["rejectDupKeys"].asBool();
  features.allowSpecialFloats_ = settings_["allowSpecialFloats"].asBool();
  return new OurCharReader<_Traits, _Alloc>(collectComments, features);
}
template<typename _Traits, typename _Alloc>
void CharReaderBuilder<_Traits, _Alloc>::getValidReaderKeys(std::set<std::basic_string<char, _Traits, _Alloc>>* valid_keys)
{
  valid_keys->clear();
  valid_keys->insert("collectComments");
  valid_keys->insert("allowComments");
  valid_keys->insert("strictRoot");
  valid_keys->insert("allowDroppedNullPlaceholders");
  valid_keys->insert("allowNumericKeys");
  valid_keys->insert("allowSingleQuotes");
  valid_keys->insert("stackLimit");
  valid_keys->insert("failIfExtra");
  valid_keys->insert("rejectDupKeys");
  valid_keys->insert("allowSpecialFloats");
}
template<typename _Traits, typename _Alloc>
bool CharReaderBuilder<_Traits, _Alloc>::validate(Json::Value<_Traits, _Alloc>* invalid) const
{
  Json::Value<_Traits, _Alloc> my_invalid;
  if (!invalid) invalid = &my_invalid;  // so we do not need to test for NULL
  Json::Value<_Traits, _Alloc>& inv = *invalid;
  std::set<std::basic_string<char, _Traits, _Alloc>> valid_keys;
  getValidReaderKeys(&valid_keys);
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
Value<_Traits, _Alloc>& CharReaderBuilder<_Traits, _Alloc>::operator[](std::basic_string<char, _Traits, _Alloc> key)
{
  return settings_[key];
}
// static
template<typename _Traits, typename _Alloc>
void CharReaderBuilder<_Traits, _Alloc>::strictMode(Json::Value<_Traits, _Alloc>* settings)
{
//! [CharReaderBuilderStrictMode]
  (*settings)["allowComments"] = false;
  (*settings)["strictRoot"] = true;
  (*settings)["allowDroppedNullPlaceholders"] = false;
  (*settings)["allowNumericKeys"] = false;
  (*settings)["allowSingleQuotes"] = false;
  (*settings)["stackLimit"] = 1000;
  (*settings)["failIfExtra"] = true;
  (*settings)["rejectDupKeys"] = true;
  (*settings)["allowSpecialFloats"] = false;
//! [CharReaderBuilderStrictMode]
}
// static
template<typename _Traits, typename _Alloc>
void CharReaderBuilder<_Traits, _Alloc>::setDefaults(Json::Value<_Traits, _Alloc>* settings)
{
//! [CharReaderBuilderDefaults]
  (*settings)["collectComments"] = true;
  (*settings)["allowComments"] = true;
  (*settings)["strictRoot"] = false;
  (*settings)["allowDroppedNullPlaceholders"] = false;
  (*settings)["allowNumericKeys"] = false;
  (*settings)["allowSingleQuotes"] = false;
  (*settings)["stackLimit"] = 1000;
  (*settings)["failIfExtra"] = false;
  (*settings)["rejectDupKeys"] = false;
  (*settings)["allowSpecialFloats"] = false;
//! [CharReaderBuilderDefaults]
}

//////////////////////////////////
// global functions

/** Consume entire stream and use its begin/end.
  * Someday we might have a real StreamReader, but for now this
  * is convenient.
  */
template<typename _Traits, typename _Alloc>
JSON_API bool parseFromStream(
    typename CharReader<_Traits, _Alloc>::Factory const& fact, std::istream& sin,
    Value<_Traits, _Alloc>* root, std::basic_string<char, _Traits, _Alloc>* errs)
{
  std::ostringstream ssin;
  ssin << sin.rdbuf();
  std::basic_string<char, _Traits, _Alloc> doc = ssin.str();
  char const* begin = doc.data();
  char const* end = begin + doc.size();

#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
using CharReaderPtr = std::unique_ptr<CharReader<_Traits, _Alloc>>;
#else
using CharReaderPtr = std::auto_ptr<CharReader<_Traits, _Alloc>>;
#endif

  // Note that we do not actually need a null-terminator.
  CharReaderPtr const reader(fact.newCharReader());
  return reader->parse(begin, end, root, errs);
}

/** \brief Read from 'sin' into 'root'.

 Always keep comments from the input JSON.

 This can be used to read a file into a particular sub-object.
 For example:
 \code
 Json::Value<_Traits, _Alloc> root;
 cin >> root["dir"]["file"];
 cout << root;
 \endcode
 Result:
 \verbatim
 {
 "dir": {
     "file": {
     // The input stream JSON would be nested here.
     }
 }
 }
 \endverbatim
 \throw std::exception on parse error.
 \see Json::operator<<()
*/
template<typename _Traits, typename _Alloc>
JSON_API std::istream& operator>>(std::istream& sin, Value<_Traits, _Alloc>& root) {
  CharReaderBuilder<_Traits, _Alloc> b;
  std::basic_string<char, _Traits, _Alloc> errs;
  bool ok = parseFromStream(b, sin, &root, &errs);
  if (!ok) {
    fprintf(stderr,
            "Error from reader: %s",
            errs.c_str());

    throwRuntimeError(errs);
  }
  return sin;
}

} // namespace Json

#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(pop)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

#endif // CPPTL_JSON_READER_H_INCLUDED
