#include"Json.h"
using namespace std;

int main()
{
	Json j;
	string str; 
	string temp;
	getline(cin, temp);
	while (temp.size()!=0)
	{
		str += temp;
		str += '\n';
		getline(cin, temp);
	}
	j.loads(str);
	j.get_value<string>(0,string("_id"));
}