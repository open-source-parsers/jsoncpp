#include "json/json.h"
#include <iostream>
/*
parse a string to Value object with CharReaderBuilder class or Reader class
$g++ readFromString.cpp -ljsoncpp -std=c++11 -o readFromString
$./readFromString
colin
20
*/
int main() {
  std::string strRes = R"({"Age": 20, "Name": "colin"})";
  auto nLen = (int)strRes.length();
  JSONCPP_STRING err;
  Json::Value root;

#if 0 // old way
    Json::Reader myreader;
    myreader.parse(strRes,root);
#else // new way
  Json::CharReaderBuilder builder;
  std::unique_ptr<Json::CharReader> const reader(builder.newCharReader());
  if (!reader->parse(strRes.c_str(), strRes.c_str() + nLen, &root, &err)) {
    std::cout << "error" << std::endl;
    return EXIT_FAILURE;
  }
#endif
  std::string strName = root["Name"].asString();
  int strAge = root["Age"].asInt();

  std::cout << strName << std::endl;
  std::cout << strAge << std::endl;
  return EXIT_SUCCESS;
}
