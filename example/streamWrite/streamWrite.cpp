#include "json/json.h"
#include <sstream>
/*
write the Value object to stream
>g++ streamWrite.cpp -ljsoncpp -std=c++11 -o streamWrite
>./streamWrite
{
    "Age" : 20,
    "Name" : "robin"
}
*/
int main() {
  std::string strRes = " ";
  Json::Value root;
  Json::StreamWriterBuilder jsonbuilder;
  Json::StreamWriter* writer(jsonbuilder.newStreamWriter());
  std::ostringstream os;

  root["Name"] = "robin";
  root["Age"] = 20;
  writer->write(root, &os);
  strRes = os.str();

  // std::cout << strRes <<std::endl;
  delete writer;
  return 0;
}
