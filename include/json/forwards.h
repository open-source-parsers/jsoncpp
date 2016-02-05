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

// writer.h
template<typename _Traits, typename _Alloc>
class FastWriter;
template<typename _Traits, typename _Alloc>
class StyledWriter;

// reader_declaration.h
template<typename _Traits, typename _Alloc>
class Reader;

// features.h
class Features;

// value.h
typedef unsigned int ArrayIndex;
class StaticString;
template<typename _Traits, typename _Alloc>
class Path;
template<typename _Traits, typename _Alloc>
class PathArgument;
template<typename _Traits, typename _Alloc>
class Value;
template<typename _Traits, typename _Alloc>
class ValueIteratorBase;
template<typename _Traits, typename _Alloc>
class ValueIterator;
template<typename _Traits, typename _Alloc>
class ValueConstIterator;

} // namespace Json

#endif // JSON_FORWARDS_H_INCLUDED
