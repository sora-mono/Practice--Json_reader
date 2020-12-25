#pragma once
#include<iostream>
#include<vector>
#include<map>
#include<stack>
#include<queue>
#include<variant>
#include<exception>
#include<string>


const int MAX_STR_LENGTH = 500;

using namespace std;

class Json;

template<class T>
concept Json_or_chararray =
(is_same<char*, typename decay<T>::type>::value
	|| is_same<const char*, typename decay<T>::type>::value
	|| is_same<Json, typename decay<T>::type>::value
	|| is_same<const Json, typename decay<T>::type>::value);

template<class T>
concept Not_string =
!(is_same<char*, typename decay<T>::type>::value
	|| is_same<const char*, typename decay<T>::type>::value
	|| is_same<string, typename decay<T>::type>::value
	|| is_same<const string, typename decay<T>::type>::value);

class Json
{
public:
	struct node
	{
		using pointer = variant
			<
			std::monostate,
			vector<shared_ptr<node>>*,
			map<variant<std::monostate, int, double, string>, shared_ptr<node>>*,
			string*,
			int,
			double,
			bool,
			shared_ptr<node>
			>;
		using obj_vec = vector<shared_ptr<node>>;
		using obj_map = map<variant<std::monostate, int, double, string>, shared_ptr<node>>;
		using obj_str = string;
		using obj_var = variant<std::monostate, int, double, string>;
		using num_int = int;
		using num_double = double;
		enum class type_list :char { ROOT,EMPTY,DICTIONARY, VECTOR, LEAF_INT, LEAF_DOUBLE, LEAF_STRING, LEAF_BOOL };
		inline node() :type(node::type_list::EMPTY), p(nullptr) {}
		template<class T>
		inline node(type_list type_, T p_);
		inline node(node::type_list type_) :type(type_), p(nullptr) {}
		node(node&& n)noexcept;
		node(const node& n) = delete;
		~node();
		void show(ostream& out, size_t level)const;
		void show(ostream& s, const obj_var& x)const;
		//template<class T>
		//pointer get_value(const T& t)const;
		//template<class T, class...Args>
		//pointer get_value(const T& t, const Args&...args)const;
		template<class T>
		inline T& get_value_reference()const;
		template<class T>
		inline T get_value_noreference()const;
		template<class T>
		inline T get_value_noreference();

		template<class Tr, class T, class...Args>
		inline Tr& get_value_reference(const T& t, const Args&...args);

		template<class Tr, Not_string T, class ...Args>
		inline Tr& get_value_reference(const T& t, const Args&...args);

		template<Json_or_chararray Tr, class T, class...Args>
		inline Tr get_value_noreference(const T& t, const Args&...args);

		template<Json_or_chararray Tr, Not_string T, class...Args>
		inline Tr get_value_noreference(const T& t, const Args&...args);

		type_list type;
		pointer* p;
	};

	Json();
	Json(const string& str);
	Json(const Json& j) = default;
	Json(Json&& j) noexcept;

	inline ~Json() {};
	Json& loads(const string& str);
	void output(ostream& out)const;
	auto get_type()const { return root.get()->type; }

	template<class Tr, class...Args>
	inline Tr get_value(const Args&...args)
		requires is_same<typename decay<Tr>::type, char*>::value
		|| is_same<typename decay<Tr>::type, const char*>::value
		|| is_same<typename decay<Tr>::type, Json>::value
		|| is_same<typename decay<Tr>::type, const Json>::value
	{ return root->get_value_noreference<Tr, Args...>(args...); }

	template<class Tr, class ...Args>
	inline Tr& get_value(const Args&...args) { return root->get_value_reference<Tr, Args...>(args...); }

private:
	shared_ptr<node> root;
	inline Json(Json::node& n) :root(&n) {}
	inline Json(shared_ptr<node>& root_) : root(root_) {}
};

template<Json_or_chararray Tr, class T, class...Args>
Tr Json::node::get_value_noreference(const T& t, const Args&...args)
{
	if (type == node::type_list::DICTIONARY)
	{
		obj_map* p_map = get<obj_map*>(*p);
		obj_map::iterator iter = p_map->find(obj_var(t));
		if (iter != p_map->end())
		{
			return iter->second.get()->get_value_noreference<Tr, const Args...>(args...);
		}
		else
		{
			throw invalid_argument("Can't find the given key in map");
		}
	}
	else
	{
		throw invalid_argument("Can't index with string type key for it's not a dictionary");
	}
}

template<Json_or_chararray Tr, Not_string T, class...Args>
Tr Json::node::get_value_noreference(const T& t, const Args&...args)
{
	switch (type)
	{
	case Json::node::type_list::EMPTY:
		throw invalid_argument("Emtpy node");
		break;
	case Json::node::type_list::ROOT:
		throw runtime_error("Type should never used by user");
		break;
	case Json::node::type_list::DICTIONARY:
	{
		obj_map* p_map = get<obj_map*>(*p);
		obj_map::iterator iter = p_map->find(obj_var(t));
		if (iter != p_map->end())
		{
			return iter->second.get()->get_value_noreference<Tr, const Args...>(args...);
		}
		else
		{
			throw invalid_argument("Can't find the given key in map");
		}
	}
	break;
	case Json::node::type_list::VECTOR:
	{
		obj_vec* p_vec = get<obj_vec*>(*p);
		return	(*p_vec)[t].get()->get_value_noreference<Tr, const Args...>(args...);
	}
	break;
	case Json::node::type_list::LEAF_INT:
	case Json::node::type_list::LEAF_DOUBLE:
	case Json::node::type_list::LEAF_STRING:
	case Json::node::type_list::LEAF_BOOL:
		throw invalid_argument("Can't index in a leaf node");
		break;
	default:
		throw runtime_error("Undefined type");
		break;
	}
}

template<class Tr, class T, class...Args>
Tr& Json::node::get_value_reference(const T& t, const Args&...args)
{
	if (type == node::type_list::DICTIONARY)
	{
		obj_map* p_map = get<obj_map*>(*p);
		obj_map::iterator iter = p_map->find(obj_var(t));
		if (iter != p_map->end())
		{
			return iter->second.get()->get_value_reference<Tr, const Args...>(args...);
		}
		else
		{
			throw invalid_argument("Can't find the given key in map");
		}
	}
	else
	{
		throw invalid_argument("Can't index with string type key for it's not a dictionary");
	}
}

template<class Tr, Not_string T, class...Args>
Tr& Json::node::get_value_reference(const T& t, const Args&...args)
{
	switch (type)
	{
	case Json::node::type_list::EMPTY:
		throw invalid_argument("Emtpy node");
		break;
	case Json::node::type_list::ROOT:
		throw runtime_error("Type should never used by user");
		break;
	case Json::node::type_list::DICTIONARY:
	{
		obj_map* p_map = get<obj_map*>(*p);
		obj_map::iterator iter = p_map->find(obj_var(t));
		if (iter != p_map->end())
		{
			return iter->second.get()->get_value_reference<Tr, const Args...>(args...);
		}
		else
		{
			throw invalid_argument("Can't find the given key in map");
		}
	}
	break;
	case Json::node::type_list::VECTOR:
	{
		obj_vec* p_vec = get<obj_vec*>(*p);
		return	(*p_vec)[t].get()->get_value_reference<Tr, const Args...>(args...);
	}
	break;
	case Json::node::type_list::LEAF_INT:
	case Json::node::type_list::LEAF_DOUBLE:
	case Json::node::type_list::LEAF_STRING:
	case Json::node::type_list::LEAF_BOOL:
		throw invalid_argument("Can't index in a leaf node");
		break;
	default:
		throw runtime_error("Undefined type");
		break;
	}
}