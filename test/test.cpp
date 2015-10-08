#include <json/json.h>
//#include "../jsoncpp/include/json/json.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;


int main()
{
	Json::Value value;

	for(int i = 10; i < 20; ++i)
	{
		stringstream sstream;	
		sstream << i;
		string str;
		sstream >> str;
		//cout << str << endl;
		value[str] = str;	
	}	

	//in old code, this will get a compile error
	Json::Value::const_iterator iter = value.begin();
	for(; iter != value.end(); ++iter)
	{
		cout << *iter << endl;
	}
	return 0;
}
