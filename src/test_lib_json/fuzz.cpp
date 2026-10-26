// Copyright 2007-2019 The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#include "fuzz.h"

#include <cstdint>
#include <cstdlib>
#include <json/config.h>
#include <json/json.h>
#include <memory>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  Json::CharReaderBuilder builder;

  if (size < sizeof(uint32_t)) {
    return 0;
  }

  const uint32_t hash_settings = static_cast<uint32_t>(data[0]) |
                                 (static_cast<uint32_t>(data[1]) << 8) |
                                 (static_cast<uint32_t>(data[2]) << 16) |
                                 (static_cast<uint32_t>(data[3]) << 24);
  data += sizeof(uint32_t);
  size -= sizeof(uint32_t);

  builder.settings_["failIfExtra"] = (hash_settings & (1 << 0)) != 0;
  builder.settings_["allowComments"] = (hash_settings & (1 << 1)) != 0;
  builder.settings_["strictRoot"] = (hash_settings & (1 << 2)) != 0;
  builder.settings_["allowDroppedNullPlaceholders"] =
      (hash_settings & (1 << 3)) != 0;
  builder.settings_["allowNumericKeys"] = (hash_settings & (1 << 4)) != 0;
  builder.settings_["allowSingleQuotes"] = (hash_settings & (1 << 5)) != 0;
  builder.settings_["skipBom"] = (hash_settings & (1 << 6)) != 0;
  builder.settings_["rejectDupKeys"] = (hash_settings & (1 << 7)) != 0;
  builder.settings_["allowSpecialFloats"] = (hash_settings & (1 << 8)) != 0;
  builder.settings_["collectComments"] = (hash_settings & (1 << 9)) != 0;
  builder.settings_["allowTrailingCommas"] = (hash_settings & (1 << 10)) != 0;
  // Bit 11 selects a small stack limit taken from the upper 16 bits, so that
  // the depth-limit error paths are reachable without huge inputs.
  if (hash_settings & (1 << 11)) {
    builder.settings_["stackLimit"] = (hash_settings >> 16) & 0x3FF;
  }

  // An unknown key is silently ignored by newCharReader(), so a typo here
  // would quietly stop the fuzzer from exercising that setting.
  if (!builder.validate(nullptr)) {
    std::abort();
  }

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

  Json::Value root;
  const auto data_str = reinterpret_cast<const char*>(data);
#if JSON_USE_EXCEPTION
  try {
#endif // JSON_USE_EXCEPTION
    reader->parse(data_str, data_str + size, &root, nullptr);
#if JSON_USE_EXCEPTION
  } catch (Json::Exception const&) {
  }
#endif // JSON_USE_EXCEPTION
  // Whether it succeeded or not doesn't matter.
  return 0;
}
