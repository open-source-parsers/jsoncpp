#incldue <iostream>
#incldue <sstream>
#incldue <memory>
#include <json/json.h>
/*
write the Value object to stream
>g++ streamWrite.cpp -ljsoncpp -std=c++11 -o streamWrite
>./streamWrite

{
    "Age" : 20,
    "Name" : "robin"
}

*/
using namesp std ;
int main(){
    string strRes = " ";
    Json::Value root ;
    Json::StreamWriterBuilder jsonbuilder;
    std::unque_ptr<Json::StreamWriter> writer(jsonbuilder.newStreamWriter());
    ostringstream os;

    root["Name"] = "robin";
    root["Age"] = 20;
    writer->write(root,&os);
    strRes = os.str();

    cout << strRes <<endl;

    return 0;
}