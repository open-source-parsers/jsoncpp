// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSON_FORWARDS_H_INCLUDED
#define JSON_FORWARDS_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include "config.h"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace Json {

// features.h
class Features;
class StaticString;
typedef unsigned int ArrayIndex;

namespace detail {

// writer.h
template<class _Value>
class FastWriter;
template<class _Value>
class StyledWriter;

// reader.h
template<class _Value>
class Reader;

// value.h
template<class _Value>
class Path;
template<class _Value>
class PathArgument;
template<class _Alloc, class _String>
class Value;
template<class _Value>
class ValueIteratorBase;
template<class _Value>
class ValueIterator;
template<class _Value>
class ValueConstIterator;

} // namespace detail
} // namespace Json

#endif // JSON_FORWARDS_H_INCLUDED
