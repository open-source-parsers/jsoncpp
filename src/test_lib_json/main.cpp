// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#include "jsontest.h"
#include <json/config.h>
#include <json/json.h>
#include <cstring>
#include <limits>
#include <sstream>
#include <string>
#include <iomanip>

#define USING_TRAITS_ALLOC std::char_traits<char>, std::allocator<char>
using TestValue = Json::Value<USING_TRAITS_ALLOC>;
using TestValueIterator = Json::ValueIterator<USING_TRAITS_ALLOC>;
using TestStreamWriterBuilder = Json::StreamWriterBuilder<USING_TRAITS_ALLOC>;
using TestFastWriter = Json::FastWriter<USING_TRAITS_ALLOC>;
using TestReader = Json::Reader<USING_TRAITS_ALLOC>;
using TestCharReaderBuilder = Json::CharReaderBuilder<USING_TRAITS_ALLOC>;
using TestCharReader = Json::CharReader<USING_TRAITS_ALLOC>;

// Make numeric limits more convenient to talk about.
// Assumes int type in 32 bits.
#define kint32max TestValue::maxInt
#define kint32min TestValue::minInt
#define kuint32max TestValue::maxUInt
#define kint64max TestValue::maxInt64
#define kint64min TestValue::minInt64
#define kuint64max TestValue::maxUInt64

//static const double kdint64max = double(kint64max);
//static const float kfint64max = float(kint64max);
static const float kfint32max = float(kint32max);
static const float kfuint32max = float(kuint32max);

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Json Library test cases
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
static inline double uint64ToDouble(Json::UInt64 value) {
  return static_cast<double>(value);
}
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
static inline double uint64ToDouble(Json::UInt64 value) {
  return static_cast<double>(Json::Int64(value / 2)) * 2.0 +
         Json::Int64(value & 1);
}
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)

struct ValueTest : JsonTest::TestCase {
  TestValue null_;
  TestValue emptyArray_;
  TestValue emptyObject_;
  TestValue integer_;
  TestValue unsignedInteger_;
  TestValue smallUnsignedInteger_;
  TestValue real_;
  TestValue float_;
  TestValue array1_;
  TestValue object1_;
  TestValue emptyString_;
  TestValue string1_;
  TestValue string_;
  TestValue true_;
  TestValue false_;

  ValueTest()
      : emptyArray_(Json::arrayValue), emptyObject_(Json::objectValue),
        integer_(123456789), unsignedInteger_(34567890u),
        smallUnsignedInteger_(TestValue::UInt(TestValue::maxInt)),
        real_(1234.56789), float_(0.00390625f), emptyString_(""), string1_("a"),
        string_("sometext with space"), true_(true), false_(false) {
    array1_.append(1234);
    object1_["id"] = 1234;
  }

  struct IsCheck {
    /// Initialize all checks to \c false by default.
    IsCheck();

    bool isObject_;
    bool isArray_;
    bool isBool_;
    bool isString_;
    bool isNull_;

    bool isInt_;
    bool isInt64_;
    bool isUInt_;
    bool isUInt64_;
    bool isIntegral_;
    bool isDouble_;
    bool isNumeric_;
  };

  void checkConstMemberCount(const TestValue& value,
                             unsigned int expectedCount);

  void checkMemberCount(TestValue& value, unsigned int expectedCount);

  void checkIs(const TestValue& value, const IsCheck& check);

  void checkIsLess(const TestValue& x, const TestValue& y);

  void checkIsEqual(const TestValue& x, const TestValue& y);

  /// Normalize the representation of floating-point number by stripped leading
  /// 0 in exponent.
  static std::string normalizeFloatingPointStr(const std::string& s);
};

std::string ValueTest::normalizeFloatingPointStr(const std::string& s) {
  std::string::size_type index = s.find_last_of("eE");
  if (index != std::string::npos) {
    std::string::size_type hasSign =
        (s[index + 1] == '+' || s[index + 1] == '-') ? 1 : 0;
    std::string::size_type exponentStartIndex = index + 1 + hasSign;
    std::string normalized = s.substr(0, exponentStartIndex);
    std::string::size_type indexDigit =
        s.find_first_not_of('0', exponentStartIndex);
    std::string exponent = "0";
    if (indexDigit !=
        std::string::npos) // There is an exponent different from 0
    {
      exponent = s.substr(indexDigit);
    }
    return normalized + exponent;
  }
  return s;
}

JSONTEST_FIXTURE(ValueTest, checkNormalizeFloatingPointStr) {
  JSONTEST_ASSERT_STRING_EQUAL("0.0", normalizeFloatingPointStr("0.0"));
  JSONTEST_ASSERT_STRING_EQUAL("0e0", normalizeFloatingPointStr("0e0"));
  JSONTEST_ASSERT_STRING_EQUAL("1234.0", normalizeFloatingPointStr("1234.0"));
  JSONTEST_ASSERT_STRING_EQUAL("1234.0e0",
                               normalizeFloatingPointStr("1234.0e0"));
  JSONTEST_ASSERT_STRING_EQUAL("1234.0e+0",
                               normalizeFloatingPointStr("1234.0e+0"));
  JSONTEST_ASSERT_STRING_EQUAL("1234e-1", normalizeFloatingPointStr("1234e-1"));
  JSONTEST_ASSERT_STRING_EQUAL("1234e10", normalizeFloatingPointStr("1234e10"));
  JSONTEST_ASSERT_STRING_EQUAL("1234e10",
                               normalizeFloatingPointStr("1234e010"));
  JSONTEST_ASSERT_STRING_EQUAL("1234e+10",
                               normalizeFloatingPointStr("1234e+010"));
  JSONTEST_ASSERT_STRING_EQUAL("1234e-10",
                               normalizeFloatingPointStr("1234e-010"));
  JSONTEST_ASSERT_STRING_EQUAL("1234e+100",
                               normalizeFloatingPointStr("1234e+100"));
  JSONTEST_ASSERT_STRING_EQUAL("1234e-100",
                               normalizeFloatingPointStr("1234e-100"));
  JSONTEST_ASSERT_STRING_EQUAL("1234e+1",
                               normalizeFloatingPointStr("1234e+001"));
}

JSONTEST_FIXTURE(ValueTest, memberCount) {
  JSONTEST_ASSERT_PRED(checkMemberCount(emptyArray_, 0));
  JSONTEST_ASSERT_PRED(checkMemberCount(emptyObject_, 0));
  JSONTEST_ASSERT_PRED(checkMemberCount(array1_, 1));
  JSONTEST_ASSERT_PRED(checkMemberCount(object1_, 1));
  JSONTEST_ASSERT_PRED(checkMemberCount(null_, 0));
  JSONTEST_ASSERT_PRED(checkMemberCount(integer_, 0));
  JSONTEST_ASSERT_PRED(checkMemberCount(unsignedInteger_, 0));
  JSONTEST_ASSERT_PRED(checkMemberCount(smallUnsignedInteger_, 0));
  JSONTEST_ASSERT_PRED(checkMemberCount(real_, 0));
  JSONTEST_ASSERT_PRED(checkMemberCount(emptyString_, 0));
  JSONTEST_ASSERT_PRED(checkMemberCount(string_, 0));
  JSONTEST_ASSERT_PRED(checkMemberCount(true_, 0));
}

JSONTEST_FIXTURE(ValueTest, objects) {
  // Types
  IsCheck checks;
  checks.isObject_ = true;
  JSONTEST_ASSERT_PRED(checkIs(emptyObject_, checks));
  JSONTEST_ASSERT_PRED(checkIs(object1_, checks));

  JSONTEST_ASSERT_EQUAL(Json::objectValue, emptyObject_.type());

  // Empty object okay
  JSONTEST_ASSERT(emptyObject_.isConvertibleTo(Json::nullValue));

  // Non-empty object not okay
  JSONTEST_ASSERT(!object1_.isConvertibleTo(Json::nullValue));

  // Always okay
  JSONTEST_ASSERT(emptyObject_.isConvertibleTo(Json::objectValue));

  // Never okay
  JSONTEST_ASSERT(!emptyObject_.isConvertibleTo(Json::arrayValue));
  JSONTEST_ASSERT(!emptyObject_.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!emptyObject_.isConvertibleTo(Json::uintValue));
  JSONTEST_ASSERT(!emptyObject_.isConvertibleTo(Json::realValue));
  JSONTEST_ASSERT(!emptyObject_.isConvertibleTo(Json::booleanValue));
  JSONTEST_ASSERT(!emptyObject_.isConvertibleTo(Json::stringValue));

  // Access through const reference
  const TestValue& constObject = object1_;

  JSONTEST_ASSERT_EQUAL(TestValue(1234), constObject["id"]);
  JSONTEST_ASSERT_EQUAL(TestValue(), constObject["unknown id"]);

  // Access through non-const reference
  JSONTEST_ASSERT_EQUAL(TestValue(1234), object1_["id"]);
  JSONTEST_ASSERT_EQUAL(TestValue(), object1_["unknown id"]);

  object1_["some other id"] = "foo";
  JSONTEST_ASSERT_EQUAL(TestValue("foo"), object1_["some other id"]);
  JSONTEST_ASSERT_EQUAL(TestValue("foo"), object1_["some other id"]);

  // Remove.
  TestValue got;
  bool did;
  did = object1_.removeMember("some other id", &got);
  JSONTEST_ASSERT_EQUAL(TestValue("foo"), got);
  JSONTEST_ASSERT_EQUAL(true, did);
  got = TestValue("bar");
  did = object1_.removeMember("some other id", &got);
  JSONTEST_ASSERT_EQUAL(TestValue("bar"), got);
  JSONTEST_ASSERT_EQUAL(false, did);
}

JSONTEST_FIXTURE(ValueTest, arrays) {
  const unsigned int index0 = 0;

  // Types
  IsCheck checks;
  checks.isArray_ = true;
  JSONTEST_ASSERT_PRED(checkIs(emptyArray_, checks));
  JSONTEST_ASSERT_PRED(checkIs(array1_, checks));

  JSONTEST_ASSERT_EQUAL(Json::arrayValue, array1_.type());

  // Empty array okay
  JSONTEST_ASSERT(emptyArray_.isConvertibleTo(Json::nullValue));

  // Non-empty array not okay
  JSONTEST_ASSERT(!array1_.isConvertibleTo(Json::nullValue));

  // Always okay
  JSONTEST_ASSERT(emptyArray_.isConvertibleTo(Json::arrayValue));

  // Never okay
  JSONTEST_ASSERT(!emptyArray_.isConvertibleTo(Json::objectValue));
  JSONTEST_ASSERT(!emptyArray_.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!emptyArray_.isConvertibleTo(Json::uintValue));
  JSONTEST_ASSERT(!emptyArray_.isConvertibleTo(Json::realValue));
  JSONTEST_ASSERT(!emptyArray_.isConvertibleTo(Json::booleanValue));
  JSONTEST_ASSERT(!emptyArray_.isConvertibleTo(Json::stringValue));

  // Access through const reference
  const TestValue& constArray = array1_;
  JSONTEST_ASSERT_EQUAL(TestValue(1234), constArray[index0]);
  JSONTEST_ASSERT_EQUAL(TestValue(1234), constArray[0]);

  // Access through non-const reference
  JSONTEST_ASSERT_EQUAL(TestValue(1234), array1_[index0]);
  JSONTEST_ASSERT_EQUAL(TestValue(1234), array1_[0]);

  array1_[2] = TestValue(17);
  JSONTEST_ASSERT_EQUAL(TestValue(), array1_[1]);
  JSONTEST_ASSERT_EQUAL(TestValue(17), array1_[2]);
  TestValue got;
  JSONTEST_ASSERT_EQUAL(true, array1_.removeIndex(2, &got));
  JSONTEST_ASSERT_EQUAL(TestValue(17), got);
  JSONTEST_ASSERT_EQUAL(false, array1_.removeIndex(2, &got)); // gone now
}
JSONTEST_FIXTURE(ValueTest, arrayIssue252)
{
  int count = 5;
  TestValue root;
  TestValue item;
  root["array"] = TestValue::nullRef;
  for (int i = 0; i < count; i++)
  {
    item["a"] = i;
    item["b"] = i;
    root["array"][i] = item;
  }
  //JSONTEST_ASSERT_EQUAL(5, root["array"].size());
}

JSONTEST_FIXTURE(ValueTest, null) {
  JSONTEST_ASSERT_EQUAL(Json::nullValue, null_.type());

  IsCheck checks;
  checks.isNull_ = true;
  JSONTEST_ASSERT_PRED(checkIs(null_, checks));

  JSONTEST_ASSERT(null_.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(null_.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(null_.isConvertibleTo(Json::uintValue));
  JSONTEST_ASSERT(null_.isConvertibleTo(Json::realValue));
  JSONTEST_ASSERT(null_.isConvertibleTo(Json::booleanValue));
  JSONTEST_ASSERT(null_.isConvertibleTo(Json::stringValue));
  JSONTEST_ASSERT(null_.isConvertibleTo(Json::arrayValue));
  JSONTEST_ASSERT(null_.isConvertibleTo(Json::objectValue));

  JSONTEST_ASSERT_EQUAL(Json::Int(0), null_.asInt());
  JSONTEST_ASSERT_EQUAL(Json::LargestInt(0), null_.asLargestInt());
  JSONTEST_ASSERT_EQUAL(Json::UInt(0), null_.asUInt());
  JSONTEST_ASSERT_EQUAL(Json::LargestUInt(0), null_.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(0.0, null_.asDouble());
  JSONTEST_ASSERT_EQUAL(0.0, null_.asFloat());
  JSONTEST_ASSERT_STRING_EQUAL("", null_.asString());

  JSONTEST_ASSERT_EQUAL(TestValue::null, null_);
}

JSONTEST_FIXTURE(ValueTest, strings) {
  JSONTEST_ASSERT_EQUAL(Json::stringValue, string1_.type());

  IsCheck checks;
  checks.isString_ = true;
  JSONTEST_ASSERT_PRED(checkIs(emptyString_, checks));
  JSONTEST_ASSERT_PRED(checkIs(string_, checks));
  JSONTEST_ASSERT_PRED(checkIs(string1_, checks));

  // Empty string okay
  JSONTEST_ASSERT(emptyString_.isConvertibleTo(Json::nullValue));

  // Non-empty string not okay
  JSONTEST_ASSERT(!string1_.isConvertibleTo(Json::nullValue));

  // Always okay
  JSONTEST_ASSERT(string1_.isConvertibleTo(Json::stringValue));

  // Never okay
  JSONTEST_ASSERT(!string1_.isConvertibleTo(Json::objectValue));
  JSONTEST_ASSERT(!string1_.isConvertibleTo(Json::arrayValue));
  JSONTEST_ASSERT(!string1_.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!string1_.isConvertibleTo(Json::uintValue));
  JSONTEST_ASSERT(!string1_.isConvertibleTo(Json::realValue));

  JSONTEST_ASSERT_STRING_EQUAL("a", string1_.asString());
  JSONTEST_ASSERT_STRING_EQUAL("a", string1_.asCString());
}

JSONTEST_FIXTURE(ValueTest, bools) {
  JSONTEST_ASSERT_EQUAL(Json::booleanValue, false_.type());

  IsCheck checks;
  checks.isBool_ = true;
  JSONTEST_ASSERT_PRED(checkIs(false_, checks));
  JSONTEST_ASSERT_PRED(checkIs(true_, checks));

  // False okay
  JSONTEST_ASSERT(false_.isConvertibleTo(Json::nullValue));

  // True not okay
  JSONTEST_ASSERT(!true_.isConvertibleTo(Json::nullValue));

  // Always okay
  JSONTEST_ASSERT(true_.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(true_.isConvertibleTo(Json::uintValue));
  JSONTEST_ASSERT(true_.isConvertibleTo(Json::realValue));
  JSONTEST_ASSERT(true_.isConvertibleTo(Json::booleanValue));
  JSONTEST_ASSERT(true_.isConvertibleTo(Json::stringValue));

  // Never okay
  JSONTEST_ASSERT(!true_.isConvertibleTo(Json::arrayValue));
  JSONTEST_ASSERT(!true_.isConvertibleTo(Json::objectValue));

  JSONTEST_ASSERT_EQUAL(true, true_.asBool());
  JSONTEST_ASSERT_EQUAL(1, true_.asInt());
  JSONTEST_ASSERT_EQUAL(1, true_.asLargestInt());
  JSONTEST_ASSERT_EQUAL(1, true_.asUInt());
  JSONTEST_ASSERT_EQUAL(1, true_.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(1.0, true_.asDouble());
  JSONTEST_ASSERT_EQUAL(1.0, true_.asFloat());

  JSONTEST_ASSERT_EQUAL(false, false_.asBool());
  JSONTEST_ASSERT_EQUAL(0, false_.asInt());
  JSONTEST_ASSERT_EQUAL(0, false_.asLargestInt());
  JSONTEST_ASSERT_EQUAL(0, false_.asUInt());
  JSONTEST_ASSERT_EQUAL(0, false_.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(0.0, false_.asDouble());
  JSONTEST_ASSERT_EQUAL(0.0, false_.asFloat());
}

JSONTEST_FIXTURE(ValueTest, integers) {
  IsCheck checks;
  TestValue val;

  // Conversions that don't depend on the value.
  JSONTEST_ASSERT(TestValue(17).isConvertibleTo(Json::realValue));
  JSONTEST_ASSERT(TestValue(17).isConvertibleTo(Json::stringValue));
  JSONTEST_ASSERT(TestValue(17).isConvertibleTo(Json::booleanValue));
  JSONTEST_ASSERT(!TestValue(17).isConvertibleTo(Json::arrayValue));
  JSONTEST_ASSERT(!TestValue(17).isConvertibleTo(Json::objectValue));

  JSONTEST_ASSERT(TestValue(17U).isConvertibleTo(Json::realValue));
  JSONTEST_ASSERT(TestValue(17U).isConvertibleTo(Json::stringValue));
  JSONTEST_ASSERT(TestValue(17U).isConvertibleTo(Json::booleanValue));
  JSONTEST_ASSERT(!TestValue(17U).isConvertibleTo(Json::arrayValue));
  JSONTEST_ASSERT(!TestValue(17U).isConvertibleTo(Json::objectValue));

  JSONTEST_ASSERT(TestValue(17.0).isConvertibleTo(Json::realValue));
  JSONTEST_ASSERT(TestValue(17.0).isConvertibleTo(Json::stringValue));
  JSONTEST_ASSERT(TestValue(17.0).isConvertibleTo(Json::booleanValue));
  JSONTEST_ASSERT(!TestValue(17.0).isConvertibleTo(Json::arrayValue));
  JSONTEST_ASSERT(!TestValue(17.0).isConvertibleTo(Json::objectValue));

  // Default int
  val = TestValue(Json::intValue);

  JSONTEST_ASSERT_EQUAL(Json::intValue, val.type());

  checks = IsCheck();
  checks.isInt_ = true;
  checks.isInt64_ = true;
  checks.isUInt_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(0, val.asInt());
  JSONTEST_ASSERT_EQUAL(0, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(0, val.asUInt());
  JSONTEST_ASSERT_EQUAL(0, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(0.0, val.asDouble());
  JSONTEST_ASSERT_EQUAL(0.0, val.asFloat());
  JSONTEST_ASSERT_EQUAL(false, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("0", val.asString());

  // Default uint
  val = TestValue(Json::uintValue);

  JSONTEST_ASSERT_EQUAL(Json::uintValue, val.type());

  checks = IsCheck();
  checks.isInt_ = true;
  checks.isInt64_ = true;
  checks.isUInt_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(0, val.asInt());
  JSONTEST_ASSERT_EQUAL(0, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(0, val.asUInt());
  JSONTEST_ASSERT_EQUAL(0, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(0.0, val.asDouble());
  JSONTEST_ASSERT_EQUAL(0.0, val.asFloat());
  JSONTEST_ASSERT_EQUAL(false, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("0", val.asString());

  // Default real
  val = TestValue(Json::realValue);

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  JSONTEST_ASSERT(val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));

  checks = IsCheck();
  checks.isInt_ = true;
  checks.isInt64_ = true;
  checks.isUInt_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT_EQUAL(0, val.asInt());
  JSONTEST_ASSERT_EQUAL(0, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(0, val.asUInt());
  JSONTEST_ASSERT_EQUAL(0, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(0.0, val.asDouble());
  JSONTEST_ASSERT_EQUAL(0.0, val.asFloat());
  JSONTEST_ASSERT_EQUAL(false, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("0", val.asString());

  // Zero (signed constructor arg)
  val = TestValue(0);

  JSONTEST_ASSERT_EQUAL(Json::intValue, val.type());

  checks = IsCheck();
  checks.isInt_ = true;
  checks.isInt64_ = true;
  checks.isUInt_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(0, val.asInt());
  JSONTEST_ASSERT_EQUAL(0, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(0, val.asUInt());
  JSONTEST_ASSERT_EQUAL(0, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(0.0, val.asDouble());
  JSONTEST_ASSERT_EQUAL(0.0, val.asFloat());
  JSONTEST_ASSERT_EQUAL(false, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("0", val.asString());

  // Zero (unsigned constructor arg)
  val = TestValue(0u);

  JSONTEST_ASSERT_EQUAL(Json::uintValue, val.type());

  checks = IsCheck();
  checks.isInt_ = true;
  checks.isInt64_ = true;
  checks.isUInt_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(0, val.asInt());
  JSONTEST_ASSERT_EQUAL(0, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(0, val.asUInt());
  JSONTEST_ASSERT_EQUAL(0, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(0.0, val.asDouble());
  JSONTEST_ASSERT_EQUAL(0.0, val.asFloat());
  JSONTEST_ASSERT_EQUAL(false, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("0", val.asString());

  // Zero (floating-point constructor arg)
  val = TestValue(0.0);

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isInt_ = true;
  checks.isInt64_ = true;
  checks.isUInt_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(0, val.asInt());
  JSONTEST_ASSERT_EQUAL(0, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(0, val.asUInt());
  JSONTEST_ASSERT_EQUAL(0, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(0.0, val.asDouble());
  JSONTEST_ASSERT_EQUAL(0.0, val.asFloat());
  JSONTEST_ASSERT_EQUAL(false, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("0", val.asString());

  // 2^20 (signed constructor arg)
  val = TestValue(1 << 20);

  JSONTEST_ASSERT_EQUAL(Json::intValue, val.type());
  checks = IsCheck();
  checks.isInt_ = true;
  checks.isInt64_ = true;
  checks.isUInt_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL((1 << 20), val.asInt());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asLargestInt());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asUInt());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asDouble());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("1048576", val.asString());

  // 2^20 (unsigned constructor arg)
  val = TestValue(Json::UInt(1 << 20));

  JSONTEST_ASSERT_EQUAL(Json::uintValue, val.type());

  checks = IsCheck();
  checks.isInt_ = true;
  checks.isInt64_ = true;
  checks.isUInt_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL((1 << 20), val.asInt());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asLargestInt());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asUInt());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asDouble());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("1048576", val.asString());

  // 2^20 (floating-point constructor arg)
  val = TestValue((1 << 20) / 1.0);

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isInt_ = true;
  checks.isInt64_ = true;
  checks.isUInt_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL((1 << 20), val.asInt());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asLargestInt());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asUInt());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asDouble());
  JSONTEST_ASSERT_EQUAL((1 << 20), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("1048576",
                               normalizeFloatingPointStr(val.asString()));

  // -2^20
  val = TestValue(-(1 << 20));

  JSONTEST_ASSERT_EQUAL(Json::intValue, val.type());

  checks = IsCheck();
  checks.isInt_ = true;
  checks.isInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(-(1 << 20), val.asInt());
  JSONTEST_ASSERT_EQUAL(-(1 << 20), val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(-(1 << 20), val.asDouble());
  JSONTEST_ASSERT_EQUAL(-(1 << 20), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("-1048576", val.asString());

  // int32 max
  val = TestValue(kint32max);

  JSONTEST_ASSERT_EQUAL(Json::intValue, val.type());

  checks = IsCheck();
  checks.isInt_ = true;
  checks.isInt64_ = true;
  checks.isUInt_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(kint32max, val.asInt());
  JSONTEST_ASSERT_EQUAL(kint32max, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(kint32max, val.asUInt());
  JSONTEST_ASSERT_EQUAL(kint32max, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(kint32max, val.asDouble());
  JSONTEST_ASSERT_EQUAL(kfint32max, val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("2147483647", val.asString());

  // int32 min
  val = TestValue(kint32min);

  JSONTEST_ASSERT_EQUAL(Json::intValue, val.type());

  checks = IsCheck();
  checks.isInt_ = true;
  checks.isInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(kint32min, val.asInt());
  JSONTEST_ASSERT_EQUAL(kint32min, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(kint32min, val.asDouble());
  JSONTEST_ASSERT_EQUAL(kint32min, val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("-2147483648", val.asString());

  // uint32 max
  val = TestValue(kuint32max);

  JSONTEST_ASSERT_EQUAL(Json::uintValue, val.type());

  checks = IsCheck();
  checks.isInt64_ = true;
  checks.isUInt_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));

#ifndef JSON_NO_INT64
  JSONTEST_ASSERT_EQUAL(kuint32max, val.asLargestInt());
#endif
  JSONTEST_ASSERT_EQUAL(kuint32max, val.asUInt());
  JSONTEST_ASSERT_EQUAL(kuint32max, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(kuint32max, val.asDouble());
  JSONTEST_ASSERT_EQUAL(kfuint32max, val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("4294967295", val.asString());

#ifdef JSON_NO_INT64
  // int64 max
  val = TestValue(double(kint64max));

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(double(kint64max), val.asDouble());
  JSONTEST_ASSERT_EQUAL(float(kint64max), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("9.22337e+18", val.asString());

  // int64 min
  val = TestValue(double(kint64min));

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(double(kint64min), val.asDouble());
  JSONTEST_ASSERT_EQUAL(float(kint64min), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("-9.22337e+18", val.asString());

  // uint64 max
  val = TestValue(double(kuint64max));

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(double(kuint64max), val.asDouble());
  JSONTEST_ASSERT_EQUAL(float(kuint64max), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("1.84467e+19", val.asString());
#else // ifdef JSON_NO_INT64
  // 2^40 (signed constructor arg)
  val = TestValue(Json::Int64(1) << 40);

  JSONTEST_ASSERT_EQUAL(Json::intValue, val.type());

  checks = IsCheck();
  checks.isInt64_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asInt64());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asLargestInt());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asUInt64());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asDouble());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("1099511627776", val.asString());

  // 2^40 (unsigned constructor arg)
  val = TestValue(Json::UInt64(1) << 40);

  JSONTEST_ASSERT_EQUAL(Json::uintValue, val.type());

  checks = IsCheck();
  checks.isInt64_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asInt64());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asLargestInt());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asUInt64());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asDouble());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("1099511627776", val.asString());

  // 2^40 (floating-point constructor arg)
  val = TestValue((Json::Int64(1) << 40) / 1.0);

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isInt64_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asInt64());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asLargestInt());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asUInt64());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asDouble());
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 40), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("1099511627776",
                               normalizeFloatingPointStr(val.asString()));

  // -2^40
  val = TestValue(-(Json::Int64(1) << 40));

  JSONTEST_ASSERT_EQUAL(Json::intValue, val.type());

  checks = IsCheck();
  checks.isInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(-(Json::Int64(1) << 40), val.asInt64());
  JSONTEST_ASSERT_EQUAL(-(Json::Int64(1) << 40), val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(-(Json::Int64(1) << 40), val.asDouble());
  JSONTEST_ASSERT_EQUAL(-(Json::Int64(1) << 40), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("-1099511627776", val.asString());

  // int64 max
  val = TestValue(Json::Int64(kint64max));

  JSONTEST_ASSERT_EQUAL(Json::intValue, val.type());

  checks = IsCheck();
  checks.isInt64_ = true;
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(kint64max, val.asInt64());
  JSONTEST_ASSERT_EQUAL(kint64max, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(kint64max, val.asUInt64());
  JSONTEST_ASSERT_EQUAL(kint64max, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(double(kint64max), val.asDouble());
  JSONTEST_ASSERT_EQUAL(float(kint64max), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("9223372036854775807", val.asString());

  // int64 max (floating point constructor). Note that kint64max is not exactly
  // representable as a double, and will be rounded up to be higher.
  val = TestValue(double(kint64max));

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(Json::UInt64(1) << 63, val.asUInt64());
  JSONTEST_ASSERT_EQUAL(Json::UInt64(1) << 63, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(uint64ToDouble(Json::UInt64(1) << 63), val.asDouble());
  JSONTEST_ASSERT_EQUAL(float(uint64ToDouble(Json::UInt64(1) << 63)),
                        val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("9.2233720368547758e+18",
                               normalizeFloatingPointStr(val.asString()));

  // int64 min
  val = TestValue(Json::Int64(kint64min));

  JSONTEST_ASSERT_EQUAL(Json::intValue, val.type());

  checks = IsCheck();
  checks.isInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(kint64min, val.asInt64());
  JSONTEST_ASSERT_EQUAL(kint64min, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(double(kint64min), val.asDouble());
  JSONTEST_ASSERT_EQUAL(float(kint64min), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("-9223372036854775808", val.asString());

  // int64 min (floating point constructor). Note that kint64min *is* exactly
  // representable as a double.
  val = TestValue(double(kint64min));

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(kint64min, val.asInt64());
  JSONTEST_ASSERT_EQUAL(kint64min, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(-9223372036854775808.0, val.asDouble());
  JSONTEST_ASSERT_EQUAL(-9223372036854775808.0, val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("-9.2233720368547758e+18",
                               normalizeFloatingPointStr(val.asString()));

  // 10^19
  const Json::UInt64 ten_to_19 = static_cast<Json::UInt64>(1e19);
  val = TestValue(Json::UInt64(ten_to_19));

  JSONTEST_ASSERT_EQUAL(Json::uintValue, val.type());

  checks = IsCheck();
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(ten_to_19, val.asUInt64());
  JSONTEST_ASSERT_EQUAL(ten_to_19, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(uint64ToDouble(ten_to_19), val.asDouble());
  JSONTEST_ASSERT_EQUAL(float(uint64ToDouble(ten_to_19)), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("10000000000000000000", val.asString());

  // 10^19 (double constructor). Note that 10^19 is not exactly representable
  // as a double.
  val = TestValue(uint64ToDouble(ten_to_19));

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(1e19, val.asDouble());
  JSONTEST_ASSERT_EQUAL(1e19, val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("1e+19",
                               normalizeFloatingPointStr(val.asString()));

  // uint64 max
  val = TestValue(Json::UInt64(kuint64max));

  JSONTEST_ASSERT_EQUAL(Json::uintValue, val.type());

  checks = IsCheck();
  checks.isUInt64_ = true;
  checks.isIntegral_ = true;
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(kuint64max, val.asUInt64());
  JSONTEST_ASSERT_EQUAL(kuint64max, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(uint64ToDouble(kuint64max), val.asDouble());
  JSONTEST_ASSERT_EQUAL(float(uint64ToDouble(kuint64max)), val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("18446744073709551615", val.asString());

  // uint64 max (floating point constructor). Note that kuint64max is not
  // exactly representable as a double, and will be rounded up to be higher.
  val = TestValue(uint64ToDouble(kuint64max));

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));

  JSONTEST_ASSERT_EQUAL(18446744073709551616.0, val.asDouble());
  JSONTEST_ASSERT_EQUAL(18446744073709551616.0, val.asFloat());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_STRING_EQUAL("1.8446744073709552e+19",
                               normalizeFloatingPointStr(val.asString()));
#endif
}

JSONTEST_FIXTURE(ValueTest, nonIntegers) {
  IsCheck checks;
  TestValue val;

  // Small positive number
  val = TestValue(1.5);

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::realValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::booleanValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::stringValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::arrayValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::objectValue));

  JSONTEST_ASSERT_EQUAL(1.5, val.asDouble());
  JSONTEST_ASSERT_EQUAL(1.5, val.asFloat());
  JSONTEST_ASSERT_EQUAL(1, val.asInt());
  JSONTEST_ASSERT_EQUAL(1, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(1, val.asUInt());
  JSONTEST_ASSERT_EQUAL(1, val.asLargestUInt());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_EQUAL("1.5", val.asString());

  // Small negative number
  val = TestValue(-1.5);

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::realValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::booleanValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::stringValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::arrayValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::objectValue));

  JSONTEST_ASSERT_EQUAL(-1.5, val.asDouble());
  JSONTEST_ASSERT_EQUAL(-1.5, val.asFloat());
  JSONTEST_ASSERT_EQUAL(-1, val.asInt());
  JSONTEST_ASSERT_EQUAL(-1, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_EQUAL("-1.5", val.asString());

  // A bit over int32 max
  val = TestValue(kint32max + 0.5);

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(val.isConvertibleTo(Json::uintValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::realValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::booleanValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::stringValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::arrayValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::objectValue));

  JSONTEST_ASSERT_EQUAL(2147483647.5, val.asDouble());
  JSONTEST_ASSERT_EQUAL(float(2147483647.5), val.asFloat());
  JSONTEST_ASSERT_EQUAL(2147483647U, val.asUInt());
#ifdef JSON_HAS_INT64
  JSONTEST_ASSERT_EQUAL(2147483647L, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL(2147483647U, val.asLargestUInt());
#endif
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_EQUAL("2147483647.5",
                        normalizeFloatingPointStr(val.asString()));

  // A bit under int32 min
  val = TestValue(kint32min - 0.5);

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(val.isConvertibleTo(Json::realValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::booleanValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::stringValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::arrayValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::objectValue));

  JSONTEST_ASSERT_EQUAL(-2147483648.5, val.asDouble());
  JSONTEST_ASSERT_EQUAL(float(-2147483648.5), val.asFloat());
#ifdef JSON_HAS_INT64
  JSONTEST_ASSERT_EQUAL(-(Json::Int64(1) << 31), val.asLargestInt());
#endif
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_EQUAL("-2147483648.5",
                        normalizeFloatingPointStr(val.asString()));

  // A bit over uint32 max
  val = TestValue(kuint32max + 0.5);

  JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

  checks = IsCheck();
  checks.isDouble_ = true;
  checks.isNumeric_ = true;
  JSONTEST_ASSERT_PRED(checkIs(val, checks));

  JSONTEST_ASSERT(val.isConvertibleTo(Json::realValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::booleanValue));
  JSONTEST_ASSERT(val.isConvertibleTo(Json::stringValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::nullValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::intValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::uintValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::arrayValue));
  JSONTEST_ASSERT(!val.isConvertibleTo(Json::objectValue));

  JSONTEST_ASSERT_EQUAL(4294967295.5, val.asDouble());
  JSONTEST_ASSERT_EQUAL(float(4294967295.5), val.asFloat());
#ifdef JSON_HAS_INT64
  JSONTEST_ASSERT_EQUAL((Json::Int64(1) << 32) - 1, val.asLargestInt());
  JSONTEST_ASSERT_EQUAL((Json::UInt64(1) << 32) - Json::UInt64(1),
                        val.asLargestUInt());
#endif
  JSONTEST_ASSERT_EQUAL(true, val.asBool());
  JSONTEST_ASSERT_EQUAL("4294967295.5",
                        normalizeFloatingPointStr(val.asString()));

  val = TestValue(1.2345678901234);
  JSONTEST_ASSERT_STRING_EQUAL("1.2345678901234001",
                               normalizeFloatingPointStr(val.asString()));

  // A 16-digit floating point number.
  val = TestValue(2199023255552000.0f);
  JSONTEST_ASSERT_EQUAL(float(2199023255552000.0f), val.asFloat());
  JSONTEST_ASSERT_STRING_EQUAL("2199023255552000",
                               normalizeFloatingPointStr(val.asString()));

  // A very large floating point number.
  val = TestValue(3.402823466385289e38);
  JSONTEST_ASSERT_EQUAL(float(3.402823466385289e38), val.asFloat());
  JSONTEST_ASSERT_STRING_EQUAL("3.402823466385289e+38",
                               normalizeFloatingPointStr(val.asString()));

  // An even larger floating point number.
  val = TestValue(1.2345678e300);
  JSONTEST_ASSERT_EQUAL(double(1.2345678e300), val.asDouble());
  JSONTEST_ASSERT_STRING_EQUAL("1.2345678e+300",
                               normalizeFloatingPointStr(val.asString()));
}

void ValueTest::checkConstMemberCount(const TestValue& value,
                                      unsigned int expectedCount) {
  unsigned int count = 0;
  TestValue::const_iterator itEnd = value.end();
  for (TestValue::const_iterator it = value.begin(); it != itEnd; ++it) {
    ++count;
  }
  JSONTEST_ASSERT_EQUAL(expectedCount, count) << "TestValue::const_iterator";
}

void ValueTest::checkMemberCount(TestValue& value,
                                 unsigned int expectedCount) {
  JSONTEST_ASSERT_EQUAL(expectedCount, value.size());

  unsigned int count = 0;
  TestValue::iterator itEnd = value.end();
  for (TestValue::iterator it = value.begin(); it != itEnd; ++it) {
    ++count;
  }
  JSONTEST_ASSERT_EQUAL(expectedCount, count) << "TestValue::iterator";

  JSONTEST_ASSERT_PRED(checkConstMemberCount(value, expectedCount));
}

ValueTest::IsCheck::IsCheck()
    : isObject_(false), isArray_(false), isBool_(false), isString_(false),
      isNull_(false), isInt_(false), isInt64_(false), isUInt_(false),
      isUInt64_(false), isIntegral_(false), isDouble_(false),
      isNumeric_(false) {}

void ValueTest::checkIs(const TestValue& value, const IsCheck& check) {
  JSONTEST_ASSERT_EQUAL(check.isObject_, value.isObject());
  JSONTEST_ASSERT_EQUAL(check.isArray_, value.isArray());
  JSONTEST_ASSERT_EQUAL(check.isBool_, value.isBool());
  JSONTEST_ASSERT_EQUAL(check.isDouble_, value.isDouble());
  JSONTEST_ASSERT_EQUAL(check.isInt_, value.isInt());
  JSONTEST_ASSERT_EQUAL(check.isUInt_, value.isUInt());
  JSONTEST_ASSERT_EQUAL(check.isIntegral_, value.isIntegral());
  JSONTEST_ASSERT_EQUAL(check.isNumeric_, value.isNumeric());
  JSONTEST_ASSERT_EQUAL(check.isString_, value.isString());
  JSONTEST_ASSERT_EQUAL(check.isNull_, value.isNull());

#ifdef JSON_HAS_INT64
  JSONTEST_ASSERT_EQUAL(check.isInt64_, value.isInt64());
  JSONTEST_ASSERT_EQUAL(check.isUInt64_, value.isUInt64());
#else
  JSONTEST_ASSERT_EQUAL(false, value.isInt64());
  JSONTEST_ASSERT_EQUAL(false, value.isUInt64());
#endif
}

JSONTEST_FIXTURE(ValueTest, compareNull) {
  JSONTEST_ASSERT_PRED(checkIsEqual(TestValue(), TestValue()));
}

JSONTEST_FIXTURE(ValueTest, compareInt) {
  JSONTEST_ASSERT_PRED(checkIsLess(0, 10));
  JSONTEST_ASSERT_PRED(checkIsEqual(10, 10));
  JSONTEST_ASSERT_PRED(checkIsEqual(-10, -10));
  JSONTEST_ASSERT_PRED(checkIsLess(-10, 0));
}

JSONTEST_FIXTURE(ValueTest, compareUInt) {
  JSONTEST_ASSERT_PRED(checkIsLess(0u, 10u));
  JSONTEST_ASSERT_PRED(checkIsLess(0u, TestValue::maxUInt));
  JSONTEST_ASSERT_PRED(checkIsEqual(10u, 10u));
}

JSONTEST_FIXTURE(ValueTest, compareDouble) {
  JSONTEST_ASSERT_PRED(checkIsLess(0.0, 10.0));
  JSONTEST_ASSERT_PRED(checkIsEqual(10.0, 10.0));
  JSONTEST_ASSERT_PRED(checkIsEqual(-10.0, -10.0));
  JSONTEST_ASSERT_PRED(checkIsLess(-10.0, 0.0));
}

JSONTEST_FIXTURE(ValueTest, compareString) {
  JSONTEST_ASSERT_PRED(checkIsLess("", " "));
  JSONTEST_ASSERT_PRED(checkIsLess("", "a"));
  JSONTEST_ASSERT_PRED(checkIsLess("abcd", "zyui"));
  JSONTEST_ASSERT_PRED(checkIsLess("abc", "abcd"));
  JSONTEST_ASSERT_PRED(checkIsEqual("abcd", "abcd"));
  JSONTEST_ASSERT_PRED(checkIsEqual(" ", " "));
  JSONTEST_ASSERT_PRED(checkIsLess("ABCD", "abcd"));
  JSONTEST_ASSERT_PRED(checkIsEqual("ABCD", "ABCD"));
}

JSONTEST_FIXTURE(ValueTest, compareBoolean) {
  JSONTEST_ASSERT_PRED(checkIsLess(false, true));
  JSONTEST_ASSERT_PRED(checkIsEqual(false, false));
  JSONTEST_ASSERT_PRED(checkIsEqual(true, true));
}

JSONTEST_FIXTURE(ValueTest, compareArray) {
  // array compare size then content
  TestValue emptyArray(Json::arrayValue);
  TestValue l1aArray;
  l1aArray.append(0);
  TestValue l1bArray;
  l1bArray.append(10);
  TestValue l2aArray;
  l2aArray.append(0);
  l2aArray.append(0);
  TestValue l2bArray;
  l2bArray.append(0);
  l2bArray.append(10);
  JSONTEST_ASSERT_PRED(checkIsLess(emptyArray, l1aArray));
  JSONTEST_ASSERT_PRED(checkIsLess(emptyArray, l2aArray));
  JSONTEST_ASSERT_PRED(checkIsLess(l1aArray, l2aArray));
  JSONTEST_ASSERT_PRED(checkIsLess(l2aArray, l2bArray));
  JSONTEST_ASSERT_PRED(checkIsEqual(emptyArray, TestValue(emptyArray)));
  JSONTEST_ASSERT_PRED(checkIsEqual(l1aArray, TestValue(l1aArray)));
  JSONTEST_ASSERT_PRED(checkIsEqual(l2bArray, TestValue(l2bArray)));
}

JSONTEST_FIXTURE(ValueTest, compareObject) {
  // object compare size then content
  TestValue emptyObject(Json::objectValue);
  TestValue l1aObject;
  l1aObject["key1"] = 0;
  TestValue l1bObject;
  l1aObject["key1"] = 10;
  TestValue l2aObject;
  l2aObject["key1"] = 0;
  l2aObject["key2"] = 0;
  JSONTEST_ASSERT_PRED(checkIsLess(emptyObject, l1aObject));
  JSONTEST_ASSERT_PRED(checkIsLess(emptyObject, l2aObject));
  JSONTEST_ASSERT_PRED(checkIsLess(l1aObject, l2aObject));
  JSONTEST_ASSERT_PRED(checkIsEqual(emptyObject, TestValue(emptyObject)));
  JSONTEST_ASSERT_PRED(checkIsEqual(l1aObject, TestValue(l1aObject)));
  JSONTEST_ASSERT_PRED(checkIsEqual(l2aObject, TestValue(l2aObject)));
}

JSONTEST_FIXTURE(ValueTest, compareType) {
  // object of different type are ordered according to their type
  JSONTEST_ASSERT_PRED(checkIsLess(TestValue(), TestValue(1)));
  JSONTEST_ASSERT_PRED(checkIsLess(TestValue(1), TestValue(1u)));
  JSONTEST_ASSERT_PRED(checkIsLess(TestValue(1u), TestValue(1.0)));
  JSONTEST_ASSERT_PRED(checkIsLess(TestValue(1.0), TestValue("a")));
  JSONTEST_ASSERT_PRED(checkIsLess(TestValue("a"), TestValue(true)));
  JSONTEST_ASSERT_PRED(
      checkIsLess(TestValue(true), TestValue(Json::arrayValue)));
  JSONTEST_ASSERT_PRED(checkIsLess(TestValue(Json::arrayValue),
                                   TestValue(Json::objectValue)));
}

void ValueTest::checkIsLess(const TestValue& x, const TestValue& y) {
  JSONTEST_ASSERT(x < y);
  JSONTEST_ASSERT(y > x);
  JSONTEST_ASSERT(x <= y);
  JSONTEST_ASSERT(y >= x);
  JSONTEST_ASSERT(!(x == y));
  JSONTEST_ASSERT(!(y == x));
  JSONTEST_ASSERT(!(x >= y));
  JSONTEST_ASSERT(!(y <= x));
  JSONTEST_ASSERT(!(x > y));
  JSONTEST_ASSERT(!(y < x));
  JSONTEST_ASSERT(x.compare(y) < 0);
  JSONTEST_ASSERT(y.compare(x) >= 0);
}

void ValueTest::checkIsEqual(const TestValue& x, const TestValue& y) {
  JSONTEST_ASSERT(x == y);
  JSONTEST_ASSERT(y == x);
  JSONTEST_ASSERT(x <= y);
  JSONTEST_ASSERT(y <= x);
  JSONTEST_ASSERT(x >= y);
  JSONTEST_ASSERT(y >= x);
  JSONTEST_ASSERT(!(x < y));
  JSONTEST_ASSERT(!(y < x));
  JSONTEST_ASSERT(!(x > y));
  JSONTEST_ASSERT(!(y > x));
  JSONTEST_ASSERT(x.compare(y) == 0);
  JSONTEST_ASSERT(y.compare(x) == 0);
}

JSONTEST_FIXTURE(ValueTest, typeChecksThrowExceptions) {
#if JSON_USE_EXCEPTION

  TestValue intVal(1);
  TestValue strVal("Test");
  TestValue objVal(Json::objectValue);
  TestValue arrVal(Json::arrayValue);

  JSONTEST_ASSERT_THROWS(intVal["test"]);
  JSONTEST_ASSERT_THROWS(strVal["test"]);
  JSONTEST_ASSERT_THROWS(arrVal["test"]);

  JSONTEST_ASSERT_THROWS(intVal.removeMember("test"));
  JSONTEST_ASSERT_THROWS(strVal.removeMember("test"));
  JSONTEST_ASSERT_THROWS(arrVal.removeMember("test"));

  JSONTEST_ASSERT_THROWS(intVal.getMemberNames());
  JSONTEST_ASSERT_THROWS(strVal.getMemberNames());
  JSONTEST_ASSERT_THROWS(arrVal.getMemberNames());

  JSONTEST_ASSERT_THROWS(intVal[0]);
  JSONTEST_ASSERT_THROWS(objVal[0]);
  JSONTEST_ASSERT_THROWS(strVal[0]);

  JSONTEST_ASSERT_THROWS(intVal.clear());

  JSONTEST_ASSERT_THROWS(intVal.resize(1));
  JSONTEST_ASSERT_THROWS(strVal.resize(1));
  JSONTEST_ASSERT_THROWS(objVal.resize(1));

  JSONTEST_ASSERT_THROWS(intVal.asCString());

  JSONTEST_ASSERT_THROWS(objVal.asString());
  JSONTEST_ASSERT_THROWS(arrVal.asString());

  JSONTEST_ASSERT_THROWS(strVal.asInt());
  JSONTEST_ASSERT_THROWS(objVal.asInt());
  JSONTEST_ASSERT_THROWS(arrVal.asInt());

  JSONTEST_ASSERT_THROWS(strVal.asUInt());
  JSONTEST_ASSERT_THROWS(objVal.asUInt());
  JSONTEST_ASSERT_THROWS(arrVal.asUInt());

  JSONTEST_ASSERT_THROWS(strVal.asInt64());
  JSONTEST_ASSERT_THROWS(objVal.asInt64());
  JSONTEST_ASSERT_THROWS(arrVal.asInt64());

  JSONTEST_ASSERT_THROWS(strVal.asUInt64());
  JSONTEST_ASSERT_THROWS(objVal.asUInt64());
  JSONTEST_ASSERT_THROWS(arrVal.asUInt64());

  JSONTEST_ASSERT_THROWS(strVal.asDouble());
  JSONTEST_ASSERT_THROWS(objVal.asDouble());
  JSONTEST_ASSERT_THROWS(arrVal.asDouble());

  JSONTEST_ASSERT_THROWS(strVal.asFloat());
  JSONTEST_ASSERT_THROWS(objVal.asFloat());
  JSONTEST_ASSERT_THROWS(arrVal.asFloat());

  JSONTEST_ASSERT_THROWS(strVal.asBool());
  JSONTEST_ASSERT_THROWS(objVal.asBool());
  JSONTEST_ASSERT_THROWS(arrVal.asBool());

#endif
}

JSONTEST_FIXTURE(ValueTest, offsetAccessors) {
  TestValue x;
  JSONTEST_ASSERT(x.getOffsetStart() == 0);
  JSONTEST_ASSERT(x.getOffsetLimit() == 0);
  x.setOffsetStart(10);
  x.setOffsetLimit(20);
  JSONTEST_ASSERT(x.getOffsetStart() == 10);
  JSONTEST_ASSERT(x.getOffsetLimit() == 20);
  TestValue y(x);
  JSONTEST_ASSERT(y.getOffsetStart() == 10);
  JSONTEST_ASSERT(y.getOffsetLimit() == 20);
  TestValue z;
  z.swap(y);
  JSONTEST_ASSERT(z.getOffsetStart() == 10);
  JSONTEST_ASSERT(z.getOffsetLimit() == 20);
  JSONTEST_ASSERT(y.getOffsetStart() == 0);
  JSONTEST_ASSERT(y.getOffsetLimit() == 0);
}

JSONTEST_FIXTURE(ValueTest, StaticString) {
  char mutant[] = "hello";
  Json::StaticString ss(mutant);
  std::string regular(mutant);
  mutant[1] = 'a';
  JSONTEST_ASSERT_STRING_EQUAL("hallo", ss.c_str());
  JSONTEST_ASSERT_STRING_EQUAL("hello", regular.c_str());
  {
    TestValue root;
    root["top"] = ss;
    JSONTEST_ASSERT_STRING_EQUAL("hallo", root["top"].asString());
    mutant[1] = 'u';
    JSONTEST_ASSERT_STRING_EQUAL("hullo", root["top"].asString());
  }
  {
    TestValue root;
    root["top"] = regular;
    JSONTEST_ASSERT_STRING_EQUAL("hello", root["top"].asString());
    mutant[1] = 'u';
    JSONTEST_ASSERT_STRING_EQUAL("hello", root["top"].asString());
  }
}

#include <iostream>

JSONTEST_FIXTURE(ValueTest, CommentBefore) {
  TestValue val; // fill val
  val.setComment(std::string("// this comment should appear before"), Json::commentBefore);
  TestStreamWriterBuilder wbuilder;
  wbuilder.settings_["commentStyle"] = "All";
  {
    char const expected[] = "// this comment should appear before\nnull";
    std::string result = Json::writeString(wbuilder, val);
    JSONTEST_ASSERT_STRING_EQUAL(expected, result);
    std::string res2 = val.toStyledString();
    std::string exp2 = "\n";
    exp2 += expected;
    exp2 += "\n";
    JSONTEST_ASSERT_STRING_EQUAL(exp2, res2);
  }
  TestValue other = "hello";
  val.swapPayload(other);
  {
    char const expected[] = "// this comment should appear before\n\"hello\"";
    std::string result = Json::writeString(wbuilder, val);
    JSONTEST_ASSERT_STRING_EQUAL(expected, result);
    std::string res2 = val.toStyledString();
    std::string exp2 = "\n";
    exp2 += expected;
    exp2 += "\n";
    JSONTEST_ASSERT_STRING_EQUAL(exp2, res2);
    JSONTEST_ASSERT_STRING_EQUAL("null\n", other.toStyledString());
  }
  val = "hello";
  // val.setComment("// this comment should appear before", Json::CommentPlacement::commentBefore);
  // Assignment over-writes comments.
  {
    char const expected[] = "\"hello\"";
    std::string result = Json::writeString(wbuilder, val);
    JSONTEST_ASSERT_STRING_EQUAL(expected, result);
    std::string res2 = val.toStyledString();
    std::string exp2 = "";
    exp2 += expected;
    exp2 += "\n";
    JSONTEST_ASSERT_STRING_EQUAL(exp2, res2);
  }
}

JSONTEST_FIXTURE(ValueTest, zeroes) {
  char const cstr[] = "h\0i";
  std::string binary(cstr, sizeof(cstr));  // include trailing 0
  JSONTEST_ASSERT_EQUAL(4U, binary.length());
  TestStreamWriterBuilder b;
  {
    TestValue root;
    root = binary;
    JSONTEST_ASSERT_STRING_EQUAL(binary, root.asString());
  }
  {
    char const top[] = "top";
    TestValue root;
    root[top] = binary;
    JSONTEST_ASSERT_STRING_EQUAL(binary, root[top].asString());
    TestValue removed;
    bool did;
    did = root.removeMember(top, top + sizeof(top) - 1U,
        &removed);
    JSONTEST_ASSERT(did);
    JSONTEST_ASSERT_STRING_EQUAL(binary, removed.asString());
    did = root.removeMember(top, top + sizeof(top) - 1U,
        &removed);
    JSONTEST_ASSERT(!did);
    JSONTEST_ASSERT_STRING_EQUAL(binary, removed.asString()); // still
  }
}

JSONTEST_FIXTURE(ValueTest, zeroesInKeys) {
  char const cstr[] = "h\0i";
  std::string binary(cstr, sizeof(cstr));  // include trailing 0
  JSONTEST_ASSERT_EQUAL(4U, binary.length());
  {
    TestValue root;
    root[binary] = "there";
    JSONTEST_ASSERT_STRING_EQUAL("there", root[binary].asString());
    JSONTEST_ASSERT(!root.isMember("h"));
    JSONTEST_ASSERT(root.isMember(binary));
    JSONTEST_ASSERT_STRING_EQUAL("there", root.get(binary, TestValue::nullRef).asString());
    TestValue removed;
    bool did;
    did = root.removeMember(binary.data(), binary.data() + binary.length(),
        &removed);
    JSONTEST_ASSERT(did);
    JSONTEST_ASSERT_STRING_EQUAL("there", removed.asString());
    did = root.removeMember(binary.data(), binary.data() + binary.length(),
        &removed);
    JSONTEST_ASSERT(!did);
    JSONTEST_ASSERT_STRING_EQUAL("there", removed.asString()); // still
    JSONTEST_ASSERT(!root.isMember(binary));
    JSONTEST_ASSERT_STRING_EQUAL("", root.get(binary, TestValue::nullRef).asString());
  }
}

JSONTEST_FIXTURE(ValueTest, specialFloats) {
  TestStreamWriterBuilder b;
  b.settings_["useSpecialFloats"] = true;

  TestValue v = std::numeric_limits<double>::quiet_NaN();
  std::string expected = "NaN";
  std::string result = Json::writeString(b, v);
  JSONTEST_ASSERT_STRING_EQUAL(expected, result);

  v = std::numeric_limits<double>::infinity();
  expected = "Infinity";
  result = Json::writeString(b, v);
  JSONTEST_ASSERT_STRING_EQUAL(expected, result);

  v = -std::numeric_limits<double>::infinity();
  expected = "-Infinity";
  result = Json::writeString(b, v);
  JSONTEST_ASSERT_STRING_EQUAL(expected, result);
}

JSONTEST_FIXTURE(ValueTest, precision) {
    TestStreamWriterBuilder b;
    b.settings_["precision"] = 5;

    TestValue v = 100.0/3;
    std::string expected = "33.333";
    std::string result = Json::writeString(b, v);
    JSONTEST_ASSERT_STRING_EQUAL(expected, result);
    
    v = 0.25000000;
    expected = "0.25";
    result = Json::writeString(b, v);
    JSONTEST_ASSERT_STRING_EQUAL(expected, result);

    v = 0.2563456;
    expected = "0.25635";
    result = Json::writeString(b, v);
    JSONTEST_ASSERT_STRING_EQUAL(expected, result);

    b.settings_["precision"] = 1;
    expected = "0.3";
    result = Json::writeString(b, v);
    JSONTEST_ASSERT_STRING_EQUAL(expected, result);

    b.settings_["precision"] = 17;
    v = 1234857476305.256345694873740545068;
    expected = "1234857476305.2563";
    result = Json::writeString(b, v);
    JSONTEST_ASSERT_STRING_EQUAL(expected, result);

    b.settings_["precision"] = 24;
    v = 0.256345694873740545068;
    expected = "0.25634569487374054";
    result = Json::writeString(b, v);
    JSONTEST_ASSERT_STRING_EQUAL(expected, result);
}

struct WriterTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(WriterTest, dropNullPlaceholders) {
  TestFastWriter writer;
  TestValue nullValue;
  JSONTEST_ASSERT(writer.write(nullValue) == "null\n");

  writer.dropNullPlaceholders();
  JSONTEST_ASSERT(writer.write(nullValue) == "\n");
}

struct StreamWriterTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(StreamWriterTest, dropNullPlaceholders) {
  TestStreamWriterBuilder b;
  TestValue nullValue;
  b.settings_["dropNullPlaceholders"] = false;
  JSONTEST_ASSERT(Json::writeString(b, nullValue) == "null");
  b.settings_["dropNullPlaceholders"] = true;
  JSONTEST_ASSERT(Json::writeString(b, nullValue) == "");
}

JSONTEST_FIXTURE(StreamWriterTest, writeZeroes) {
  std::string binary("hi", 3);  // include trailing 0
  JSONTEST_ASSERT_EQUAL(3, binary.length());
  std::string expected("\"hi\\u0000\"");  // unicoded zero
  TestStreamWriterBuilder b;
  {
    TestValue root;
    root = binary;
    JSONTEST_ASSERT_STRING_EQUAL(binary, root.asString());
    std::string out = Json::writeString(b, root);
    JSONTEST_ASSERT_EQUAL(expected.size(), out.size());
    JSONTEST_ASSERT_STRING_EQUAL(expected, out);
  }
  {
    TestValue root;
    root["top"] = binary;
    JSONTEST_ASSERT_STRING_EQUAL(binary, root["top"].asString());
    std::string out = Json::writeString(b, root["top"]);
    JSONTEST_ASSERT_STRING_EQUAL(expected, out);
  }
}

struct ReaderTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(ReaderTest, parseWithNoErrors) {
  TestReader reader;
  TestValue root;
  bool ok = reader.parse("{ \"property\" : \"value\" }", root);
  JSONTEST_ASSERT(ok);
  JSONTEST_ASSERT(reader.getFormattedErrorMessages().size() == 0);
  JSONTEST_ASSERT(reader.getStructuredErrors().size() == 0);
}

JSONTEST_FIXTURE(ReaderTest, parseWithNoErrorsTestingOffsets) {
  TestReader reader;
  TestValue root;
  bool ok = reader.parse("{ \"property\" : [\"value\", \"value2\"], \"obj\" : "
                         "{ \"nested\" : 123, \"bool\" : true}, \"null\" : "
                         "null, \"false\" : false }",
                         root);
  JSONTEST_ASSERT(ok);
  JSONTEST_ASSERT(reader.getFormattedErrorMessages().size() == 0);
  JSONTEST_ASSERT(reader.getStructuredErrors().size() == 0);
  JSONTEST_ASSERT(root["property"].getOffsetStart() == 15);
  JSONTEST_ASSERT(root["property"].getOffsetLimit() == 34);
  JSONTEST_ASSERT(root["property"][0].getOffsetStart() == 16);
  JSONTEST_ASSERT(root["property"][0].getOffsetLimit() == 23);
  JSONTEST_ASSERT(root["property"][1].getOffsetStart() == 25);
  JSONTEST_ASSERT(root["property"][1].getOffsetLimit() == 33);
  JSONTEST_ASSERT(root["obj"].getOffsetStart() == 44);
  JSONTEST_ASSERT(root["obj"].getOffsetLimit() == 76);
  JSONTEST_ASSERT(root["obj"]["nested"].getOffsetStart() == 57);
  JSONTEST_ASSERT(root["obj"]["nested"].getOffsetLimit() == 60);
  JSONTEST_ASSERT(root["obj"]["bool"].getOffsetStart() == 71);
  JSONTEST_ASSERT(root["obj"]["bool"].getOffsetLimit() == 75);
  JSONTEST_ASSERT(root["null"].getOffsetStart() == 87);
  JSONTEST_ASSERT(root["null"].getOffsetLimit() == 91);
  JSONTEST_ASSERT(root["false"].getOffsetStart() == 103);
  JSONTEST_ASSERT(root["false"].getOffsetLimit() == 108);
  JSONTEST_ASSERT(root.getOffsetStart() == 0);
  JSONTEST_ASSERT(root.getOffsetLimit() == 110);
}

JSONTEST_FIXTURE(ReaderTest, parseWithOneError) {
  TestReader reader;
  TestValue root;
  bool ok = reader.parse("{ \"property\" :: \"value\" }", root);
  JSONTEST_ASSERT(!ok);
  JSONTEST_ASSERT(reader.getFormattedErrorMessages() ==
                  "* Line 1, Column 15\n  Syntax error: value, object or array "
                  "expected.\n");
  std::vector<TestReader::StructuredError> errors =
      reader.getStructuredErrors();
  JSONTEST_ASSERT(errors.size() == 1);
  JSONTEST_ASSERT(errors.at(0).offset_start == 14);
  JSONTEST_ASSERT(errors.at(0).offset_limit == 15);
  JSONTEST_ASSERT(errors.at(0).message ==
                  "Syntax error: value, object or array expected.");
}

JSONTEST_FIXTURE(ReaderTest, parseChineseWithOneError) {
  TestReader reader;
  TestValue root;
  bool ok = reader.parse("{ \"pr佐藤erty\" :: \"value\" }", root);
  JSONTEST_ASSERT(!ok);
  JSONTEST_ASSERT(reader.getFormattedErrorMessages() ==
                  "* Line 1, Column 19\n  Syntax error: value, object or array "
                  "expected.\n");
  std::vector<TestReader::StructuredError> errors =
      reader.getStructuredErrors();
  JSONTEST_ASSERT(errors.size() == 1);
  JSONTEST_ASSERT(errors.at(0).offset_start == 18);
  JSONTEST_ASSERT(errors.at(0).offset_limit == 19);
  JSONTEST_ASSERT(errors.at(0).message ==
                  "Syntax error: value, object or array expected.");
}

JSONTEST_FIXTURE(ReaderTest, parseWithDetailError) {
  TestReader reader;
  TestValue root;
  bool ok = reader.parse("{ \"property\" : \"v\\alue\" }", root);
  JSONTEST_ASSERT(!ok);
  JSONTEST_ASSERT(reader.getFormattedErrorMessages() ==
                  "* Line 1, Column 16\n  Bad escape sequence in string\nSee "
                  "Line 1, Column 20 for detail.\n");
  std::vector<TestReader::StructuredError> errors =
      reader.getStructuredErrors();
  JSONTEST_ASSERT(errors.size() == 1);
  JSONTEST_ASSERT(errors.at(0).offset_start == 15);
  JSONTEST_ASSERT(errors.at(0).offset_limit == 23);
  JSONTEST_ASSERT(errors.at(0).message == "Bad escape sequence in string");
}

struct CharReaderTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(CharReaderTest, parseWithNoErrors) {
  TestCharReaderBuilder b;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  TestValue root;
  char const doc[] = "{ \"property\" : \"value\" }";
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(ok);
  JSONTEST_ASSERT(errs.size() == 0);
  delete reader;
}

JSONTEST_FIXTURE(CharReaderTest, parseWithNoErrorsTestingOffsets) {
  TestCharReaderBuilder b;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  TestValue root;
  char const doc[] =
                         "{ \"property\" : [\"value\", \"value2\"], \"obj\" : "
                         "{ \"nested\" : 123, \"bool\" : true}, \"null\" : "
                         "null, \"false\" : false }";
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(ok);
  JSONTEST_ASSERT(errs.size() == 0);
  delete reader;
}

JSONTEST_FIXTURE(CharReaderTest, parseWithOneError) {
  TestCharReaderBuilder b;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  TestValue root;
  char const doc[] =
      "{ \"property\" :: \"value\" }";
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(!ok);
  JSONTEST_ASSERT(errs ==
                  "* Line 1, Column 15\n  Syntax error: value, object or array "
                  "expected.\n");
  delete reader;
}

JSONTEST_FIXTURE(CharReaderTest, parseChineseWithOneError) {
  TestCharReaderBuilder b;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  TestValue root;
  char const doc[] =
      "{ \"pr佐藤erty\" :: \"value\" }";
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(!ok);
  JSONTEST_ASSERT(errs ==
                  "* Line 1, Column 19\n  Syntax error: value, object or array "
                  "expected.\n");
  delete reader;
}

JSONTEST_FIXTURE(CharReaderTest, parseWithDetailError) {
  TestCharReaderBuilder b;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  TestValue root;
  char const doc[] =
      "{ \"property\" : \"v\\alue\" }";
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(!ok);
  JSONTEST_ASSERT(errs ==
                  "* Line 1, Column 16\n  Bad escape sequence in string\nSee "
                  "Line 1, Column 20 for detail.\n");
  delete reader;
}

JSONTEST_FIXTURE(CharReaderTest, parseWithStackLimit) {
  TestCharReaderBuilder b;
  TestValue root;
  char const doc[] =
      "{ \"property\" : \"value\" }";
  {
  b.settings_["stackLimit"] = 2;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(ok);
  JSONTEST_ASSERT(errs == "");
  JSONTEST_ASSERT_EQUAL("value", root["property"]);
  delete reader;
  }
  {
  b.settings_["stackLimit"] = 1;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  JSONTEST_ASSERT_THROWS(reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs));
  delete reader;
  }
}

struct CharReaderStrictModeTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(CharReaderStrictModeTest, dupKeys) {
  TestCharReaderBuilder b;
  TestValue root;
  char const doc[] =
      "{ \"property\" : \"value\", \"key\" : \"val1\", \"key\" : \"val2\" }";
  {
    b.strictMode(&b.settings_);
    TestCharReader* reader(b.newCharReader());
    std::string errs;
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(!ok);
    JSONTEST_ASSERT_STRING_EQUAL(
        "* Line 1, Column 41\n"
        "  Duplicate key: 'key'\n",
        errs);
    JSONTEST_ASSERT_EQUAL("val1", root["key"]); // so far
    delete reader;
  }
}
struct CharReaderFailIfExtraTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(CharReaderFailIfExtraTest, issue164) {
  // This is interpreted as a string value followed by a colon.
  TestCharReaderBuilder b;
  TestValue root;
  char const doc[] =
      " \"property\" : \"value\" }";
  {
  b.settings_["failIfExtra"] = false;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(ok);
  JSONTEST_ASSERT(errs == "");
  JSONTEST_ASSERT_EQUAL("property", root);
  delete reader;
  }
  {
  b.settings_["failIfExtra"] = true;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(!ok);
  JSONTEST_ASSERT_STRING_EQUAL(errs,
      "* Line 1, Column 13\n"
      "  Extra non-whitespace after JSON value.\n");
  JSONTEST_ASSERT_EQUAL("property", root);
  delete reader;
  }
  {
  b.settings_["failIfExtra"] = false;
  b.strictMode(&b.settings_);
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(!ok);
  JSONTEST_ASSERT_STRING_EQUAL(errs,
      "* Line 1, Column 13\n"
      "  Extra non-whitespace after JSON value.\n");
  JSONTEST_ASSERT_EQUAL("property", root);
  delete reader;
  }
}
JSONTEST_FIXTURE(CharReaderFailIfExtraTest, issue107) {
  // This is interpretted as an int value followed by a colon.
  TestCharReaderBuilder b;
  TestValue root;
  char const doc[] =
      "1:2:3";
  b.settings_["failIfExtra"] = true;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(!ok);
  JSONTEST_ASSERT_STRING_EQUAL(
      "* Line 1, Column 2\n"
      "  Extra non-whitespace after JSON value.\n",
      errs);
  JSONTEST_ASSERT_EQUAL(1, root.asInt());
  delete reader;
}
JSONTEST_FIXTURE(CharReaderFailIfExtraTest, commentAfterObject) {
  TestCharReaderBuilder b;
  TestValue root;
  {
  char const doc[] =
      "{ \"property\" : \"value\" } //trailing\n//comment\n";
  b.settings_["failIfExtra"] = true;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(ok);
  JSONTEST_ASSERT_STRING_EQUAL("", errs);
  JSONTEST_ASSERT_EQUAL("value", root["property"]);
  delete reader;
  }
}
JSONTEST_FIXTURE(CharReaderFailIfExtraTest, commentAfterArray) {
  TestCharReaderBuilder b;
  TestValue root;
  char const doc[] =
      "[ \"property\" , \"value\" ] //trailing\n//comment\n";
  b.settings_["failIfExtra"] = true;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(ok);
  JSONTEST_ASSERT_STRING_EQUAL("", errs);
  JSONTEST_ASSERT_EQUAL("value", root[1u]);
  delete reader;
}
JSONTEST_FIXTURE(CharReaderFailIfExtraTest, commentAfterBool) {
  TestCharReaderBuilder b;
  TestValue root;
  char const doc[] =
      " true /*trailing\ncomment*/";
  b.settings_["failIfExtra"] = true;
  TestCharReader* reader(b.newCharReader());
  std::string errs;
  bool ok = reader->parse(
      doc, doc + std::strlen(doc),
      &root, &errs);
  JSONTEST_ASSERT(ok);
  JSONTEST_ASSERT_STRING_EQUAL("", errs);
  JSONTEST_ASSERT_EQUAL(true, root.asBool());
  delete reader;
}
struct CharReaderAllowDropNullTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(CharReaderAllowDropNullTest, issue178) {
  TestCharReaderBuilder b;
  b.settings_["allowDroppedNullPlaceholders"] = true;
  TestValue root;
  std::string errs;
  TestCharReader* reader(b.newCharReader());
  {
    char const doc[] = "{\"a\":,\"b\":true}";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(2u, root.size());
    JSONTEST_ASSERT_EQUAL(Json::nullValue, root.get("a", true));
  }
  {
    char const doc[] = "{\"a\":}";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(1u, root.size());
    JSONTEST_ASSERT_EQUAL(Json::nullValue, root.get("a", true));
  }
  {
    char const doc[] = "[]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT(errs == "");
    JSONTEST_ASSERT_EQUAL(0u, root.size());
    JSONTEST_ASSERT_EQUAL(Json::arrayValue, root);
  }
  {
    char const doc[] = "[null]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT(errs == "");
    JSONTEST_ASSERT_EQUAL(1u, root.size());
  }
  {
    char const doc[] = "[,]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(2u, root.size());
  }
  {
    char const doc[] = "[,,,]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(4u, root.size());
  }
  {
    char const doc[] = "[null,]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(2u, root.size());
  }
  {
    char const doc[] = "[,null]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT(errs == "");
    JSONTEST_ASSERT_EQUAL(2u, root.size());
  }
  {
    char const doc[] = "[,,]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(3u, root.size());
  }
  {
    char const doc[] = "[null,,]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(3u, root.size());
  }
  {
    char const doc[] = "[,null,]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(3u, root.size());
  }
  {
    char const doc[] = "[,,null]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT(errs == "");
    JSONTEST_ASSERT_EQUAL(3u, root.size());
  }
  {
    char const doc[] = "[[],,,]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(4u, root.size());
    JSONTEST_ASSERT_EQUAL(Json::arrayValue, root[0u]);
  }
  {
    char const doc[] = "[,[],,]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(4u, root.size());
    JSONTEST_ASSERT_EQUAL(Json::arrayValue, root[1u]);
  }
  {
    char const doc[] = "[,,,[]]";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT(errs == "");
    JSONTEST_ASSERT_EQUAL(4u, root.size());
    JSONTEST_ASSERT_EQUAL(Json::arrayValue, root[3u]);
  }
  delete reader;
}

struct CharReaderAllowSingleQuotesTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(CharReaderAllowSingleQuotesTest, issue182) {
  TestCharReaderBuilder b;
  b.settings_["allowSingleQuotes"] = true;
  TestValue root;
  std::string errs;
  TestCharReader* reader(b.newCharReader());
  {
    char const doc[] = "{'a':true,\"b\":true}";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(2u, root.size());
    JSONTEST_ASSERT_EQUAL(true, root.get("a", false));
    JSONTEST_ASSERT_EQUAL(true, root.get("b", false));
  }
  {
    char const doc[] = "{'a': 'x', \"b\":'y'}";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(2u, root.size());
    JSONTEST_ASSERT_STRING_EQUAL("x", root["a"].asString());
    JSONTEST_ASSERT_STRING_EQUAL("y", root["b"].asString());
  }
  delete reader;
}

struct CharReaderAllowZeroesTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(CharReaderAllowZeroesTest, issue176) {
  TestCharReaderBuilder b;
  b.settings_["allowSingleQuotes"] = true;
  TestValue root;
  std::string errs;
  TestCharReader* reader(b.newCharReader());
  {
    char const doc[] = "{'a':true,\"b\":true}";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(2u, root.size());
    JSONTEST_ASSERT_EQUAL(true, root.get("a", false));
    JSONTEST_ASSERT_EQUAL(true, root.get("b", false));
  }
  {
    char const doc[] = "{'a': 'x', \"b\":'y'}";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(2u, root.size());
    JSONTEST_ASSERT_STRING_EQUAL("x", root["a"].asString());
    JSONTEST_ASSERT_STRING_EQUAL("y", root["b"].asString());
  }
  delete reader;
}

struct CharReaderAllowSpecialFloatsTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(CharReaderAllowSpecialFloatsTest, issue209) {
  TestCharReaderBuilder b;
  b.settings_["allowSpecialFloats"] = true;
  TestValue root;
  std::string errs;
  TestCharReader* reader(b.newCharReader());
  {
    char const doc[] = "{\"a\":NaN,\"b\":Infinity,\"c\":-Infinity}";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(3u, root.size());
    double n = root["a"].asDouble();
    JSONTEST_ASSERT(n != n);
    JSONTEST_ASSERT_EQUAL(std::numeric_limits<double>::infinity(), root.get("b", 0.0));
    JSONTEST_ASSERT_EQUAL(-std::numeric_limits<double>::infinity(), root.get("c", 0.0));
  }

  struct TestData {
    int line;
    bool ok;
    std::string in;
  };
  const TestData test_data[] = {
    {__LINE__, 1, "{\"a\":9}"},
    {__LINE__, 0, "{\"a\":0Infinity}"},
    {__LINE__, 0, "{\"a\":1Infinity}"},
    {__LINE__, 0, "{\"a\":9Infinity}"},
    {__LINE__, 0, "{\"a\":0nfinity}"},
    {__LINE__, 0, "{\"a\":1nfinity}"},
    {__LINE__, 0, "{\"a\":9nfinity}"},
    {__LINE__, 0, "{\"a\":nfinity}"},
    {__LINE__, 0, "{\"a\":.nfinity}"},
    {__LINE__, 0, "{\"a\":9nfinity}"},
    {__LINE__, 0, "{\"a\":-nfinity}"},
    {__LINE__, 1, "{\"a\":Infinity}"},
    {__LINE__, 0, "{\"a\":.Infinity}"},
    {__LINE__, 0, "{\"a\":_Infinity}"},
    {__LINE__, 0, "{\"a\":_nfinity}"},
    {__LINE__, 1, "{\"a\":-Infinity}"}
  };
  for (size_t tdi = 0; tdi < sizeof(test_data) / sizeof(*test_data); ++tdi) {
    const TestData& td = test_data[tdi];
    bool ok = reader->parse(&*td.in.begin(),
                            &*td.in.begin() + td.in.size(),
                            &root, &errs);
    JSONTEST_ASSERT(td.ok == ok)
        << "line:" << td.line << "\n"
        << "  expected: {"
        << "ok:" << td.ok
        << ", in:\'" << td.in << "\'"
        << "}\n"
        << "  actual: {"
        << "ok:" << ok
        << "}\n";
  }

  {
    char const doc[] = "{\"posInf\": Infinity, \"NegInf\": -Infinity}";
    bool ok = reader->parse(
        doc, doc + std::strlen(doc),
        &root, &errs);
    JSONTEST_ASSERT(ok);
    JSONTEST_ASSERT_STRING_EQUAL("", errs);
    JSONTEST_ASSERT_EQUAL(2u, root.size());
    JSONTEST_ASSERT_EQUAL(std::numeric_limits<double>::infinity(), root["posInf"].asDouble());
    JSONTEST_ASSERT_EQUAL(-std::numeric_limits<double>::infinity(), root["NegInf"].asDouble());
  }
  delete reader;
}

struct BuilderTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(BuilderTest, settings) {
  {
    TestValue errs;
    TestCharReaderBuilder rb;
    JSONTEST_ASSERT_EQUAL(false, rb.settings_.isMember("foo"));
    JSONTEST_ASSERT_EQUAL(true, rb.validate(&errs));
    rb["foo"] = "bar";
    JSONTEST_ASSERT_EQUAL(true, rb.settings_.isMember("foo"));
    JSONTEST_ASSERT_EQUAL(false, rb.validate(&errs));
  }
  {
    TestValue errs;
    TestStreamWriterBuilder wb;
    JSONTEST_ASSERT_EQUAL(false, wb.settings_.isMember("foo"));
    JSONTEST_ASSERT_EQUAL(true, wb.validate(&errs));
    wb["foo"] = "bar";
    JSONTEST_ASSERT_EQUAL(true, wb.settings_.isMember("foo"));
    JSONTEST_ASSERT_EQUAL(false, wb.validate(&errs));
  }
}

struct IteratorTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(IteratorTest, distance) {
  TestValue json;
  json["k1"] = "a";
  json["k2"] = "b";
  int dist = 0;
  std::string str;
  for (TestValueIterator it = json.begin(); it != json.end(); ++it) {
    dist = it - json.begin();
    str = it->asString().c_str();
  }
  JSONTEST_ASSERT_EQUAL(1, dist);
  JSONTEST_ASSERT_STRING_EQUAL("b", str);
}

JSONTEST_FIXTURE(IteratorTest, names) {
  TestValue json;
  json["k1"] = "a";
  json["k2"] = "b";
  TestValueIterator it = json.begin();
  JSONTEST_ASSERT(it != json.end());
  JSONTEST_ASSERT_EQUAL(TestValue("k1"), it.key());
  JSONTEST_ASSERT_STRING_EQUAL("k1", it.name());
  JSONTEST_ASSERT_EQUAL(-1, it.index());
  ++it;
  JSONTEST_ASSERT(it != json.end());
  JSONTEST_ASSERT_EQUAL(TestValue("k2"), it.key());
  JSONTEST_ASSERT_STRING_EQUAL("k2", it.name());
  JSONTEST_ASSERT_EQUAL(-1, it.index());
  ++it;
  JSONTEST_ASSERT(it == json.end());
}

JSONTEST_FIXTURE(IteratorTest, indexes) {
  TestValue json;
  json[0] = "a";
  json[1] = "b";
  TestValueIterator it = json.begin();
  JSONTEST_ASSERT(it != json.end());
  JSONTEST_ASSERT_EQUAL(TestValue(Json::ArrayIndex(0)), it.key());
  JSONTEST_ASSERT_STRING_EQUAL("", it.name());
  JSONTEST_ASSERT_EQUAL(0, it.index());
  ++it;
  JSONTEST_ASSERT(it != json.end());
  JSONTEST_ASSERT_EQUAL(TestValue(Json::ArrayIndex(1)), it.key());
  JSONTEST_ASSERT_STRING_EQUAL("", it.name());
  JSONTEST_ASSERT_EQUAL(1, it.index());
  ++it;
  JSONTEST_ASSERT(it == json.end());
}

JSONTEST_FIXTURE(IteratorTest, const) {
  TestValue const v;
  JSONTEST_ASSERT_THROWS(
    TestValue::iterator it(v.begin()) // Compile, but throw.
  );

  TestValue value;

  for(int i = 9; i < 12; ++i)
  {
    std::ostringstream out;
    out << std::setw(2) << i;
    std::string str = out.str();
    value[str] = str;
  }

  std::ostringstream out;
  //in old code, this will get a compile error
  TestValue::const_iterator iter = value.begin();
  for(; iter != value.end(); ++iter)
  {
    out << *iter << ',';
  }
  std::string expected = "\" 9\",\"10\",\"11\",";
  JSONTEST_ASSERT_STRING_EQUAL(expected, out.str());
}

struct RValueTest : JsonTest::TestCase {};

JSONTEST_FIXTURE(RValueTest, moveConstruction) {
#if JSON_HAS_RVALUE_REFERENCES
  TestValue json;
  json["key"] = "value";
  TestValue moved = std::move(json);
  JSONTEST_ASSERT(moved != json); // Possibly not nullValue; definitely not equal.
  JSONTEST_ASSERT_EQUAL(Json::objectValue, moved.type());
  JSONTEST_ASSERT_EQUAL(Json::stringValue, moved["key"].type());
#endif
}

int main(int argc, const char* argv[]) {
  JsonTest::Runner runner;
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, checkNormalizeFloatingPointStr);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, memberCount);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, objects);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, arrays);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, arrayIssue252);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, null);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, strings);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, bools);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, integers);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, nonIntegers);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, compareNull);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, compareInt);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, compareUInt);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, compareDouble);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, compareString);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, compareBoolean);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, compareArray);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, compareObject);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, compareType);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, offsetAccessors);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, typeChecksThrowExceptions);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, StaticString);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, CommentBefore);
  //JSONTEST_REGISTER_FIXTURE(runner, ValueTest, nulls);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, zeroes);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, zeroesInKeys);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, specialFloats);
  JSONTEST_REGISTER_FIXTURE(runner, ValueTest, precision);

  JSONTEST_REGISTER_FIXTURE(runner, WriterTest, dropNullPlaceholders);
  JSONTEST_REGISTER_FIXTURE(runner, StreamWriterTest, dropNullPlaceholders);
  JSONTEST_REGISTER_FIXTURE(runner, StreamWriterTest, writeZeroes);

  JSONTEST_REGISTER_FIXTURE(runner, ReaderTest, parseWithNoErrors);
  JSONTEST_REGISTER_FIXTURE(
      runner, ReaderTest, parseWithNoErrorsTestingOffsets);
  JSONTEST_REGISTER_FIXTURE(runner, ReaderTest, parseWithOneError);
  JSONTEST_REGISTER_FIXTURE(runner, ReaderTest, parseChineseWithOneError);
  JSONTEST_REGISTER_FIXTURE(runner, ReaderTest, parseWithDetailError);

  JSONTEST_REGISTER_FIXTURE(runner, CharReaderTest, parseWithNoErrors);
  JSONTEST_REGISTER_FIXTURE(
      runner, CharReaderTest, parseWithNoErrorsTestingOffsets);
  JSONTEST_REGISTER_FIXTURE(runner, CharReaderTest, parseWithOneError);
  JSONTEST_REGISTER_FIXTURE(runner, CharReaderTest, parseChineseWithOneError);
  JSONTEST_REGISTER_FIXTURE(runner, CharReaderTest, parseWithDetailError);
  JSONTEST_REGISTER_FIXTURE(runner, CharReaderTest, parseWithStackLimit);

  JSONTEST_REGISTER_FIXTURE(runner, CharReaderStrictModeTest, dupKeys);

  JSONTEST_REGISTER_FIXTURE(runner, CharReaderFailIfExtraTest, issue164);
  JSONTEST_REGISTER_FIXTURE(runner, CharReaderFailIfExtraTest, issue107);
  JSONTEST_REGISTER_FIXTURE(runner, CharReaderFailIfExtraTest, commentAfterObject);
  JSONTEST_REGISTER_FIXTURE(runner, CharReaderFailIfExtraTest, commentAfterArray);
  JSONTEST_REGISTER_FIXTURE(runner, CharReaderFailIfExtraTest, commentAfterBool);

  JSONTEST_REGISTER_FIXTURE(runner, CharReaderAllowDropNullTest, issue178);

  JSONTEST_REGISTER_FIXTURE(runner, CharReaderAllowSingleQuotesTest, issue182);

  JSONTEST_REGISTER_FIXTURE(runner, CharReaderAllowZeroesTest, issue176);

  JSONTEST_REGISTER_FIXTURE(runner, CharReaderAllowSpecialFloatsTest, issue209);

  JSONTEST_REGISTER_FIXTURE(runner, BuilderTest, settings);

  JSONTEST_REGISTER_FIXTURE(runner, IteratorTest, distance);
  JSONTEST_REGISTER_FIXTURE(runner, IteratorTest, names);
  JSONTEST_REGISTER_FIXTURE(runner, IteratorTest, indexes);
  JSONTEST_REGISTER_FIXTURE(runner, IteratorTest, const);

  JSONTEST_REGISTER_FIXTURE(runner, RValueTest, moveConstruction);

  return runner.runCommandLine(argc, argv);
}
