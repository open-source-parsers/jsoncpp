// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#include <json/json.h>
#include "jsontest.h"


// TODO:
// - boolean value returns that they are integral. Should not be.
// - unsigned integer in integer range are not considered to be valid integer. Should check range.


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
      bool isDouble_;
      bool isInt_;
      bool isUInt_;
      bool isIntegral_;
      bool isNumeric_;
      bool isString_;
      bool isNull_;
   };

   void checkConstMemberCount( const Json::Value &value, unsigned int expectedCount );

   void checkMemberCount( Json::Value &value, unsigned int expectedCount );

   void checkIs( const Json::Value &value, const IsCheck &check );

   void checkIsLess( const Json::Value &x, const Json::Value &y );

   void checkIsEqual( const Json::Value &x, const Json::Value &y );
};


JSONTEST_FIXTURE( ValueTest, size )
{
   JSONTEST_ASSERT_PRED( checkMemberCount(emptyArray_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(emptyObject_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(array1_, 1) );
   JSONTEST_ASSERT_PRED( checkMemberCount(object1_, 1) );
   JSONTEST_ASSERT_PRED( checkMemberCount(null_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(integer_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(real_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(emptyString_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(string_, 0) );
   JSONTEST_ASSERT_PRED( checkMemberCount(true_, 0) );
}


JSONTEST_FIXTURE( ValueTest, isObject )
{
   IsCheck checks;
   checks.isObject_ = true;
   JSONTEST_ASSERT_PRED( checkIs( emptyObject_, checks ) );
   JSONTEST_ASSERT_PRED( checkIs( object1_, checks ) );
}


JSONTEST_FIXTURE( ValueTest, isArray )
{
   IsCheck checks;
   checks.isArray_ = true;
   JSONTEST_ASSERT_PRED( checkIs( emptyArray_, checks ) );
   JSONTEST_ASSERT_PRED( checkIs( array1_, checks ) );
}


JSONTEST_FIXTURE( ValueTest, isNull )
{
   IsCheck checks;
   checks.isNull_ = true;
   checks.isObject_ = true;
   checks.isArray_ = true;
   JSONTEST_ASSERT_PRED( checkIs( null_, checks ) );
}


JSONTEST_FIXTURE( ValueTest, isString )
{
   IsCheck checks;
   checks.isString_ = true;
   JSONTEST_ASSERT_PRED( checkIs( emptyString_, checks ) );
   JSONTEST_ASSERT_PRED( checkIs( string_, checks ) );
   JSONTEST_ASSERT_PRED( checkIs( string1_, checks ) );
}


JSONTEST_FIXTURE( ValueTest, isBool )
{
   IsCheck checks;
   checks.isBool_ = true;
   checks.isIntegral_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( false_, checks ) );
   JSONTEST_ASSERT_PRED( checkIs( true_, checks ) );
}


JSONTEST_FIXTURE( ValueTest, isDouble )
{
   IsCheck checks;
   checks.isDouble_ = true;
   checks.isNumeric_ = true;
   JSONTEST_ASSERT_PRED( checkIs( real_, checks ) );
}


JSONTEST_FIXTURE( ValueTest, isInt )
{
   IsCheck checks;
   checks.isInt_ = true;
   checks.isNumeric_ = true;
   checks.isIntegral_ = true;
   JSONTEST_ASSERT_PRED( checkIs( integer_, checks ) );
}


JSONTEST_FIXTURE( ValueTest, isUInt )
{
   IsCheck checks;
   checks.isUInt_ = true;
   checks.isNumeric_ = true;
   checks.isIntegral_ = true;
   JSONTEST_ASSERT_PRED( checkIs( unsignedInteger_, checks ) );
   JSONTEST_ASSERT_PRED( checkIs( smallUnsignedInteger_, checks ) );
}


JSONTEST_FIXTURE( ValueTest, accessArray )
{
	const unsigned int index0 = 0;
	JSONTEST_ASSERT( Json::Value(1234) == array1_[index0] ) << "Json::Value::operator[ArrayIndex]";
	JSONTEST_ASSERT( Json::Value(1234) == array1_[0] ) << "Json::Value::operator[int]";

	const Json::Value &constArray = array1_;
	JSONTEST_ASSERT( Json::Value(1234) == constArray[index0] ) << "Json::Value::operator[ArrayIndex] const";
	JSONTEST_ASSERT( Json::Value(1234) == constArray[0] ) << "Json::Value::operator[int] const";
}


JSONTEST_FIXTURE( ValueTest, asFloat )
{
	JSONTEST_ASSERT_EQUAL( 0.00390625f, float_.asFloat() ) << "Json::Value::asFloat()";
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
   , isDouble_( false )
   , isInt_( false )
   , isUInt_( false )
   , isIntegral_( false )
   , isNumeric_( false )
   , isString_( false )
   , isNull_( false )
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
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, size );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, isObject );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, isArray );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, isBool );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, isInt );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, isUInt );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, isDouble );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, isString );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, isNull );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, isNull );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, accessArray );
   JSONTEST_REGISTER_FIXTURE( runner, ValueTest, asFloat );
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
