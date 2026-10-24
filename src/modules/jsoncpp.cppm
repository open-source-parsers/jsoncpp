// Copyright 2007-2010 Baptiste Lepilleur and The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

module;

#include <json/json.h>

export module jsoncpp;

export namespace Json {
    // writer.h
    using Json::StreamWriter;
    using Json::StreamWriterBuilder;
    using Json::Writer;
    using Json::FastWriter;
    using Json::StyledWriter;
    using Json::StyledStreamWriter;
    using Json::valueToString;
    using Json::valueToQuotedString;
    using Json::operator<<;

    // reader.h
    using Json::Reader;
    using Json::CharReader;
    using Json::CharReaderBuilder;
    using Json::parseFromStream;
    using Json::operator>>;

    // json_features.h
    using Json::Features;

    // value.h
    using Json::ArrayIndex;
    using Json::StaticString;
    using Json::Path;
    using Json::PathArgument;
    using Json::Value;
    using Json::ValueIteratorBase;
    using Json::ValueIterator;
    using Json::ValueConstIterator;
    using Json::ValueMembersView;
    using Json::ValueConstMembersView;

    // config.h
    using Json::version;
    using Json::Int;
    using Json::UInt;
    using Json::Int64;
    using Json::UInt64;
    using Json::LargestInt;
    using Json::LargestUInt;
    using Json::Allocator;
    using Json::String;
    using Json::IStringStream;
    using Json::OStringStream;
    using Json::IStream;
    using Json::OStream;

    // allocator.h
    using Json::SecureAllocator;
    using Json::operator==;
    using Json::operator!=;

    // value.h
    #if JSON_USE_EXCEPTION
    using Json::Exception;
    using Json::RuntimeError;
    using Json::LogicError;
    #endif
    using Json::ValueType;
    using Json::CommentPlacement;
    using Json::PrecisionType;
    using Json::MemberProxy;
    using Json::ConstMemberProxy;
    using Json::swap;
} // namespace Json
