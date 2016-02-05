// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

// included by json_value.cpp

namespace Json {
namespace detail {

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueIteratorBase
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

template<class _Value>
ValueIteratorBase<_Value>::ValueIteratorBase()
    : current_(), isNull_(true) {
}

template<class _Value>
ValueIteratorBase<_Value>::ValueIteratorBase(
    const typename _Value::ObjectValues::iterator& current)
    : current_(current), isNull_(false) {}

template<class _Value>
_Value& ValueIteratorBase<_Value>::deref() const {
  return current_->second;
}

template<class _Value>
void ValueIteratorBase<_Value>::increment() {
  ++current_;
}

template<class _Value>
void ValueIteratorBase<_Value>::decrement() {
  --current_;
}

template<class _Value>
typename ValueIteratorBase<_Value>::difference_type
ValueIteratorBase<_Value>::computeDistance(const SelfType& other) const {
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
  for (typename _Value::ObjectValues::iterator it = current_; it != other.current_;
       ++it) {
    ++myDistance;
  }
  return myDistance;
#endif
}

template<class _Value>
bool ValueIteratorBase<_Value>::isEqual(const SelfType& other) const {
  if (isNull_) {
    return other.isNull_;
  }
  return current_ == other.current_;
}

template<class _Value>
void ValueIteratorBase<_Value>::copy(const SelfType& other) {
  current_ = other.current_;
  isNull_ = other.isNull_;
}

template<class _Value>
_Value ValueIteratorBase<_Value>::key() const {
  const typename _Value::CZString czstring = (*current_).first;
  if (czstring.data()) {
    if (czstring.isStaticString())
      return _Value(StaticString(czstring.data()));
    return _Value(czstring.data(), czstring.data() + czstring.length());
  }
  return _Value(czstring.index());
}

template<class _Value>
UInt ValueIteratorBase<_Value>::index() const {
  const typename _Value::CZString czstring = (*current_).first;
  if (!czstring.data())
    return czstring.index();
  return typename _Value::UInt(-1);
}

template<class _Value>
typename ValueIteratorBase<_Value>::String ValueIteratorBase<_Value>::name() const {
  char const* keey;
  char const* end;
  keey = memberName(&end);
  if (!keey) return String();
  return String(keey, end);
}

template<class _Value>
char const* ValueIteratorBase<_Value>::memberName() const {
  const char* cname = (*current_).first.data();
  return cname ? cname : "";
}

template<class _Value>
char const* ValueIteratorBase<_Value>::memberName(char const** end) const {
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

template<class _Value>
ValueConstIterator<_Value>::ValueConstIterator() {}

template<class _Value>
ValueConstIterator<_Value>::ValueConstIterator(
    const typename _Value::ObjectValues::iterator& current)
    : ValueIteratorBase<_Value>(current) {}

template<class _Value>
ValueConstIterator<_Value>::ValueConstIterator(ValueIterator<_Value> const& other)
    : ValueIteratorBase<_Value>(other) {}

template<class _Value>
ValueConstIterator<_Value>& ValueConstIterator<_Value>::
operator=(const ValueIteratorBase<_Value>& other) {
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

template<class _Value>
ValueIterator<_Value>::ValueIterator() {}

template<class _Value>
ValueIterator<_Value>::ValueIterator(const typename _Value::ObjectValues::iterator& current)
    : ValueIteratorBase<_Value>(current) {}

template<class _Value>
ValueIterator<_Value>::ValueIterator(const ValueConstIterator<_Value>& other)
    : ValueIteratorBase<_Value>(other) {
  throwRuntimeError("ConstIterator to Iterator should never be allowed.");
}

template<class _Value>
ValueIterator<_Value>::ValueIterator(const ValueIterator<_Value>& other)
    : ValueIteratorBase<_Value>(other) {}

template<class _Value>
ValueIterator<_Value>& ValueIterator<_Value>::operator=(const SelfType& other) {
  copy(other);
  return *this;
}

} // namespace detail
} // namespace Json
