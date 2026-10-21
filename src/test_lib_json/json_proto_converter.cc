// Copyright 2020 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "json_proto_converter.h"

#include <charconv>
#include <cmath>

namespace json_proto {

// Writes str as a quoted, JSON-escaped string, so that generated quotes,
// backslashes and control characters cannot break the surrounding structure.
void JsonProtoConverter::AppendEscapedString(const std::string& str) {
  static const char kHexDigits[] = "0123456789abcdef";
  data_ << '"';
  for (char c : str) {
    switch (c) {
      case '"':
        data_ << "\\\"";
        break;
      case '\\':
        data_ << "\\\\";
        break;
      case '\b':
        data_ << "\\b";
        break;
      case '\f':
        data_ << "\\f";
        break;
      case '\n':
        data_ << "\\n";
        break;
      case '\r':
        data_ << "\\r";
        break;
      case '\t':
        data_ << "\\t";
        break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          data_ << "\\u00" << kHexDigits[(c >> 4) & 0xF]
                << kHexDigits[c & 0xF];
        } else {
          data_ << c;
        }
        break;
    }
  }
  data_ << '"';
}

void JsonProtoConverter::AppendArray(const ArrayValue& array_value) {
  data_ << '[';
  bool need_comma = false;
  for (const auto& value : array_value.value()) {
    // Trailing comma inside of an array makes JSON invalid, avoid adding that.
    if (need_comma)
      data_ << ',';
    else
      need_comma = true;

    AppendValue(value);
  }
  data_ << ']';
}

// Writes value as a decimal number without an exponent: scientific notation
// would add a stray exponent to the generated number (e.g. "1e+20e3"), while
// std::fixed with the stream's default precision would truncate small values
// ("0.0001004" -> "0.000100"), collapsing distinct mutations. to_chars with
// chars_format::fixed writes the shortest fixed-point string that round-trips
// the value, avoiding both. Non-finite values have no decimal form; write 0
// instead ("inf"/"nan" text would not be a valid JSON token) so the rest of
// the generated structure remains parseable.
void JsonProtoConverter::AppendPlainFloat(float value) {
  if (!std::isfinite(value)) {
    data_ << '0';
    return;
  }
  char buf[96];  // shortest round-trip fixed form of a float fits comfortably
  const auto result =
      std::to_chars(buf, buf + sizeof(buf), value, std::chars_format::fixed);
  if (result.ec == std::errc()) {
    data_.write(buf, result.ptr - buf);
  } else {
    data_ << '0';
  }
}

void JsonProtoConverter::AppendNumber(const NumberValue& number_value) {
  if (number_value.has_float_value()) {
    const double value = number_value.float_value().value();
    // Keep "inf"/"nan" out of the output; they are not valid JSON numbers.
    data_ << (std::isfinite(value) ? value : 0.0);
  } else if (number_value.has_exponent_value()) {
    auto value = number_value.exponent_value();
    data_ << value.base();
    data_ << (value.use_uppercase() ? 'E' : 'e');
    data_ << value.exponent();
  } else if (number_value.has_exponent_frac_value()) {
    auto value = number_value.exponent_frac_value();
    AppendPlainFloat(value.base());
    data_ << (value.use_uppercase() ? 'E' : 'e');
    data_ << value.exponent();
  } else {
    data_ << number_value.integer_value().value();
  }
}

void JsonProtoConverter::AppendObject(const JsonObject& json_object) {
  data_ << '{';
  AppendEscapedString(json_object.name());
  data_ << ':';
  AppendValue(json_object.value());
  data_ << '}';
}

void JsonProtoConverter::AppendValue(const JsonValue& json_value) {
  if (json_value.has_object_value()) {
    AppendObject(json_value.object_value());
  } else if (json_value.has_array_value()) {
    AppendArray(json_value.array_value());
  } else if (json_value.has_number_value()) {
    AppendNumber(json_value.number_value());
  } else if (json_value.has_string_value()) {
    AppendEscapedString(json_value.string_value().value());
  } else if (json_value.has_boolean_value()) {
    data_ << (json_value.boolean_value().value() ? "true" : "false");
  } else {
    data_ << "null";
  }
}

std::string JsonProtoConverter::Convert(const JsonObject& json_object) {
  AppendObject(json_object);
  return data_.str();
}

std::string JsonProtoConverter::Convert(
    const json_proto::ArrayValue& json_array) {
  AppendArray(json_array);
  return data_.str();
}

}  // namespace json_proto
