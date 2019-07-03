#include "json/json.h"
/*
parse a string to Value object with CharReaderBuilder class or Reader class
>g++ readFromString.cpp -ljsoncpp -std=c++11 -o readFromString
>./readFromString
colin
20
*/
int main() {
  std::string strRes = "{\"Age\": 20, \"Name\": \"colin\"}";
  int nLen = (int)strRes.length();
  const char* pStart = strRes.c_str();

  JSONCPP_STRING errs;
  Json::Value root;

#if 0 // old way
    Json::Reader myreader;
    myreader.parse(strRes,root);
#else // new way
  Json::CharReaderBuilder jsonreader;
  Json::CharReader* reader(jsonreader.newCharReader());
  if (!reader.parse(pStart, pStart + nLen, &root, &err)) {
    // std::cout << "error" << std::endl;
    return -1;
  }
#endif
  std::string strName = root["Name"].asString();
  int Age = root["Age"].asInt();

  // std::cout << strName << std::endl;
  // std::cout << strAge << std::endl;
  delete reader;
  return 0;
}
