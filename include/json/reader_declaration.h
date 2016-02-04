// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef CPPTL_JSON_READER_DECL_H_INCLUDED
#define CPPTL_JSON_READER_DECL_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "features.h"
#include "value_declaration.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <deque>
#include <iosfwd>
#include <stack>
#include <string>
#include <istream>

// Disable warning C4251: <data member>: <type> needs to have dll-interface to
// be used by...
#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

namespace Json {

/** \brief Unserialize a <a HREF="http://www.json.org">JSON</a> document into a
 *Value<_Traits, _Alloc>.
 *
 * \deprecated Use CharReader and CharReaderBuilder.
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API Reader {
public:
  typedef char Char;
  typedef const Char* Location;

  /** \brief An error tagged with where in the JSON text it was encountered.
   *
   * The offsets give the [start, limit) range of bytes within the text. Note
   * that this is bytes, not codepoints.
   *
   */
  struct StructuredError {
    size_t offset_start;
    size_t offset_limit;
    std::basic_string<char, _Traits, _Alloc> message;
  };

  /** \brief Constructs a Reader allowing all features
   * for parsing.
   */
  Reader();

  /** \brief Constructs a Reader allowing the specified feature set
   * for parsing.
   */
  Reader(const Features& features);

  /** \brief Read a Value<_Traits, _Alloc> from a <a HREF="http://www.json.org">JSON</a>
   * document.
   * \param document UTF-8 encoded string containing the document to read.
   * \param root [out] Contains the root value of the document if it was
   *             successfully parsed.
   * \param collectComments \c true to collect comment and allow writing them
   * back during
   *                        serialization, \c false to discard comments.
   *                        This parameter is ignored if
   * Features::allowComments_
   *                        is \c false.
   * \return \c true if the document was successfully parsed, \c false if an
   * error occurred.
   */
  bool
  parse(const std::basic_string<char, _Traits, _Alloc>& document, Value<_Traits, _Alloc>& root, bool collectComments = true);

  /** \brief Read a Value<_Traits, _Alloc> from a <a HREF="http://www.json.org">JSON</a>
   document.
   * \param beginDoc Pointer on the beginning of the UTF-8 encoded string of the
   document to read.
   * \param endDoc Pointer on the end of the UTF-8 encoded string of the
   document to read.
   *               Must be >= beginDoc.
   * \param root [out] Contains the root value of the document if it was
   *             successfully parsed.
   * \param collectComments \c true to collect comment and allow writing them
   back during
   *                        serialization, \c false to discard comments.
   *                        This parameter is ignored if
   Features::allowComments_
   *                        is \c false.
   * \return \c true if the document was successfully parsed, \c false if an
   error occurred.
   */
  bool parse(const char* beginDoc,
             const char* endDoc,
             Value<_Traits, _Alloc>& root,
             bool collectComments = true);

  /// \brief Parse from input stream.
  /// \see Json::operator>>(std::istream&, Json::Value<_Traits, _Alloc>&).
  bool parse(std::istream& is, Value<_Traits, _Alloc>& root, bool collectComments = true);

  /** \brief Returns a user friendly string that list errors in the parsed
   * document.
   * \return Formatted error message with the list of errors with their location
   * in
   *         the parsed document. An empty string is returned if no error
   * occurred
   *         during parsing.
   * \deprecated Use getFormattedErrorMessages() instead (typo fix).
   */
  JSONCPP_DEPRECATED("Use getFormattedErrorMessages() instead.")
  std::basic_string<char, _Traits, _Alloc> getFormatedErrorMessages() const;

  /** \brief Returns a user friendly string that list errors in the parsed
   * document.
   * \return Formatted error message with the list of errors with their location
   * in
   *         the parsed document. An empty string is returned if no error
   * occurred
   *         during parsing.
   */
  std::basic_string<char, _Traits, _Alloc> getFormattedErrorMessages() const;

  /** \brief Returns a vector of structured erros encounted while parsing.
   * \return A (possibly empty) vector of StructuredError objects. Currently
   *         only one error can be returned, but the caller should tolerate
   * multiple
   *         errors.  This can occur if the parser recovers from a non-fatal
   *         parse error and then encounters additional errors.
   */
  std::vector<Reader::StructuredError> getStructuredErrors() const;

  /** \brief Add a semantic error message.
   * \param value JSON Value<_Traits, _Alloc> location associated with the error
   * \param message The error message.
   * \return \c true if the error was successfully added, \c false if the
   * Value<_Traits, _Alloc> offset exceeds the document size.
   */
  bool pushError(const Value<_Traits, _Alloc>& value, const std::basic_string<char, _Traits, _Alloc>& message);

  /** \brief Add a semantic error message with extra context.
   * \param value JSON Value<_Traits, _Alloc> location associated with the error
   * \param message The error message.
   * \param extra Additional JSON Value<_Traits, _Alloc> location to contextualize the error
   * \return \c true if the error was successfully added, \c false if either
   * Value<_Traits, _Alloc> offset exceeds the document size.
   */
  bool pushError(const Value<_Traits, _Alloc>& value, const std::basic_string<char, _Traits, _Alloc>& message, const Value<_Traits, _Alloc>& extra);

  /** \brief Return whether there are any errors.
   * \return \c true if there are no errors to report \c false if
   * errors have occurred.
   */
  bool good() const;

  static bool containsNewLine(Reader<_Traits, _Alloc>::Location begin, Reader<_Traits, _Alloc>::Location end);
  static std::basic_string<char, _Traits, _Alloc> normalizeEOL(Reader<_Traits, _Alloc>::Location begin, Reader<_Traits, _Alloc>::Location end);

private:
  enum TokenType {
    tokenEndOfStream = 0,
    tokenObjectBegin,
    tokenObjectEnd,
    tokenArrayBegin,
    tokenArrayEnd,
    tokenString,
    tokenNumber,
    tokenTrue,
    tokenFalse,
    tokenNull,
    tokenArraySeparator,
    tokenMemberSeparator,
    tokenComment,
    tokenError
  };

  class Token {
  public:
    TokenType type_;
    Location start_;
    Location end_;
  };

  class ErrorInfo {
  public:
    Token token_;
    std::basic_string<char, _Traits, _Alloc> message_;
    Location extra_;
  };

  typedef std::deque<ErrorInfo> Errors;

  bool readToken(Token& token);
  void skipSpaces();
  bool match(Location pattern, int patternLength);
  bool readComment();
  bool readCStyleComment();
  bool readCppStyleComment();
  bool readString();
  void readNumber();
  bool readValue();
  bool readObject(Token& token);
  bool readArray(Token& token);
  bool decodeNumber(Token& token);
  bool decodeNumber(Token& token, Value<_Traits, _Alloc>& decoded);
  bool decodeString(Token& token);
  bool decodeString(Token& token, std::basic_string<char, _Traits, _Alloc>& decoded);
  bool decodeDouble(Token& token);
  bool decodeDouble(Token& token, Value<_Traits, _Alloc>& decoded);
  bool decodeUnicodeCodePoint(Token& token,
                              Location& current,
                              Location end,
                              unsigned int& unicode);
  bool decodeUnicodeEscapeSequence(Token& token,
                                   Location& current,
                                   Location end,
                                   unsigned int& unicode);
  bool addError(const std::basic_string<char, _Traits, _Alloc>& message, Token& token, Location extra = 0);
  bool recoverFromError(TokenType skipUntilToken);
  bool addErrorAndRecover(const std::basic_string<char, _Traits, _Alloc>& message,
                          Token& token,
                          TokenType skipUntilToken);
  void skipUntilSpace();
  Value<_Traits, _Alloc>& currentValue();
  Char getNextChar();
  void
  getLocationLineAndColumn(Location location, int& line, int& column) const;
  std::basic_string<char, _Traits, _Alloc> getLocationLineAndColumn(Location location) const;
  void addComment(Location begin, Location end, CommentPlacement placement);
  void skipCommentTokens(Token& token);

  typedef std::stack<Value<_Traits, _Alloc>*> Nodes;
  Nodes nodes_;
  Errors errors_;
  std::basic_string<char, _Traits, _Alloc> document_;
  Location begin_;
  Location end_;
  Location current_;
  Location lastValueEnd_;
  Value<_Traits, _Alloc>* lastValue_;
  std::basic_string<char, _Traits, _Alloc> commentsBefore_;
  Features features_;
  bool collectComments_;

  static int const stackLimit_g;
  static int       stackDepth_g;  // see readValue()
};  // Reader

/** Interface for reading JSON from a char array.
 */
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API CharReader {
public:
  virtual ~CharReader() {}
  /** \brief Read a Value<_Traits, _Alloc> from a <a HREF="http://www.json.org">JSON</a>
   document.
   * The document must be a UTF-8 encoded string containing the document to read.
   *
   * \param beginDoc Pointer on the beginning of the UTF-8 encoded string of the
   document to read.
   * \param endDoc Pointer on the end of the UTF-8 encoded string of the
   document to read.
   *        Must be >= beginDoc.
   * \param root [out] Contains the root value of the document if it was
   *             successfully parsed.
   * \param errs [out] Formatted error messages (if not NULL)
   *        a user friendly string that lists errors in the parsed
   * document.
   * \return \c true if the document was successfully parsed, \c false if an
   error occurred.
   */
  virtual bool parse(
      char const* beginDoc, char const* endDoc,
      Value<_Traits, _Alloc>* root, std::basic_string<char, _Traits, _Alloc>* errs) = 0;

  class JSON_API Factory {
  public:
    virtual ~Factory() {}
    /** \brief Allocate a CharReader via operator new().
     * \throw std::exception if something goes wrong (e.g. invalid settings)
     */
    virtual CharReader* newCharReader() const = 0;
  };  // Factory
};  // CharReader

/** \brief Build a CharReader implementation.

Usage:
\code
  using namespace Json;
  CharReaderBuilder builder;
  builder["collectComments"] = false;
  Value<_Traits, _Alloc> value;
  std::basic_string<char, _Traits, _Alloc> errs;
  bool ok = parseFromStream(builder, std::cin, &value, &errs);
\endcode
*/
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class JSON_API CharReaderBuilder : public CharReader<_Traits, _Alloc>::Factory {
public:
  // Note: We use a Json::Value<_Traits, _Alloc> so that we can add data-members to this class
  // without a major version bump.
  /** Configuration of this builder.
    These are case-sensitive.
    Available settings (case-sensitive):
    - `"collectComments": false or true`
      - true to collect comment and allow writing them
        back during serialization, false to discard comments.
        This parameter is ignored if allowComments is false.
    - `"allowComments": false or true`
      - true if comments are allowed.
    - `"strictRoot": false or true`
      - true if root must be either an array or an object value
    - `"allowDroppedNullPlaceholders": false or true`
      - true if dropped null placeholders are allowed. (See StreamWriterBuilder.)
    - `"allowNumericKeys": false or true`
      - true if numeric object keys are allowed.
    - `"allowSingleQuotes": false or true`
      - true if '' are allowed for strings (both keys and values)
    - `"stackLimit": integer`
      - Exceeding stackLimit (recursive depth of `readValue()`) will
        cause an exception.
      - This is a security issue (seg-faults caused by deeply nested JSON),
        so the default is low.
    - `"failIfExtra": false or true`
      - If true, `parse()` returns false when extra non-whitespace trails
        the JSON value in the input string.
    - `"rejectDupKeys": false or true`
      - If true, `parse()` returns false when a key is duplicated within an object.
    - `"allowSpecialFloats": false or true`
      - If true, special float values (NaNs and infinities) are allowed 
        and their values are lossfree restorable.

    You can examine 'settings_` yourself
    to see the defaults. You can also write and read them just like any
    JSON Value<_Traits, _Alloc>.
    \sa setDefaults()
    */
  Json::Value<_Traits, _Alloc> settings_;

  CharReaderBuilder();
  ~CharReaderBuilder() override;

  CharReader<_Traits, _Alloc>* newCharReader() const override;

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
   * \snippet src/lib_json/json_reader.cpp CharReaderBuilderDefaults
   */
  static void setDefaults(Json::Value<_Traits, _Alloc>* settings);
  /** Same as old Features::strictMode().
   * \pre 'settings' != NULL (but Json::null is fine)
   * \remark Defaults:
   * \snippet src/lib_json/json_reader.cpp CharReaderBuilderStrictMode
   */
  static void strictMode(Json::Value<_Traits, _Alloc>* settings);
private:
  static void getValidReaderKeys(std::set<std::basic_string<char, _Traits, _Alloc>>* valid_keys);
};

// exact copy of Features
class OurFeatures {
public:
  static OurFeatures all();
  bool allowComments_;
  bool strictRoot_;
  bool allowDroppedNullPlaceholders_;
  bool allowNumericKeys_;
  bool allowSingleQuotes_;
  bool failIfExtra_;
  bool rejectDupKeys_;
  bool allowSpecialFloats_;
  int stackLimit_;
};  // OurFeatures


// Implementation of class Reader
// ////////////////////////////////

// exact copy of Reader, renamed to OurReader
template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class OurReader {
public:
  typedef char Char;
  typedef const Char* Location;
  struct StructuredError {
    size_t offset_start;
    size_t offset_limit;
    std::basic_string<char, _Traits, _Alloc> message;
  };

  OurReader(OurFeatures const& features);
  bool parse(const char* beginDoc,
             const char* endDoc,
             Value<_Traits, _Alloc>& root,
             bool collectComments = true);
  std::basic_string<char, _Traits, _Alloc> getFormattedErrorMessages() const;
  std::vector<OurReader::StructuredError> getStructuredErrors() const;
  bool pushError(const Value<_Traits, _Alloc>& value, const std::basic_string<char, _Traits, _Alloc>& message);
  bool pushError(const Value<_Traits, _Alloc>& value, const std::basic_string<char, _Traits, _Alloc>& message, const Value<_Traits, _Alloc>& extra);
  bool good() const;

private:
  OurReader(OurReader const&);  // no impl
  void operator=(OurReader const&);  // no impl

  enum TokenType {
    tokenEndOfStream = 0,
    tokenObjectBegin,
    tokenObjectEnd,
    tokenArrayBegin,
    tokenArrayEnd,
    tokenString,
    tokenNumber,
    tokenTrue,
    tokenFalse,
    tokenNull,
    tokenNaN,
    tokenPosInf,
    tokenNegInf,
    tokenArraySeparator,
    tokenMemberSeparator,
    tokenComment,
    tokenError
  };

  class Token {
  public:
    TokenType type_;
    Location start_;
    Location end_;
  };

  class ErrorInfo {
  public:
    Token token_;
    std::basic_string<char, _Traits, _Alloc> message_;
    Location extra_;
  };

  typedef std::deque<ErrorInfo> Errors;

  bool readToken(Token& token);
  void skipSpaces();
  bool match(Location pattern, int patternLength);
  bool readComment();
  bool readCStyleComment();
  bool readCppStyleComment();
  bool readString();
  bool readStringSingleQuote();
  bool readNumber(bool checkInf);
  bool readValue();
  bool readObject(Token& token);
  bool readArray(Token& token);
  bool decodeNumber(Token& token);
  bool decodeNumber(Token& token, Value<_Traits, _Alloc>& decoded);
  bool decodeString(Token& token);
  bool decodeString(Token& token, std::basic_string<char, _Traits, _Alloc>& decoded);
  bool decodeDouble(Token& token);
  bool decodeDouble(Token& token, Value<_Traits, _Alloc>& decoded);
  bool decodeUnicodeCodePoint(Token& token,
                              Location& current,
                              Location end,
                              unsigned int& unicode);
  bool decodeUnicodeEscapeSequence(Token& token,
                                   Location& current,
                                   Location end,
                                   unsigned int& unicode);
  bool addError(const std::basic_string<char, _Traits, _Alloc>& message, Token& token, Location extra = 0);
  bool recoverFromError(TokenType skipUntilToken);
  bool addErrorAndRecover(const std::basic_string<char, _Traits, _Alloc>& message,
                          Token& token,
                          TokenType skipUntilToken);
  void skipUntilSpace();
  Value<_Traits, _Alloc>& currentValue();
  Char getNextChar();
  void
  getLocationLineAndColumn(Location location, int& line, int& column) const;
  std::basic_string<char, _Traits, _Alloc> getLocationLineAndColumn(Location location) const;
  void addComment(Location begin, Location end, CommentPlacement placement);
  void skipCommentTokens(Token& token);

  typedef std::stack<Value<_Traits, _Alloc>*> Nodes;
  Nodes nodes_;
  Errors errors_;
  std::basic_string<char, _Traits, _Alloc> document_;
  Location begin_;
  Location end_;
  Location current_;
  Location lastValueEnd_;
  Value<_Traits, _Alloc>* lastValue_;
  std::basic_string<char, _Traits, _Alloc> commentsBefore_;
  int stackDepth_;

  OurFeatures const features_;
  bool collectComments_;
};  // OurReader

template<typename _Traits JSONCPP_DEFAULT_CHAR_TRAITS, typename _Alloc JSONCPP_DEFAULT_ALLOCATOR>
class OurCharReader : public CharReader<_Traits, _Alloc> {
  bool const collectComments_;
  OurReader<_Traits, _Alloc> reader_;
public:
  OurCharReader(
    bool collectComments,
    OurFeatures const& features)
  : collectComments_(collectComments)
  , reader_(features)
  {}
  bool parse(
      char const* beginDoc, char const* endDoc,
      Value<_Traits, _Alloc>* root, std::basic_string<char, _Traits, _Alloc>* errs) override {
    bool ok = reader_.parse(beginDoc, endDoc, *root, collectComments_);
    if (errs) {
      *errs = reader_.getFormattedErrorMessages();
    }
    return ok;
  }
};

//////////////////////////////////
// global functions

/** Consume entire stream and use its begin/end.
  * Someday we might have a real StreamReader, but for now this
  * is convenient.
  */
template<typename _Traits, typename _Alloc>
JSON_API bool parseFromStream(
    typename CharReader<_Traits, _Alloc>::Factory const& fact, std::istream& sin,
    Value<_Traits, _Alloc>* root, std::basic_string<char, _Traits, _Alloc>* errs);

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
JSON_API std::istream& operator>>(std::istream& sin, Value<_Traits, _Alloc>& root);

} // namespace Json

#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(pop)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

#endif // CPPTL_JSON_READER_DECL_H_INCLUDED
