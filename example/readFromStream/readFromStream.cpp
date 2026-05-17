#include "json/json.h"
#include <fstream>
#include <iostream>
/** \brief Parse from stream, collect comments, access data, and capture error info.
 * Example Usage:
 * $g++ readFromStream.cpp -ljsoncpp -std=c++11 -o readFromStream
 * $./readFromStream
 * // comment head
 * [
 *    // comment before
 *    {
 *     "key" :
 *      {
 *        "id" : 1,
 *        "val" : "value 1"
 *      }
 *    },
 *    {
 *      "key" :
 *      {
 *        "id" : 2,
 *        "val" : "value 2"
 *      }
 *    }
 *  ] // comment tail
 * // comment after
 * 1
 * value 1
 * 2
 * value 2
 */
int main(int argc, char* argv[]) {
  Json::Value root;
  std::ifstream ifs;
  ifs.open(argv[1]);

  Json::CharReaderBuilder builder;
  builder["collectComments"] = true;
  JSONCPP_STRING errs;
  if (!parseFromStream(builder, ifs, &root, &errs)) {
    std::cout << errs << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << root << std::endl;

  for (Json::Value::const_iterator it = root.begin(); it != root.end(); ++it) {
    int id = it->get("key", "").get("id", "").asInt();
    std::string val = it->get("key", "").get("val", "").asString();

    std::cout << id << std::endl;
    std::cout << val << std::endl;
  }

  return EXIT_SUCCESS;
}
