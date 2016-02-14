/* This will be the new Json::Value in JsonCpp-2.0.0
 *
 * When we are ready, we will rename this to Value,
 * Value to ValImpl, and this will wrap the ValImpl.
 * Then we will move the Exceptions, Iterators, and Paths here too.
 * For now, we are working on the API we *want*.
 */
#include <vector>

// Disable warning C4251: <data member>: <type> needs to have dll-interface to
// be used by...
#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)

/** \brief JSON (JavaScript Object Notation).
 */
namespace Json {

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
template <String>
class JSON_API Thing {
public:
  typedef std::vector<String> Members;
  //typedef ThingIterator iterator;
  //typedef ThingConstIterator const_iterator;

  static const Thing& null;  ///< We regret this reference to a global instance; prefer the simpler Thing().
  static const Thing& nullRef;  ///< just a kludge for binary-compatibility; same as null
  /// Minimum signed integer value that can be stored in a Json::Thing.
  static const LargestInt minLargestInt;
  /// Maximum signed integer value that can be stored in a Json::Thing.
  static const LargestInt maxLargestInt;
  /// Maximum unsigned integer value that can be stored in a Json::Thing.
  static const LargestUInt maxLargestUInt;

  /// Minimum signed int value that can be stored in a Json::Thing.
  static const Int minInt;
  /// Maximum signed int value that can be stored in a Json::Thing.
  static const Int maxInt;
  /// Maximum unsigned int value that can be stored in a Json::Thing.
  static const UInt maxUInt;

#if defined(JSON_HAS_INT64)
  /// Minimum signed 64 bits int value that can be stored in a Json::Thing.
  static const Int64 minInt64;
  /// Maximum signed 64 bits int value that can be stored in a Json::Thing.
  static const Int64 maxInt64;
  /// Maximum unsigned 64 bits int value that can be stored in a Json::Thing.
  static const UInt64 maxUInt64;
#endif // defined(JSON_HAS_INT64)

public:
  //typedef std::map<CZString, Thing> ObjectValues;

public:
  /** \brief Create a default Thing of the given type.

    This is a very useful constructor.
    To create an empty array, pass arrayThing.
    To create an empty object, pass objectThing.
    Another Thing can then be set to this one by assignment.
This is useful since clear() and resize() will not alter types.

    Examples:
\code
Json::Thing null_value; // null
Json::Thing arr_value(Json::arrayThing); // []
Json::Thing obj_value(Json::objectThing); // {}
\endcode
  */
  Thing(ThingType type = nullThing);
  Thing(Int value);
  Thing(UInt value);
#if defined(JSON_HAS_INT64)
  Thing(Int64 value);
  Thing(UInt64 value);
#endif // if defined(JSON_HAS_INT64)
  Thing(double value);
  Thing(const char* value); ///< Copy til first 0. (NULL causes to seg-fault.)
  Thing(const char* begin, const char* end); ///< Copy all, incl zeroes.
  Thing(const String& value); ///< Copy data() til size(). Embedded zeroes too.
  Thing(bool value);
  /// Deep copy.
  Thing(const Thing& other);
#if JSON_HAS_RVALUE_REFERENCES
  /// Move constructor
  Thing(Thing&& other);
#endif
  ~Thing();

  /// Deep copy, then swap(other).
  /// \note Over-write existing comments. To preserve comments, use #swapPayload().
  Thing& operator=(Thing other);
  /// Swap everything.
  void swap(Thing& other);
  /// Swap values but leave comments and source offsets in place.
  void swapPayload(Thing& other);

  ThingType type() const;

  /// Compare payload only, not comments etc.
  bool operator<(const Thing& other) const;
  bool operator<=(const Thing& other) const;
  bool operator>=(const Thing& other) const;
  bool operator>(const Thing& other) const;
  bool operator==(const Thing& other) const;
  bool operator!=(const Thing& other) const;
  int compare(const Thing& other) const;

  const char* asCString() const; ///< Embedded zeroes could cause you trouble!
  String asString() const; ///< Embedded zeroes are possible.
  /** Get raw char* of string-value.
   *  \return false if !string. (Seg-fault if str or end are NULL.)
   */
  bool getString(
      char const** begin, char const** end) const;
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

  bool isConvertibleTo(ThingType other) const;

  /// Number of values in array or object
  ArrayIndex size() const;

  /// \brief Return true if empty array, empty object, or null;
  /// otherwise, false.
  bool empty() const;

  /// Return isNull()
  bool operator!() const;

  /// Remove all object members and array elements.
  /// \pre type() is arrayThing, objectThing, or nullThing
  /// \post type() is unchanged
  void clear();

  /// Resize the array to size elements.
  /// New elements are initialized to null.
  /// May only be called on nullThing or arrayThing.
  /// \pre type() is arrayThing or nullThing
  /// \post type() is arrayThing
  void resize(ArrayIndex size);

  /// Access an array element (zero based index ).
  /// If the array contains less than index element, then null value are
  /// inserted
  /// in the array so that its size is index+1.
  /// (You may need to say 'value[0u]' to get your compiler to distinguish
  ///  this from the operator[] which takes a string.)
  Thing& operator[](ArrayIndex index);

  /// Access an array element (zero based index ).
  /// If the array contains less than index element, then null value are
  /// inserted
  /// in the array so that its size is index+1.
  /// (You may need to say 'value[0u]' to get your compiler to distinguish
  ///  this from the operator[] which takes a string.)
  Thing& operator[](int index);

  /// Access an array element (zero based index )
  /// (You may need to say 'value[0u]' to get your compiler to distinguish
  ///  this from the operator[] which takes a string.)
  const Thing& operator[](ArrayIndex index) const;

  /// Access an array element (zero based index )
  /// (You may need to say 'value[0u]' to get your compiler to distinguish
  ///  this from the operator[] which takes a string.)
  const Thing& operator[](int index) const;

  /// If the array contains at least index+1 elements, returns the element
  /// value,
  /// otherwise returns defaultThing.
  Thing get(ArrayIndex index, const Thing& defaultThing) const;
  /// Return true if index < size().
  bool isValidIndex(ArrayIndex index) const;
  /// \brief Append value to array at the end.
  ///
  /// Equivalent to jsonvalue[jsonvalue.size()] = value;
  Thing& append(const Thing& value);

  /// Access an object value by name, create a null member if it does not exist.
  /// \note Because of our implementation, keys are limited to 2^30 -1 chars.
  ///  Exceeding that will cause an exception.
  Thing& operator[](const char* key);
  /// Access an object value by name, returns null if there is no member with
  /// that name.
  const Thing& operator[](const char* key) const;
  /// Access an object value by name, create a null member if it does not exist.
  /// \param key may contain embedded nulls.
  Thing& operator[](const String& key);
  /// Access an object value by name, returns null if there is no member with
  /// that name.
  /// \param key may contain embedded nulls.
  const Thing& operator[](const String& key) const;
  /// Return the member named key if it exist, defaultThing otherwise.
  /// \note deep copy
  Thing get(const char* key, const Thing& defaultThing) const;
  /// Return the member named key if it exist, defaultThing otherwise.
  /// \note deep copy
  /// \note key may contain embedded nulls.
  Thing get(const char* begin, const char* end, const Thing& defaultThing) const;
  /// Return the member named key if it exist, defaultThing otherwise.
  /// \note deep copy
  /// \param key may contain embedded nulls.
  Thing get(const String& key, const Thing& defaultThing) const;
  /// Most general and efficient version of isMember()const, get()const,
  /// and operator[]const
  /// \note As stated elsewhere, behavior is undefined if (end-begin) >= 2^30
  Thing const* find(char const* begin, char const* end) const;
  /// Most general and efficient version of object-mutators.
  /// \note As stated elsewhere, behavior is undefined if (end-begin) >= 2^30
  /// \return non-zero, but JSON_ASSERT if this is neither object nor nullThing.
  Thing const* demand(char const* begin, char const* end);
  /// \brief Remove and return the named member.
  ///
  /// Do nothing if it did not exist.
  /// \return the removed Thing, or null.
  /// \pre type() is objectThing or nullThing
  /// \post type() is unchanged
  /// \deprecated
  Thing removeMember(const char* key);
  /// Same as removeMember(const char*)
  /// \param key may contain embedded nulls.
  /// \deprecated
  Thing removeMember(const String& key);
  /// Same as removeMember(const char* begin, const char* end, Thing* removed),
  /// but 'key' is null-terminated.
  bool removeMember(const char* key, Thing* removed);
  /** \brief Remove the named map member.

      Update 'removed' iff removed.
      \param key may contain embedded nulls.
      \return true iff removed (no exceptions)
  */
  bool removeMember(String const& key, Thing* removed);
  /// Same as removeMember(String const& key, Thing* removed)
  bool removeMember(const char* begin, const char* end, Thing* removed);
  /** \brief Remove the indexed array element.

      O(n) expensive operations.
      Update 'removed' iff removed.
      \return true iff removed (no exceptions)
  */
  bool removeIndex(ArrayIndex i, Thing* removed);

  /// Return true if the object has a member named key.
  /// \note 'key' must be null-terminated.
  bool isMember(const char* key) const;
  /// Return true if the object has a member named key.
  /// \param key may contain embedded nulls.
  bool isMember(const String& key) const;
  /// Same as isMember(String const& key)const
  bool isMember(const char* begin, const char* end) const;

  /// \brief Return a list of the member names.
  ///
  /// If null, return an empty list.
  /// \pre type() is objectThing or nullThing
  /// \post if type() was nullThing, it remains nullThing
  Members getMemberNames() const;

  /// \deprecated Always pass len.
  JSONCPP_DEPRECATED("Use setComment(String const&) instead.")
  void setComment(const char* comment, CommentPlacement placement);
  /// Comments must be //... or /* ... */
  void setComment(const char* comment, size_t len, CommentPlacement placement);
  /// Comments must be //... or /* ... */
  void setComment(const String& comment, CommentPlacement placement);
  bool hasComment(CommentPlacement placement) const;
  /// Include delimiters and embedded newlines.
  String getComment(CommentPlacement placement) const;

  String toStyledString() const;

  const_iterator begin() const;
  const_iterator end() const;

  iterator begin();
  iterator end();

  // Accessors for the [start, limit) range of bytes within the JSON text from
  // which this value was parsed, if any.
  void setOffsetStart(ptrdiff_t start);
  void setOffsetLimit(ptrdiff_t limit);
  ptrdiff_t getOffsetStart() const;
  ptrdiff_t getOffsetLimit() const;
};

} // namespace Json


namespace std {
/// Specialize std::swap() for Json::Thing.
template<>
inline void swap(Json::Thing& a, Json::Thing& b) { a.swap(b); }
}


#if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
#pragma warning(pop)
#endif // if defined(JSONCPP_DISABLE_DLL_INTERFACE_WARNING)
