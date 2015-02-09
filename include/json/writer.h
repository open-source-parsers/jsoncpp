// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSON_WRITER_H_INCLUDED
#define JSON_WRITER_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "value.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <vector>
#include <string>
#include <ostream>

// Disable warning C4251: <data member>: <type> needs to have dll-interface to
// be used by...
#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

namespace Json {

class Value;

/**

Usage:
\code
  using namespace Json;
  void writeToStdout(StreamWriter::Factory const& factory, Value const& value) {
    std::unique_ptr<StreamWriter> const writer(
      factory.newStreamWriter(&std::cout));
    writer->write(value);
    std::cout << std::endl;  // add lf and flush
  }
\endcode
*/
class JSON_API StreamWriter {
protected:
  std::ostream& sout_;  // not owned; will not delete
public:
  /// Scoped enums are not available until C++11.
  struct CommentStyle {
    /// Decide whether to write comments.
    enum Enum {
      None,  ///< Drop all comments.
      Most,  ///< Recover odd behavior of previous versions (not implemented yet).
      All  ///< Keep all comments.
    };
  };

  /// Keep a reference, but do not take ownership of `sout`.
  StreamWriter(std::ostream* sout);
  virtual ~StreamWriter();
  /// Write Value into document as configured in sub-class.
  /// \return zero on success
  /// \throw std::exception possibly, depending on configuration
  virtual int write(Value const& root) = 0;

  /** \brief A simple abstract factory.
   */
  class JSON_API Factory {
  public:
    virtual ~Factory();
    /// Do not take ownership of sout, but maintain a reference.
    virtual StreamWriter* newStreamWriter(std::ostream* sout) const = 0;
  };  // Factory
};  // StreamWriter

/// \brief Write into stringstream, then return string, for convenience.
std::string writeString(Value const& root, StreamWriter::Factory const& factory);


/** \brief Build a StreamWriter implementation.

Usage:
\code
  using namespace Json;
  Value value = ...;
  StreamWriterBuilder builder;
  builder.cs_ = StreamWriter::CommentStyle::None;
  std::shared_ptr<StreamWriter> writer(
    builder.newStreamWriter(&std::cout));
  writer->write(value);
  std::cout << std::endl;  // add lf and flush
\endcode
*/
class JSON_API StreamWriterBuilder : public StreamWriter::Factory {
public:
  // Note: We cannot add data-members to this class without a major version bump.
  // So these might as well be completely exposed.

  /** \brief How to write comments.
   * Default: All
   */
  StreamWriter::CommentStyle::Enum cs_;
  /** \brief Write in human-friendly style.

      If "", then skip all indentation and newlines.
      In that case, you probably want CommentStyle::None also.
      Default: "\t"
  */
  std::string indentation_;

  StreamWriterBuilder();
  virtual ~StreamWriterBuilder();

  /// Do not take ownership of sout, but maintain a reference.
  virtual StreamWriter* newStreamWriter(std::ostream* sout) const;
};

/** \brief Build a StreamWriter implementation.
 * Comments are not written, and most whitespace is omitted.
 * In addition, there are some special settings to allow compatibility
 * with the old FastWriter.
 * Usage:
 * \code
 *   OldCompressingStreamWriterBuilder b;
 *   b.dropNullPlaceHolders_ = true; // etc.
 *   StreamWriter* w = b.newStreamWriter(&std::cout);
 *   w.write(value);
 *   delete w;
 * \endcode
 */
class JSON_API OldCompressingStreamWriterBuilder : public StreamWriter::Factory
{
public:
  // Note: We cannot add data-members to this class without a major version bump.
  // So these might as well be completely exposed.

  /** \brief Drop the "null" string from the writer's output for nullValues.
  * Strictly speaking, this is not valid JSON. But when the output is being
  * fed to a browser's Javascript, it makes for smaller output and the
  * browser can handle the output just fine.
  */
  bool dropNullPlaceholders_;
  /** \brief Do not add \n at end of document.
    * Normally, we add an extra newline, just because.
    */
  bool omitEndingLineFeed_;
  /** \brief Add a space after ':'.
    * If indentation is non-empty, we surround colon with whitespace,
    * e.g. " : "
    * This will add back the trailing space when there is no indentation.
    * This seems dubious when the entire document is on a single line,
    * but we leave this here to repduce the behavior of the old `FastWriter`.
    */
  bool enableYAMLCompatibility_;

  OldCompressingStreamWriterBuilder()
    : dropNullPlaceholders_(false)
    , omitEndingLineFeed_(false)
    , enableYAMLCompatibility_(false)
  {}
  virtual StreamWriter* newStreamWriter(std::ostream*) const;
};

/** \brief Abstract class for writers.
 * \deprecated Use StreamWriter.
 */
class JSON_API Writer {
public:
  virtual ~Writer();

  virtual std::string write(const Value& root) = 0;
};

/** \brief Outputs a Value in <a HREF="http://www.json.org">JSON</a> format
 *without formatting (not human friendly).
 *
 * The JSON document is written in a single line. It is not intended for 'human'
 *consumption,
 * but may be usefull to support feature such as RPC where bandwith is limited.
 * \sa Reader, Value
 * \deprecated Use OldCompressingStreamWriterBuilder.
 */
class JSON_API FastWriter : public Writer {
public:
  FastWriter();
  virtual ~FastWriter() {}

  void enableYAMLCompatibility();

  /** \brief Drop the "null" string from the writer's output for nullValues.
   * Strictly speaking, this is not valid JSON. But when the output is being
   * fed to a browser's Javascript, it makes for smaller output and the
   * browser can handle the output just fine.
   */
  void dropNullPlaceholders();

  void omitEndingLineFeed();

public: // overridden from Writer
  virtual std::string write(const Value& root);

private:
  void writeValue(const Value& value);

  std::string document_;
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
class JSON_API StyledWriter : public Writer {
public:
  StyledWriter();
  virtual ~StyledWriter() {}

public: // overridden from Writer
  /** \brief Serialize a Value in <a HREF="http://www.json.org">JSON</a> format.
   * \param root Value to serialize.
   * \return String containing the JSON document that represents the root value.
   */
  virtual std::string write(const Value& root);

private:
  void writeValue(const Value& value);
  void writeArrayValue(const Value& value);
  bool isMultineArray(const Value& value);
  void pushValue(const std::string& value);
  void writeIndent();
  void writeWithIndent(const std::string& value);
  void indent();
  void unindent();
  void writeCommentBeforeValue(const Value& root);
  void writeCommentAfterValueOnSameLine(const Value& root);
  bool hasCommentForValue(const Value& value);
  static std::string normalizeEOL(const std::string& text);

  typedef std::vector<std::string> ChildValues;

  ChildValues childValues_;
  std::string document_;
  std::string indentString_;
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
class JSON_API StyledStreamWriter {
public:
  StyledStreamWriter(std::string indentation = "\t");
  ~StyledStreamWriter() {}

public:
  /** \brief Serialize a Value in <a HREF="http://www.json.org">JSON</a> format.
   * \param out Stream to write to. (Can be ostringstream, e.g.)
   * \param root Value to serialize.
   * \note There is no point in deriving from Writer, since write() should not
   * return a value.
   */
  void write(std::ostream& out, const Value& root);

private:
  void writeValue(const Value& value);
  void writeArrayValue(const Value& value);
  bool isMultineArray(const Value& value);
  void pushValue(const std::string& value);
  void writeIndent();
  void writeWithIndent(const std::string& value);
  void indent();
  void unindent();
  void writeCommentBeforeValue(const Value& root);
  void writeCommentAfterValueOnSameLine(const Value& root);
  bool hasCommentForValue(const Value& value);
  static std::string normalizeEOL(const std::string& text);

  typedef std::vector<std::string> ChildValues;

  ChildValues childValues_;
  std::ostream* document_;
  std::string indentString_;
  int rightMargin_;
  std::string indentation_;
  bool addChildValues_ : 1;
  bool indented_ : 1;
};

#if defined(JSON_HAS_INT64)
std::string JSON_API valueToString(Int value);
std::string JSON_API valueToString(UInt value);
#endif // if defined(JSON_HAS_INT64)
std::string JSON_API valueToString(LargestInt value);
std::string JSON_API valueToString(LargestUInt value);
std::string JSON_API valueToString(double value);
std::string JSON_API valueToString(bool value);
std::string JSON_API valueToQuotedString(const char* value);

/// \brief Output using the StyledStreamWriter.
/// \see Json::operator>>()
JSON_API std::ostream& operator<<(std::ostream&, const Value& root);

} // namespace Json

#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(pop)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

#endif // JSON_WRITER_H_INCLUDED
