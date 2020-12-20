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

template<class T>
concept argument = requires(T t)
{
	is_same<T, int>::value || is_same<T, double>::value || is_same<T, string>::value;
};
class Json
{
private:
	struct node
	{
		using pointer = variant
			<
			vector<node>*,
			map<variant<std::monostate, int, double, string>, node>*,
			string*,
			int,
			double,
			bool
			>;
		using obj_vec = vector<node>;
		using obj_map = map<variant<std::monostate, int, double, string>, node>;
		using obj_str = string;
		using obj_var = variant<std::monostate, int, double, string>;
		using num_int = int;
		using num_double = double;
		enum class type_list :char { EMPTY, ROOT, DICTIONARY, VECTOR, LEAF_INT, LEAF_DOUBLE, LEAF_STRING, LEAF_BOOL };

		type_list type;
		pointer* p;

		inline node() :type(node::type_list::EMPTY), p(nullptr) {}
		template<class T>
		inline node(type_list type_, T p_);
		inline node(node::type_list type_) :type(type_), p(nullptr) {}
		node(node&& n)noexcept;
		node(const node& n) = delete;
		~node();
		void show(ostream& out, const node& n, size_t level)const;
		void show(ostream& s, const obj_var& x)const;
		template<argument T>
		auto& operator[](const T &arg)const;
	};
	node* root;
public:
	Json();
	Json(const string& str);
	Json(const Json& j);
	Json(Json&& j) noexcept;
	~Json();
	Json& loads(const string& str);
	void show(ostream& out)const;
	template<argument T>
	inline auto& operator[](const T& arg)const { return root->operator[](arg); };
};