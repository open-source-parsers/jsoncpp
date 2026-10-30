// Copyright 2026 Google LLC and The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSON_PROTO_CONVERTER_H_
#define JSON_PROTO_CONVERTER_H_

#include <sstream>
#include <string>

#include "json.pb.h"

namespace json_proto {

class JsonProtoConverter {
 public:
  std::string Convert(const json_proto::JsonObject&);
  std::string Convert(const json_proto::ArrayValue&);

 private:
  std::stringstream data_;

  void AppendArray(const json_proto::ArrayValue&);
  void AppendEscapedString(const std::string& str);
  void AppendNumber(const json_proto::NumberValue&);
  void AppendPlainFloat(float value);
  void AppendObject(const json_proto::JsonObject&);
  void AppendValue(const json_proto::JsonValue&);
};

}  // namespace json_proto

#endif  // JSON_PROTO_CONVERTER_H_
