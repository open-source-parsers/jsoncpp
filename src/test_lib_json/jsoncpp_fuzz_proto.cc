// Copyright 2026 Google LLC and The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#include "json.pb.h"
#include "json_proto_converter.h"
#include "libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h"

#include <cstdint>
#include <json/config.h>
#include <json/json.h>
#include <memory>
#include <string>

void FuzzJson(std::string data_str, int32_t hash_settings) {
  Json::CharReaderBuilder builder;

  builder.settings_["failIfExtra"] = hash_settings & (1 << 0);
  builder.settings_["allowComments"] = hash_settings & (1 << 1);
  builder.settings_["strictRoot"] = hash_settings & (1 << 2);
  builder.settings_["allowDroppedNullPlaceholders"] = hash_settings & (1 << 3);
  builder.settings_["allowNumericKeys"] = hash_settings & (1 << 4);
  builder.settings_["allowSingleQuotes"] = hash_settings & (1 << 5);
  builder.settings_["rejectDupKeys"] = hash_settings & (1 << 6);
  builder.settings_["allowSpecialFloats"] = hash_settings & (1 << 7);
  builder.settings_["collectComments"] = hash_settings & (1 << 8);
  builder.settings_["allowTrailingCommas"] = hash_settings & (1 << 9);

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  
  const char* begin = data_str.c_str();
  const char* end = begin + data_str.length();

  Json::Value root;
#if JSON_USE_EXCEPTION
  try {
#endif  // JSON_USE_EXCEPTION
    reader->parse(begin, end, &root, nullptr);
#if JSON_USE_EXCEPTION
  } catch (Json::Exception const&) {
  }
#endif  // JSON_USE_EXCEPTION
}

DEFINE_PROTO_FUZZER(const json_proto::JsonParseAPI &json_proto) {
  json_proto::JsonProtoConverter converter;
  std::string data_str = converter.Convert(json_proto.object_value());
  int32_t hash_settings = json_proto.settings();
  FuzzJson(data_str, hash_settings);
}
