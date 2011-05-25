// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#include <stdint.h>
#include <limits>

#include <json/config.h>
#include <json/json.h>
#include "jsontest.h"

// TODO:
// - boolean value returns that they are integral. Should not be.
// - unsigned integer in integer range are not considered to be valid integer. Should check range.

// Make numeric limits more convenient to talk about.
#define kint32max std::numeric_limits<int32_t>::max()
#define kint32min std::numeric_limits<int32_t>::min()
#define kuint32max std::numeric_limits<uint32_t>::max()
#define kint64max std::numeric_limits<int64_t>::max()
#define kint64min std::numeric_limits<int64_t>::min()
#define kuint64max std::numeric_limits<uint64_t>::max()

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Json Library test cases
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////


struct ValueTest : JsonTest::TestCase
{
   Json::Value null_;
   Json::Value emptyArray_;
   Json::Value emptyObject_;
   Json::Value integer_;
   Json::Value unsignedInteger_;
   Json::Value smallUnsignedInteger_;
   Json::Value real_;
   Json::Value float_;
   Json::Value array1_;
   Json::Value object1_;
   Json::Value emptyString_;
   Json::Value string1_;
   Json::Value string_;
   Json::Value true_;
   Json::Value false_;


   ValueTest()
      : emptyArray_( Json::arrayValue )
      , emptyObject_( Json::objectValue )
      , integer_( 123456789 )
      , smallUnsignedInteger_( Json::Value::UInt( Json::Value::maxInt ) )
      , unsignedInteger_( 34567890u )
      , real_( 1234.56789 )
      , float_( 0.00390625f )
      , emptyString_( "" )
      , string1_( "a" )
      , string_( "sometext with space" )
      , true_( true )
      , false_( false )
   {
      array1_.append( 1234 );
      object1_["id"] = 1234;
   }

   struct IsCheck
   {
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

   void checkConstMemberCount( const Json::Value &value, unsigned int expectedCount );

   void checkMemberCount( Json::Value &value, unsigned int expectedCount );

   void checkIs( const Json::Value &value, const IsCheck &check );

   void checkIsLess( const Json::Value &x, const Json::Value &y );

   void checkIsEqual( const Json::Value &x, const Json::Value &y );
};


JSONTEST_FIXTURE( ValueTest, memberCount )
{
   JSONTEST_ASSERT_PRED( checkMemberCount(emptyArray_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(emptyObject_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(array1_, 1) );
   JSONTEST_ASSERT_PRED( checkMemberCount(object1_, 1) );
   JSONTEST_ASSERT_PRED( checkMemberCount(null_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(integer_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(unsignedInteger_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(smallUnsignedInteger_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(real_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(emptyString_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(string_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(true_, 0) );
}


JSONTEST_FIXTURE( ValueTest, objects )
{
   // Types
   IsCheck checks;
   checks.isObject_ = true;
   JSONTEST_ASSERT_PRED( checkIs( emptyObject_, checks ) );
   JSONTEST_ASSERT_PRED( checkIs( object1_, checks ) );

   JSONTEST_ASSERT_EQUAL( Json::objectValue, emptyObject_.type());

   // Access through const reference
   const Json::Value &constObject = object1_;

   JSONTEST_ASSERT( Json::Value(1234) == constObject["id"] );
   JSONTEST_ASSERT( Json::Value() == constObject["unknown id"] );

   // Access through non-const reference
   JSONTEST_ASSERT( Json::Value(1234) == object1_["id"] );
   JSONTEST_ASSERT( Json::Value() == object1_["unknown id"] );

   object1_["some other id"] = "foo";
   JSONTEST_ASSERT( Json::Value("foo") == object1_["some other id"] );
}


JSONTEST_FIXTURE( ValueTest, arrays )
{
   const unsigned int index0 = 0;

   // Types
   IsCheck checks;
   checks.isArray_ = true;
   JSONTEST_ASSERT_PRED( checkIs( emptyArray_, checks ) );
   JSONTEST_ASSERT_PRED( checkIs( array1_, checks ) );

   JSONTEST_ASSERT_EQUAL( Json::arrayValue, array1_.type());

   // Access through const reference
   const Json::Value &constArray = array1_;
   JSONTEST_ASSERT( Json::Value(1234) == constArray[index0] );
   JSONTEST_ASSERT( Json::Value(1234) == constArray[0] );

   // Access through non-const reference
   JSONTEST_ASSERT( Json::Value(1234) == array1_[index0] );
   JSONTEST_ASSERT( Json::Value(1234) == array1_[0] );

   array1_[2] = Json::Value(17);
   JSONTEST_ASSERT( Json::Value() == array1_[1] );
   JSONTEST_ASSERT( Json::Value(17) == array1_[2] );
}


JSONTEST_FIXTURE( ValueTest, null )
{
   JSONTEST_ASSERT_EQUAL( Json::nullValue, null_.type());

   IsCheck checks;
   checks.isNull_ = true;
   JSONTEST_ASSERT_PRED( checkIs( null_, checks ) );
}


JSONTEST_FIXTURE( ValueTest, strings )
{
   JSONTEST_ASSERT_EQUAL( Json::stringValue, string1_.type());

   IsCheck checks;
   checks.isString_ = true;
   JSONTEST_ASSERT_PRED( checkIs( emptyString_, checks ) );
   JSONTEST_ASSERT_PRED( checkIs( string_, checks ) );
   JSONTEST_ASSERT_PRED( checkIs( string1_, checks ) );

   JSONTEST_ASSERT( std::string("a") == string1_.asString());
   JSONTEST_ASSERT( std::string("a") == string1_.asCString());
}


JSONTEST_FIXTURE( ValueTest, bools )
{
   JSONTEST_ASSERT_EQUAL( Json::booleanValue, false_.type());

   IsCheck checks;
   checks.isBool_ = true;
   JSONTEST_ASSERT_PRED( checkIs( false_, checks ) );
   JSONTEST_ASSERT_PRED( checkIs( true_, checks ) );

   JSONTEST_ASSERT( true == true_.asBool());
   JSONTEST_ASSERT( false == false_.asBool());
}


JSONTEST_FIXTURE( ValueTest, integers )
{
   IsCheck checks;
   Json::Value val;

   // Default int
   val = Json::Value(Json::intValue);

   JSONTEST_ASSERT_EQUAL(Json::intValue, val.type());

   checks = IsCheck();
   checks.isInt_ = true;
   checks.isInt64_ = true;
   checks.isUInt_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( 0 == val.asInt());
   JSONTEST_ASSERT( 0 == val.asLargestInt());
   JSONTEST_ASSERT( 0 == val.asUInt());
   JSONTEST_ASSERT( 0 == val.asLargestUInt());
   JSONTEST_ASSERT( 0.0 == val.asDouble());
   JSONTEST_ASSERT( 0.0 == val.asFloat());

   // Default uint
   val = Json::Value(Json::uintValue);

   JSONTEST_ASSERT_EQUAL(Json::uintValue, val.type());

   checks = IsCheck();
   checks.isInt_ = true;
   checks.isInt64_ = true;
   checks.isUInt_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( 0 == val.asInt());
   JSONTEST_ASSERT( 0 == val.asLargestInt());
   JSONTEST_ASSERT( 0 == val.asUInt());
   JSONTEST_ASSERT( 0 == val.asLargestUInt());
   JSONTEST_ASSERT( 0.0 == val.asDouble());
   JSONTEST_ASSERT( 0.0 == val.asFloat());

   // Default real
   val = Json::Value(Json::realValue);

   JSONTEST_ASSERT_EQUAL(Json::realValue, val.type());

   checks = IsCheck();
   checks.isInt_ = true;
   checks.isInt64_ = true;
   checks.isUInt_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( 0 == val.asInt());
   JSONTEST_ASSERT( 0 == val.asLargestInt());
   JSONTEST_ASSERT( 0 == val.asUInt());
   JSONTEST_ASSERT( 0 == val.asLargestUInt());
   JSONTEST_ASSERT( 0.0 == val.asDouble());
   JSONTEST_ASSERT( 0.0 == val.asFloat());

   // Zero (signed constructor arg)
   val = Json::Value(0);

   JSONTEST_ASSERT_EQUAL( Json::intValue, val.type());

   checks = IsCheck();
   checks.isInt_ = true;
   checks.isInt64_ = true;
   checks.isUInt_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( 0 == val.asInt());
   JSONTEST_ASSERT( 0 == val.asLargestInt());
   JSONTEST_ASSERT( 0 == val.asUInt());
   JSONTEST_ASSERT( 0 == val.asLargestUInt());
   JSONTEST_ASSERT( 0.0 == val.asDouble());
   JSONTEST_ASSERT( 0.0 == val.asFloat());

   // Zero (unsigned constructor arg)
   val = Json::Value(0u);

   JSONTEST_ASSERT_EQUAL( Json::uintValue, val.type());

   checks = IsCheck();
   checks.isInt_ = true;
   checks.isInt64_ = true;
   checks.isUInt_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( 0 == val.asInt());
   JSONTEST_ASSERT( 0 == val.asLargestInt());
   JSONTEST_ASSERT( 0 == val.asUInt());
   JSONTEST_ASSERT( 0 == val.asLargestUInt());
   JSONTEST_ASSERT( 0.0 == val.asDouble());
   JSONTEST_ASSERT( 0.0 == val.asFloat());

   // Zero (floating-point constructor arg)
   val = Json::Value(0.0);

   JSONTEST_ASSERT_EQUAL( Json::realValue, val.type());

   checks = IsCheck();
   checks.isInt_ = true;
   checks.isInt64_ = true;
   checks.isUInt_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( 0 == val.asInt());
   JSONTEST_ASSERT( 0 == val.asLargestInt());
   JSONTEST_ASSERT( 0 == val.asUInt());
   JSONTEST_ASSERT( 0 == val.asLargestUInt());
   JSONTEST_ASSERT( 0.0 == val.asDouble());
   JSONTEST_ASSERT( 0.0 == val.asFloat());

   // 2^20 (signed constructor arg)
   val = Json::Value(1 << 20);

   JSONTEST_ASSERT_EQUAL( Json::intValue, val.type());

   checks = IsCheck();
   checks.isInt_ = true;
   checks.isInt64_ = true;
   checks.isUInt_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( (1 << 20) == val.asInt());
   JSONTEST_ASSERT( (1 << 20) == val.asLargestInt());
   JSONTEST_ASSERT( (1 << 20) == val.asUInt());
   JSONTEST_ASSERT( (1 << 20) == val.asLargestUInt());
   JSONTEST_ASSERT( (1 << 20) == val.asDouble());
   JSONTEST_ASSERT( (1 << 20) == val.asFloat());

   // 2^20 (unsigned constructor arg)
   val = Json::Value(1u << 20);

   JSONTEST_ASSERT_EQUAL( Json::uintValue, val.type());

   checks = IsCheck();
   checks.isInt_ = true;
   checks.isInt64_ = true;
   checks.isUInt_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( (1 << 20) == val.asInt());
   JSONTEST_ASSERT( (1 << 20) == val.asLargestInt());
   JSONTEST_ASSERT( (1 << 20) == val.asUInt());
   JSONTEST_ASSERT( (1 << 20) == val.asLargestUInt());
   JSONTEST_ASSERT( (1 << 20) == val.asDouble());
   JSONTEST_ASSERT( (1 << 20) == val.asFloat());

   // 2^20 (floating-point constructor arg)
   val = Json::Value((1 << 20) / 1.0);

   JSONTEST_ASSERT_EQUAL( Json::realValue, val.type());

   checks = IsCheck();
   checks.isInt_ = true;
   checks.isInt64_ = true;
   checks.isUInt_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( (1 << 20) == val.asInt());
   JSONTEST_ASSERT( (1 << 20) == val.asLargestInt());
   JSONTEST_ASSERT( (1 << 20) == val.asUInt());
   JSONTEST_ASSERT( (1 << 20) == val.asLargestUInt());
   JSONTEST_ASSERT( (1 << 20) == val.asDouble());
   JSONTEST_ASSERT( (1 << 20) == val.asFloat());

   // -2^20
   val = Json::Value(-(1 << 20));

   JSONTEST_ASSERT_EQUAL( Json::intValue, val.type());

   checks = IsCheck();
   checks.isInt_ = true;
   checks.isInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( -(1 << 20) == val.asInt());
   JSONTEST_ASSERT( -(1 << 20) == val.asLargestInt());
   JSONTEST_ASSERT( -(1 << 20) == val.asDouble());
   JSONTEST_ASSERT( -(1 << 20) == val.asFloat());

   // int32 max
   val = Json::Value(kint32max);

   JSONTEST_ASSERT_EQUAL( Json::intValue, val.type());

   checks = IsCheck();
   checks.isInt_ = true;
   checks.isInt64_ = true;
   checks.isUInt_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( kint32max == val.asInt());
   JSONTEST_ASSERT( kint32max == val.asLargestInt());
   JSONTEST_ASSERT( kint32max == val.asUInt());
   JSONTEST_ASSERT( kint32max == val.asLargestUInt());
   JSONTEST_ASSERT( kint32max == val.asDouble());
   JSONTEST_ASSERT( kint32max == val.asFloat());

   // int32 min
   val = Json::Value(kint32min);

   JSONTEST_ASSERT_EQUAL( Json::intValue, val.type());

   checks = IsCheck();
   checks.isInt_ = true;
   checks.isInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( kint32min == val.asInt());
   JSONTEST_ASSERT( kint32min == val.asLargestInt());
   JSONTEST_ASSERT( kint32min == val.asDouble());
   JSONTEST_ASSERT( kint32min == val.asFloat());

   // uint32 max
   val = Json::Value(kuint32max);

   JSONTEST_ASSERT_EQUAL( Json::uintValue, val.type());

   checks = IsCheck();
   checks.isInt64_ = true;
   checks.isUInt_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

#ifndef JSON_NO_INT64
   JSONTEST_ASSERT( kuint32max == val.asLargestInt());
#endif
   JSONTEST_ASSERT( kuint32max == val.asUInt());
   JSONTEST_ASSERT( kuint32max == val.asLargestUInt());
   JSONTEST_ASSERT( kuint32max == val.asDouble());
   JSONTEST_ASSERT( kuint32max == val.asFloat());

#ifdef JSON_NO_INT64
   // int64 max
   val = Json::Value(double(kint64max));

   JSONTEST_ASSERT_EQUAL( Json::realValue, val.type());

   checks = IsCheck();
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( double(kint64max) == val.asDouble());
   JSONTEST_ASSERT( float(kint64max) == val.asFloat());

   // int64 min
   val = Json::Value(double(kint64min));

   JSONTEST_ASSERT_EQUAL( Json::realValue, val.type());

   checks = IsCheck();
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( double(kint64min) == val.asDouble());
   JSONTEST_ASSERT( float(kint64min) == val.asFloat());

   // uint64 max
   val = Json::Value(double(kuint64max));

   JSONTEST_ASSERT_EQUAL( Json::realValue, val.type());

   checks = IsCheck();
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( double(kuint64max) == val.asDouble());
   JSONTEST_ASSERT( float(kuint64max) == val.asFloat());
#else  // ifdef JSON_NO_INT64
   // 2^40 (signed constructor arg)
   val = Json::Value(1LL << 40);

   JSONTEST_ASSERT_EQUAL( Json::intValue, val.type());

   checks = IsCheck();
   checks.isInt64_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( (1LL << 40) == val.asInt64());
   JSONTEST_ASSERT( (1LL << 40) == val.asLargestInt());
   JSONTEST_ASSERT( (1LL << 40) == val.asUInt64());
   JSONTEST_ASSERT( (1LL << 40) == val.asLargestUInt());
   JSONTEST_ASSERT( (1LL << 40) == val.asDouble());
   JSONTEST_ASSERT( (1LL << 40) == val.asFloat());

   // 2^40 (unsigned constructor arg)
   val = Json::Value(1ULL << 40);

   JSONTEST_ASSERT_EQUAL( Json::uintValue, val.type());

   checks = IsCheck();
   checks.isInt64_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( (1LL << 40) == val.asInt64());
   JSONTEST_ASSERT( (1LL << 40) == val.asLargestInt());
   JSONTEST_ASSERT( (1LL << 40) == val.asUInt64());
   JSONTEST_ASSERT( (1LL << 40) == val.asLargestUInt());
   JSONTEST_ASSERT( (1LL << 40) == val.asDouble());
   JSONTEST_ASSERT( (1LL << 40) == val.asFloat());

   // 2^40 (floating-point constructor arg)
   val = Json::Value((1LL << 40) / 1.0);

   JSONTEST_ASSERT_EQUAL( Json::realValue, val.type());

   checks = IsCheck();
   checks.isInt64_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( (1LL << 40) == val.asInt64());
   JSONTEST_ASSERT( (1LL << 40) == val.asLargestInt());
   JSONTEST_ASSERT( (1LL << 40) == val.asUInt64());
   JSONTEST_ASSERT( (1LL << 40) == val.asLargestUInt());
   JSONTEST_ASSERT( (1LL << 40) == val.asDouble());
   JSONTEST_ASSERT( (1LL << 40) == val.asFloat());

   // -2^40
   val = Json::Value(-(1LL << 40));

   JSONTEST_ASSERT_EQUAL( Json::intValue, val.type());

   checks = IsCheck();
   checks.isInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( -(1LL << 40) == val.asInt64());
   JSONTEST_ASSERT( -(1LL << 40) == val.asLargestInt());
   JSONTEST_ASSERT( -(1LL << 40) == val.asDouble());
   JSONTEST_ASSERT( -(1LL << 40) == val.asFloat());

   // int64 max
   val = Json::Value(Json::Int64(kint64max));

   JSONTEST_ASSERT_EQUAL( Json::intValue, val.type());

   checks = IsCheck();
   checks.isInt64_ = true;
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( kint64max == val.asInt64());
   JSONTEST_ASSERT( kint64max == val.asLargestInt());
   JSONTEST_ASSERT( kint64max == val.asUInt64());
   JSONTEST_ASSERT( kint64max == val.asLargestUInt());
   JSONTEST_ASSERT( double(kint64max) == val.asDouble());
   JSONTEST_ASSERT( float(kint64max) == val.asFloat());

   // int64 max (floating point constructor). Note that kint64max is not exactly
   // representable as a double, and will be rounded up to be higher.
   val = Json::Value(double(kint64max));

   JSONTEST_ASSERT_EQUAL( Json::realValue, val.type());

   checks = IsCheck();
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( 9223372036854775808ULL == val.asUInt64());
   JSONTEST_ASSERT( 9223372036854775808ULL == val.asLargestUInt());
   JSONTEST_ASSERT( 9223372036854775808ULL == val.asDouble());
   JSONTEST_ASSERT( 9223372036854775808ULL == val.asFloat());

   // int64 min
   val = Json::Value(Json::Int64(kint64min));

   JSONTEST_ASSERT_EQUAL( Json::intValue, val.type());

   checks = IsCheck();
   checks.isInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( kint64min == val.asInt64());
   JSONTEST_ASSERT( kint64min == val.asLargestInt());
   JSONTEST_ASSERT( double(kint64min) == val.asDouble());
   JSONTEST_ASSERT( float(kint64min) == val.asFloat());

   // int64 min (floating point constructor). Note that kint64min *is* exactly
   // representable as a double.
   val = Json::Value(double(kint64min));

   JSONTEST_ASSERT_EQUAL( Json::realValue, val.type());

   checks = IsCheck();
   checks.isInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( -9223372036854775808LL == val.asInt64());
   JSONTEST_ASSERT( -9223372036854775808LL == val.asLargestInt());
   JSONTEST_ASSERT( -9223372036854775808.0 == val.asDouble());
   JSONTEST_ASSERT( -9223372036854775808.0 == val.asFloat());

   // uint64 max
   val = Json::Value(Json::UInt64(kuint64max));

   JSONTEST_ASSERT_EQUAL( Json::uintValue, val.type());

   checks = IsCheck();
   checks.isUInt64_ = true;
   checks.isIntegral_ = true;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( kuint64max == val.asUInt64());
   JSONTEST_ASSERT( kuint64max == val.asLargestUInt());
   JSONTEST_ASSERT( double(kuint64max) == val.asDouble());
   JSONTEST_ASSERT( float(kuint64max) == val.asFloat());

   // uint64 max (floating point constructor). Note that kuint64max is not
   // exactly representable as a double, and will be rounded up to be higher.
   val = Json::Value(double(kuint64max));

   JSONTEST_ASSERT_EQUAL( Json::realValue, val.type());

   checks = IsCheck();
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( 18446744073709551616.0 == val.asDouble());
   JSONTEST_ASSERT( 18446744073709551616.0 == val.asFloat());
#endif
}


JSONTEST_FIXTURE( ValueTest, nonIntegers )
{
   IsCheck checks;
   Json::Value val;

   // Positive number
   val = Json::Value(0.25);

   JSONTEST_ASSERT_EQUAL( Json::realValue, val.type());

   checks = IsCheck();
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( 0.25 == val.asDouble());
   JSONTEST_ASSERT( 0.25 == val.asFloat());

   // Negative number
   val = Json::Value(-0.25);

   JSONTEST_ASSERT_EQUAL( Json::realValue, val.type());

   checks = IsCheck();
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( val, checks ) );

   JSONTEST_ASSERT( -0.25 == val.asDouble());
   JSONTEST_ASSERT( -0.25 == val.asFloat());
}


void
ValueTest::checkConstMemberCount( const Json::Value &value, unsigned int expectedCount )
{
   unsigned int count = 0;
   Json::Value::const_iterator itEnd = value.end();
   for ( Json::Value::const_iterator it = value.begin(); it != itEnd; ++it )
   {
      ++count;
   }
   JSONTEST_ASSERT_EQUAL( expectedCount, count ) << "Json::Value::const_iterator";
}

void
ValueTest::checkMemberCount( Json::Value &value, unsigned int expectedCount )
{
   JSONTEST_ASSERT_EQUAL( expectedCount, value.size() );

   unsigned int count = 0;
   Json::Value::iterator itEnd = value.end();
   for ( Json::Value::iterator it = value.begin(); it != itEnd; ++it )
   {
      ++count;
   }
   JSONTEST_ASSERT_EQUAL( expectedCount, count ) << "Json::Value::iterator";

   JSONTEST_ASSERT_PRED( checkConstMemberCount(value, expectedCount) );
}


ValueTest::IsCheck::IsCheck()
   : isObject_( false )
   , isArray_( false )
   , isBool_( false )
   , isString_( false )
   , isNull_( false )
   , isInt_( false )
   , isInt64_( false )
   , isUInt_( false )
   , isUInt64_( false )
   , isIntegral_( false )
   , isDouble_( false )
   , isNumeric_( false )
{
}


void 
ValueTest::checkIs( const Json::Value &value, const IsCheck &check )
{
   JSONTEST_ASSERT_EQUAL( check.isObject_, value.isObject() );
   JSONTEST_ASSERT_EQUAL( check.isArray_, value.isArray() );
   JSONTEST_ASSERT_EQUAL( check.isBool_, value.isBool() );
   JSONTEST_ASSERT_EQUAL( check.isDouble_, value.isDouble() );
   JSONTEST_ASSERT_EQUAL( check.isInt_, value.isInt() );
   JSONTEST_ASSERT_EQUAL( check.isUInt_, value.isUInt() );
   JSONTEST_ASSERT_EQUAL( check.isIntegral_, value.isIntegral() );
   JSONTEST_ASSERT_EQUAL( check.isNumeric_, value.isNumeric() );
   JSONTEST_ASSERT_EQUAL( check.isString_, value.isString() );
   JSONTEST_ASSERT_EQUAL( check.isNull_, value.isNull() );

#ifdef JSON_HAS_INT64
   JSONTEST_ASSERT_EQUAL( check.isInt64_, value.isInt64() );
   JSONTEST_ASSERT_EQUAL( check.isUInt64_, value.isUInt64() );
#else
   JSONTEST_ASSERT_EQUAL( false, value.isInt64() );
   JSONTEST_ASSERT_EQUAL( false, value.isUInt64() );
#endif
}


JSONTEST_FIXTURE( ValueTest, compareNull )
{
    JSONTEST_ASSERT_PRED( checkIsEqual( Json::Value(), Json::Value() ) );
}


JSONTEST_FIXTURE( ValueTest, compareInt )
{
    JSONTEST_ASSERT_PRED( checkIsLess( 0, 10 ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( 10, 10 ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( -10, -10 ) );
    JSONTEST_ASSERT_PRED( checkIsLess( -10, 0 ) );
}


JSONTEST_FIXTURE( ValueTest, compareUInt )
{
    JSONTEST_ASSERT_PRED( checkIsLess( 0u, 10u ) );
    JSONTEST_ASSERT_PRED( checkIsLess( 0u, Json::Value::maxUInt ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( 10u, 10u ) );
}


JSONTEST_FIXTURE( ValueTest, compareDouble )
{
    JSONTEST_ASSERT_PRED( checkIsLess( 0.0, 10.0 ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( 10.0, 10.0 ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( -10.0, -10.0 ) );
    JSONTEST_ASSERT_PRED( checkIsLess( -10.0, 0.0 ) );
}


JSONTEST_FIXTURE( ValueTest, compareString )
{
    JSONTEST_ASSERT_PRED( checkIsLess( "", " " ) );
    JSONTEST_ASSERT_PRED( checkIsLess( "", "a" ) );
    JSONTEST_ASSERT_PRED( checkIsLess( "abcd", "zyui" ) );
    JSONTEST_ASSERT_PRED( checkIsLess( "abc", "abcd" ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( "abcd", "abcd" ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( " ", " " ) );
    JSONTEST_ASSERT_PRED( checkIsLess( "ABCD", "abcd" ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( "ABCD", "ABCD" ) );
}


JSONTEST_FIXTURE( ValueTest, compareBoolean )
{
    JSONTEST_ASSERT_PRED( checkIsLess( false, true ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( false, false ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( true, true ) );
}


JSONTEST_FIXTURE( ValueTest, compareArray )
{
    // array compare size then content
    Json::Value emptyArray(Json::arrayValue);
    Json::Value l1aArray;
    l1aArray.append( 0 );
    Json::Value l1bArray;
    l1bArray.append( 10 );
    Json::Value l2aArray;
    l2aArray.append( 0 );
    l2aArray.append( 0 );
    Json::Value l2bArray;
    l2bArray.append( 0 );
    l2bArray.append( 10 );
    JSONTEST_ASSERT_PRED( checkIsLess( emptyArray, l1aArray ) );
    JSONTEST_ASSERT_PRED( checkIsLess( emptyArray, l2aArray ) );
    JSONTEST_ASSERT_PRED( checkIsLess( l1aArray, l2aArray ) );
    JSONTEST_ASSERT_PRED( checkIsLess( l2aArray, l2bArray ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( emptyArray, Json::Value( emptyArray ) ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( l1aArray, Json::Value( l1aArray) ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( l2bArray, Json::Value( l2bArray) ) );
}


JSONTEST_FIXTURE( ValueTest, compareObject )
{
    // object compare size then content
    Json::Value emptyObject(Json::objectValue);
    Json::Value l1aObject;
    l1aObject["key1"] = 0;
    Json::Value l1bObject;
    l1aObject["key1"] = 10;
    Json::Value l2aObject;
    l2aObject["key1"] = 0;
    l2aObject["key2"] = 0;
    JSONTEST_ASSERT_PRED( checkIsLess( emptyObject, l1aObject ) );
    JSONTEST_ASSERT_PRED( checkIsLess( emptyObject, l2aObject ) );
    JSONTEST_ASSERT_PRED( checkIsLess( l1aObject, l2aObject ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( emptyObject, Json::Value( emptyObject ) ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( l1aObject, Json::Value( l1aObject ) ) );
    JSONTEST_ASSERT_PRED( checkIsEqual( l2aObject, Json::Value( l2aObject ) ) );
}


JSONTEST_FIXTURE( ValueTest, compareType )
{
    // object of different type are ordered according to their type
    JSONTEST_ASSERT_PRED( checkIsLess( Json::Value(), Json::Value(1) ) );
    JSONTEST_ASSERT_PRED( checkIsLess( Json::Value(1), Json::Value(1u) ) );
    JSONTEST_ASSERT_PRED( checkIsLess( Json::Value(1u), Json::Value(1.0) ) );
    JSONTEST_ASSERT_PRED( checkIsLess( Json::Value(1.0), Json::Value("a") ) );
    JSONTEST_ASSERT_PRED( checkIsLess( Json::Value("a"), Json::Value(true) ) );
    JSONTEST_ASSERT_PRED( checkIsLess( Json::Value(true), Json::Value(Json::arrayValue) ) );
    JSONTEST_ASSERT_PRED( checkIsLess( Json::Value(Json::arrayValue), Json::Value(Json::objectValue) ) );
}


void 
ValueTest::checkIsLess( const Json::Value &x, const Json::Value &y )
{
    JSONTEST_ASSERT( x < y );
    JSONTEST_ASSERT( y > x );
    JSONTEST_ASSERT( x <= y );
    JSONTEST_ASSERT( y >= x );
    JSONTEST_ASSERT( !(x == y) );
    JSONTEST_ASSERT( !(y == x) );
    JSONTEST_ASSERT( !(x >= y) );
    JSONTEST_ASSERT( !(y <= x) );
    JSONTEST_ASSERT( !(x > y) );
    JSONTEST_ASSERT( !(y < x) );
    JSONTEST_ASSERT( x.compare( y ) < 0 );
    JSONTEST_ASSERT( y.compare( x ) >= 0 );
}


void 
ValueTest::checkIsEqual( const Json::Value &x, const Json::Value &y )
{
    JSONTEST_ASSERT( x == y );
    JSONTEST_ASSERT( y == x );
    JSONTEST_ASSERT( x <= y );
    JSONTEST_ASSERT( y <= x );
    JSONTEST_ASSERT( x >= y );
    JSONTEST_ASSERT( y >= x );
    JSONTEST_ASSERT( !(x < y) );
    JSONTEST_ASSERT( !(y < x) );
    JSONTEST_ASSERT( !(x > y) );
    JSONTEST_ASSERT( !(y > x) );
    JSONTEST_ASSERT( x.compare( y ) == 0 );
    JSONTEST_ASSERT( y.compare( x ) == 0 );
}


int main( int argc, const char *argv[] )
{
   JsonTest::Runner runner;
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, memberCount );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, objects );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, arrays );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, null );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, strings );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, bools );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, integers );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, nonIntegers );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, compareNull );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, compareInt );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, compareUInt );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, compareDouble );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, compareString );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, compareBoolean );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, compareArray );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, compareObject );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, compareType );
   return runner.runCommandLine( argc, argv );
}
