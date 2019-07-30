// Copyright 2007-2019 The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#include "fuzz.h"

#include <sstream>
#include <string>

#include "json/config.h"
#include "json/features.h"
#include "json/reader.h"
#include "json/value.h"
#include "json/writer.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::string json_string(reinterpret_cast<const char *>(data), size);
  Json::Reader reader(Json::Features::strictMode());
  Json::Value value;
  const bool success = reader.parse(json_string, value, false);
  if (!success) {
    return 0;
  }

  // Write with StyledWriter
  Json::StyledWriter styled_writer;
  styled_writer.write(value);

  // Write with StyledStreamWriter
  Json::StyledStreamWriter styled_stream_writer;
  JSONCPP_OSTRINGSTREAM sstream;
  styled_stream_writer.write(sstream, value);
  return 0;
}
