// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef CPPTL_JSON_READER_H_INCLUDED
#define CPPTL_JSON_READER_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "features.h"
#include "value.h"
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
 *Value.
 *
 */
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
    std::string message;
  };

  /** \brief Constructs a Reader allowing all features
   * for parsing.
   */
  Reader();

  /** \brief Constructs a Reader allowing the specified feature set
   * for parsing.
   */
  Reader(const Features& features);

  /** \brief Read a Value from a <a HREF="http://www.json.org">JSON</a>
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
  parse(const std::string& document, Value& root, bool collectComments = true);

  /** \brief Read a Value from a <a HREF="http://www.json.org">JSON</a>
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
             Value& root,
             bool collectComments = true);

  /// \brief Parse from input stream.
  /// \see Json::operator>>(std::istream&, Json::Value&).
  bool parse(std::istream& is, Value& root, bool collectComments = true);

  /** \brief Returns a user friendly string that list errors in the parsed
   * document.
   * \return Formatted error message with the list of errors with their location
   * in
   *         the parsed document. An empty string is returned if no error
   * occurred
   *         during parsing.
   * \deprecated Use getFormattedErrorMessages() instead (typo fix).
   */
  JSONCPP_DEPRECATED("Use getFormattedErrorMessages instead")
  std::string getFormatedErrorMessages() const;

  /** \brief Returns a user friendly string that list errors in the parsed
   * document.
   * \return Formatted error message with the list of errors with their location
   * in
   *         the parsed document. An empty string is returned if no error
   * occurred
   *         during parsing.
   */
  std::string getFormattedErrorMessages() const;

  /** \brief Returns a vector of structured erros encounted while parsing.
   * \return A (possibly empty) vector of StructuredError objects. Currently
   *         only one error can be returned, but the caller should tolerate
   * multiple
   *         errors.  This can occur if the parser recovers from a non-fatal
   *         parse error and then encounters additional errors.
   */
  std::vector<StructuredError> getStructuredErrors() const;

  /** \brief Add a semantic error message.
   * \param value JSON Value location associated with the error
   * \param message The error message.
   * \return \c true if the error was successfully added, \c false if the
   * Value offset exceeds the document size.
   */
  bool pushError(const Value& value, const std::string& message);

  /** \brief Add a semantic error message with extra context.
   * \param value JSON Value location associated with the error
   * \param message The error message.
   * \param extra Additional JSON Value location to contextualize the error
   * \return \c true if the error was successfully added, \c false if either
   * Value offset exceeds the document size.
   */
  bool pushError(const Value& value, const std::string& message, const Value& extra);

  /** \brief Return whether there are any errors.
   * \return \c true if there are no errors to report \c false if
   * errors have occurred.
   */
  bool good() const;

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
    std::string message_;
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
  bool decodeNumber(Token& token, Value& decoded);
  bool decodeString(Token& token);
  bool decodeString(Token& token, std::string& decoded);
  bool decodeDouble(Token& token);
  bool decodeDouble(Token& token, Value& decoded);
  bool decodeUnicodeCodePoint(Token& token,
                              Location& current,
                              Location end,
                              unsigned int& unicode);
  bool decodeUnicodeEscapeSequence(Token& token,
                                   Location& current,
                                   Location end,
                                   unsigned int& unicode);
  bool addError(const std::string& message, Token& token, Location extra = 0);
  bool recoverFromError(TokenType skipUntilToken);
  bool addErrorAndRecover(const std::string& message,
                          Token& token,
                          TokenType skipUntilToken);
  void skipUntilSpace();
  Value& currentValue();
  Char getNextChar();
  void
  getLocationLineAndColumn(Location location, int& line, int& column) const;
  std::string getLocationLineAndColumn(Location location) const;
  void addComment(Location begin, Location end, CommentPlacement placement);
  void skipCommentTokens(Token& token);

  typedef std::stack<Value*> Nodes;
  Nodes nodes_;
  Errors errors_;
  std::string document_;
  Location begin_;
  Location end_;
  Location current_;
  Location lastValueEnd_;
  Value* lastValue_;
  std::string commentsBefore_;
  Features features_;
  bool collectComments_;
};  // Reader

/** Interface for reading JSON from a char array.
 */
class JSON_API CharReader {
public:
  virtual ~CharReader() {}
  /** \brief Read a Value from a <a HREF="http://www.json.org">JSON</a>
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
      Value* root, std::string* errs) = 0;

  class Factory {
  public:
    /// \brief Allocate a CharReader via operator new().
    virtual CharReader* newCharReader() const = 0;
  };  // Factory
};  // CharReader

/** \brief Build a CharReader implementation.

Usage:
\code
  using namespace Json;
  CharReaderBuilder builder;
  builder.collectComments_ = true;
  std::shared_ptr<CharReader> reader(
    builder.newCharReader());
  Value value;
  std::string errs;
  bool ok = parseFromStream(std::cin, &value, &errs);
\endcode
*/
class CharReaderBuilder : public CharReader::Factory {
public:
  bool collectComments_;
  Features features_;

  CharReaderBuilder();
  virtual ~CharReaderBuilder();

  virtual CharReader* newCharReader() const;
};

/** Consume entire stream and use its begin/end.
  * Someday we might have a real StreamReader, but for now this
  * is convenient.
  */
bool parseFromStream(
    CharReader::Factory const&,
    std::istream&,
    Value* root, std::string* errs);

/** \brief Read from 'sin' into 'root'.

 Always keep comments from the input JSON.

 This can be used to read a file into a particular sub-object.
 For example:
 \code
 Json::Value root;
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
JSON_API std::istream& operator>>(std::istream&, Value&);

} // namespace Json

#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(pop)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

#endif // CPPTL_JSON_READER_H_INCLUDED
