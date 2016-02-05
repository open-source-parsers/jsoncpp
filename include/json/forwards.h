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
template<class _Value>
class FastWriter;
template<class _Value>
class StyledWriter;

// reader.h
template<class _Value>
class Reader;

// features.h
class Features;

// value.h
typedef unsigned int ArrayIndex;
class StaticString;
template<class _Value>
class Path;
template<class _Value>
class PathArgument;
template<template<class T> class _Alloc = std::allocator<char>,
  class _String = std::basic_string<char, std::char_traits<char>, std::allocator<char>>>
class Value;
template<class _Value>
class ValueIteratorBase;
template<class _Value>
class ValueIterator;
template<class _Value>
class ValueConstIterator;

} // namespace Json

#endif // JSON_FORWARDS_H_INCLUDED
