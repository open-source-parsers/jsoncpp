// Copyright 2007-2010 Baptiste Lepilleur and The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSON_AUTOLINK_H_INCLUDED
#define JSON_AUTOLINK_H_INCLUDED

#include "config.h"

#if !defined(JSON_NO_AUTOLINK) && !defined(JSON_DLL_BUILD)
#define AUTOLINK_NAME "json"
#ifdef JSON_DLL
#define AUTOLINK_DLL
#endif
#include "autolink.h"
#endif

#endif // JSON_AUTOLINK_H_INCLUDED
