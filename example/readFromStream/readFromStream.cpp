#include "json/json.h"
#include <fstream>
#include <iostream>
/*
parse from stream,collect comments and capture error info.

$g++ readFromStream.cpp -ljsoncpp -std=c++11 -o readFromStream
$./readFromStream
// comment head
{
    // comment before
    "key" : "value"
}
// comment after
// comment tail
*/
int main(int argc, char* argv[]) {
  Json::Value jsonRoot;
  std::ifstream ifs;
  ifs.open(argv[1]);
  jsonRoot.clear();

  Json::CharReaderBuilder builder;
  builder["collectComments"] = true;
  JSONCPP_STRING errs;
  if (!parseFromStream(builder, ifs, &jsonRoot, &errs)) {
    std::cout << errs << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << jsonRoot << std::endl;
  return EXIT_SUCCESS;
}
