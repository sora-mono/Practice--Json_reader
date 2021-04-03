#include<string>
#include"Json.h"
using std::string;
class Json;
class node;

#ifndef TEMPLATE_GETVALUE
#define TEMPLATE_GETVALUE
template<>
string& Json::node::get_value_reference<string>()const
{
	if (type == type_list::LEAF_STRING)
	{
		return *get<string*>(*p);
	}
	else
	{
		throw invalid_argument("No (string) type data saved in this node");
	}
}

template<>
int& Json::node::get_value_reference<int>()const
{
	if (type == type_list::LEAF_INT)
	{
		return get<int>(*p);
	}
	else
	{
		throw invalid_argument("No (int) type saved in this node");
	}
}

template<>
double& Json::node::get_value_reference<double>()const
{
	if (type == type_list::LEAF_DOUBLE)
	{
		return get<double>(*p);
	}
	else
	{
		throw invalid_argument("No (double) type saved in this node");
	}
}

template<>
bool& Json::node::get_value_reference<bool>()const
{
	if (type == type_list::LEAF_BOOL)
	{
		return get<bool>(*p);
	}
	else
	{
		throw invalid_argument("No (bool) type saved in this node");
	}
}

template<>
const char* Json::node::get_value_noreference<const char*>()
{
	if (type == type_list::LEAF_STRING)
	{
		return get<string*>(*p)->c_str();
	}
	else
	{
		throw invalid_argument("No (const char*) type data saved in this node");
	}
}

template<>
Json Json::node::get_value_noreference<Json>()
{
	return Json(*this);
}
#endif // !TEMPLATE_GETVALUE