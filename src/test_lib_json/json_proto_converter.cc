// Copyright 2026 Google LLC and The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#include "json_proto_converter.h"

#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>

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
// the stream's default formatting would truncate small values
// ("0.0001004" -> "0.000100"), collapsing distinct mutations. Fixed notation
// with 50 digits after the decimal point round-trips every finite float
// (subnormal floats go down to ~1e-45). Non-finite values have no decimal
// form; write 0 instead ("inf"/"nan" text would not be a valid JSON token) so
// the rest of the generated structure remains parseable.
void JsonProtoConverter::AppendPlainFloat(float value) {
  if (!std::isfinite(value)) {
    data_ << '0';
    return;
  }
  // Format in a scratch stream so the fixed/precision state does not leak
  // into data_, and pin the classic ("C") locale so the decimal point is '.'.
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::fixed << std::setprecision(50) << value;
  std::string digits = out.str();
  // Trim redundant trailing zeros ("0.0001004000..." -> "0.0001004"), keeping
  // one digit after the decimal point so the token stays a float ("1.0").
  const std::size_t last_significant = digits.find_last_not_of('0');
  if (last_significant != std::string::npos) {
    digits.erase(last_significant + (digits[last_significant] == '.' ? 2 : 1));
  }
  data_ << digits;
}

void JsonProtoConverter::AppendNumber(const NumberValue& number_value) {
  if (number_value.has_float_value()) {
    const double value = number_value.float_value().value();
    // Keep "inf"/"nan" out of the output; they are not valid JSON numbers.
    // Print with max_digits10 significant digits (the stream default of 6
    // would collapse distinct generated doubles onto the same text). The
    // result may carry an exponent, which is fine: no exponent gets appended
    // to this value afterwards.
    if (!std::isfinite(value)) {
      data_ << '0';
    } else {
      std::ostringstream out;
      out.imbue(std::locale::classic());
      out << std::setprecision(std::numeric_limits<double>::max_digits10)
          << value;
      data_ << out.str();
    }
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
  bool need_comma = false;
  for (const JsonMember& member : json_object.member()) {
    // Trailing comma inside of an object makes JSON invalid, avoid adding it.
    if (need_comma)
      data_ << ',';
    else
      need_comma = true;

    AppendEscapedString(member.name());
    data_ << ':';
    AppendValue(member.value());
  }
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
