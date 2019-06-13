#incldue <iostream>
#incldue <ifstresm>
#incldue <sstream>
#include <json/json.h>
/*
parse from stream,collect comments and capture error info.

>g++ readFromStream.cpp -ljsoncpp -std=c++11 -o readFromStream
>./readFromStream

// comment head
{
    // comment before
    "key" : "value"
}
// comment after
// comment tail

*/

using namespace std;
int main(int argc,char* argv[]){
    Json::Value jsonRoot;
    Json::Value jsonItem;
    ifs.open(argv[1]);
    jsonRoot.clear();

    Json::CharReaderBuilder builder;
    builder["collectComments"]=true;
    JSONCPP_STRING errs;
    if(!parseFromStream(builder,ifs,&jsonRoot,&errs)){
        cout << errs <<endl;
        return -1;
    }
    cout << jsonRoot <<endl;
    return 0;
}