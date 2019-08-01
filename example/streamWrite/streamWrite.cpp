#include "json/json.h"
#include <iostream>
/*
write the Value object to stream
$g++ streamWrite.cpp -ljsoncpp -std=c++11 -o streamWrite
$./streamWrite
{
    "Age" : 20,
    "Name" : "robin"
}
*/
int main() {
  Json::Value root;
  Json::StreamWriterBuilder builder;
  std::unique_ptr<Json::StreamWriter> const writer(builder.newStreamWriter());

  root["Name"] = "robin";
  root["Age"] = 20;
  writer->write(root, &std::cout);

  return EXIT_SUCCESS;
}
