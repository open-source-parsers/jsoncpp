// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef CPPTL_JSON_ASSERTIONS_H_INCLUDED
# define CPPTL_JSON_ASSERTIONS_H_INCLUDED

#include <stdlib.h>
#include <iostream>

#if !defined(JSON_IS_AMALGAMATION)
# include <json/config.h>
#endif // if !defined(JSON_IS_AMALGAMATION)

#if defined(JSON_USE_EXCEPTION)
#define JSON_ASSERT( condition ) assert( condition );  // @todo <= change this into an exception throw
#define JSON_FAIL_MESSAGE( message ) throw std::runtime_error( message );
#else  // defined(JSON_USE_EXCEPTION)
#define JSON_ASSERT( condition ) assert( condition );
#define JSON_FAIL_MESSAGE( message ) { std::cerr << std::endl << message << std::endl; exit(123); }
#endif

#define JSON_ASSERT_MESSAGE( condition, message ) if (!( condition )) { JSON_FAIL_MESSAGE( message ) }

#endif // CPPTL_JSON_ASSERTIONS_H_INCLUDED
