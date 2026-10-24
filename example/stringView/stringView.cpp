#include "json/json.h"
#include <cstdlib>
#include <iostream>

#if defined(JSONCPP_HAS_STRING_VIEW)
#include <string_view>
#endif

/**
 * \brief Example using std::string_view with JsonCpp.
 */
int main() {
  Json::Value root;
  root["key"] = "value";

#if defined(JSONCPP_HAS_STRING_VIEW)
  std::cout << "Has string_view support" << std::endl;
  std::string_view sv("key");
  if (root.isMember(sv)) {
    std::cout << root[sv].asString() << std::endl;
  }
#else
  std::cout << "No string_view support" << std::endl;
  if (root.isMember("key")) {
    std::cout << root["key"].asString() << std::endl;
  }
#endif
  return EXIT_SUCCESS;
}
