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

// Writes value in fixed-point notation. The stream would otherwise render
// large values in scientific notation ("1e+20"), which would add a stray
// exponent to the generated number (e.g. "1e+20e3"). Non-finite values would
// be written as "inf"/"nan" text; write 0 instead so the number always stays
// a valid JSON token and the rest of the generated structure remains
// parseable.
void JsonProtoConverter::AppendPlainFloat(float value) {
  if (!std::isfinite(value)) {
    data_ << '0';
    return;
  }
  data_ << std::fixed << value << std::defaultfloat;
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
