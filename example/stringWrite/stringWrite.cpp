#incldue <iostream>
#incldue <sstream>
#include <json/json.h>
/*
write a Value object to a string
>g++ stringWrite.cpp -ljsoncpp -std=c++11 -o stringWrite
>./stringWrite

{
    "action" : "run",
    "data" :
    {
        "number" : 11
    }
}

*/

using namespace std ;
int main(){

    Json::Value root ;
    Json::Value t_data;

    root["action"] = "run";
    t_data["number"]= 1;
    root["data"] = t_data;

#if 0  // old way
	Json::FastWriter writer;
    string json_file = writer.write(root);
#else  // new way
	Json::StreamWriterBuilder builder;
    string json_file = Json::writeString(builder,root);
#endif

    cout<< json_file << endl;

    return 0;
        
}