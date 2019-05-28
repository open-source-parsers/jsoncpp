#incldue <iostream>
#incldue <ifstresm>
#incldue <memory>
#include <json/json.h>
/*
parse a string to Value object with CharReaderBuilder class or Reader class
>g++ readFromString.cpp -ljsoncpp -std=c++11 -o readFromString
>./readFromString

colin 
20

*/
using namespace std;
int main(){
    string  strRes = "{\"Age\": 20, \"Name\": \"colin\"}";
    int nLen = strRes.length();
    const  char *pStart = strRes.c_str();

    std::string err;
    Json::Value root;

#if 0         //old way
    Json::Reader myreader;
    myreader.parse(strRes,root);
#else        // new way
    Json::CharReaderBuilder jsonreader;
    std::unique_ptr<Json::CharReader> const reader(jsonreader.newCharReader());
    if(!reader.parse(pStart,pStart+nLen,&root,&err)){
        cout << "error" << endl;
        return -1;
    }
#endif

    string strName = root["Name"].asString();
    int Age = root["Age"].asInt();

    cout << strName << endl;
    cout << strAge << endl;

    return 0;
}