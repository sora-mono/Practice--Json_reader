#include "Json.h"

extern const int MAX_STR_LENGTH;	//可容纳最大字符串长度，定义在Json.h中

using namespace std;

template<class T>
inline Json::node::node(type_list type_, T p_) :type(type_), p(new pointer(p_)) {}

Json::node::~node()
{
	delete p;
}

Json::node::node(Json::node&& n)noexcept
{
	p = n.p;
	n.p = nullptr;
	type = n.type;
	n.type = type_list::EMPTY;
}

Json::Json()
{
	root = nullptr;
}

Json::Json(const string& str)
{
	loads(str);
}

Json::Json(Json&& j)noexcept
{
	root = j.root;
	j.root = nullptr;
}

Json& Json::loads(const string& str)	//加载字符串
{
	size_t index = 0;
	node* now = new node(node::type_list::ROOT, false);
	node::obj_var key_now = std::monostate();
	stack<pair<node::obj_var, node*>>st;
	while (index < str.size())
	{
		switch (str[index])
		{
		case'{':
			st.emplace(pair<node::obj_var, node*>(key_now, now));
			key_now = std::monostate();
			now = new node(node::type_list::DICTIONARY, new node::obj_map);
			break;
		case'[':
			st.emplace(pair<node::obj_var, node*>(key_now, now));
			key_now = std::monostate();
			now = new node(node::type_list::VECTOR, new node::obj_vec);
			break;
		case'}':
		case']':
			switch (st.top().second->type)
			{
			case node::type_list::DICTIONARY:
				if ((now->type == node::type_list::DICTIONARY && str[index] == '}')
					|| (now->type == node::type_list::VECTOR && str[index] == ']'))
				{
					get<node::obj_map*>(*st.top().second->p)->emplace(move(st.top().first), now);
					now = st.top().second;
					st.pop();
				}
				else
				{
					throw runtime_error("Can't find a pair of {} or []");
				}
				break;
			case node::type_list::VECTOR:
				if ((now->type == node::type_list::DICTIONARY && str[index] == '}')
					|| (now->type == node::type_list::VECTOR && str[index] == ']'))
				{
					get<node::obj_vec*>(*st.top().second->p)->emplace_back(now);
					now = st.top().second;
					st.pop();
				}
				else
				{
					throw runtime_error("Can't find a pair of {} or []");
				}
				break;
			case node::type_list::ROOT:
				delete st.top().second;
				st.pop();
				root.reset(now);
				index = str.size();
				break;
			default:
				throw runtime_error("Can't put a vector in a node which is not a container");
				break;
			}
			break;
		case'\'':
		case'"':
		{
			size_t index_temp = 0;
			if (str[index] == '\'')
			{
				index_temp = str.find('\'', index + 1);
			}
			else if (str[index] == '\"')
			{
				index_temp = str.find('\"', index + 1);
			}
			if (index_temp == string::npos)
			{
				throw runtime_error("Can't find a pair of \"\"or\'\'");
			}
			string str_temp = str.substr(index + 1, index_temp - index - 1);
			if (get_if<std::monostate>(&key_now) != nullptr)
			{
				if (now->type == node::type_list::VECTOR)
				{
					get<node::obj_vec*>(*now->p)->emplace_back(new node(node::type_list::LEAF_STRING, new string(str_temp)));
				}
				else
				{
					key_now = string(move(str_temp));
				}
			}
			else
			{
				if (now->type == node::type_list::DICTIONARY)
				{
					get<node::obj_map*>(*now->p)->emplace(move(key_now), new node(node::type_list::LEAF_STRING, new string(str_temp)));
					key_now = std::monostate();
				}
				else
				{
					throw runtime_error("Key:value in a container which is not a dictionary");
				}
			}
			index = index_temp;
			break;
		}
		case':':
			if (now->type != node::type_list::DICTIONARY)
			{
				throw runtime_error("Can't use key:value in a container which is not a dictionary");
			}
			break;
		case'f':
			if (str.substr(index, 5) != "false")
			{
				throw runtime_error("Invalid format!");
			}
			if (now->type == node::type_list::DICTIONARY)
			{
				get<node::obj_map*>(*now->p)->emplace(move(key_now), new node(node::type_list::LEAF_BOOL, false));
				key_now = std::monostate();
			}
			else if (now->type == node::type_list::VECTOR)
			{
				get<node::obj_vec*>(*now->p)->emplace_back(new node(node::type_list::LEAF_BOOL, false));
			}
			else
			{
				throw runtime_error("Can't save false value in a leaf");
			}
			index += 5;
			break;
		case't':
			if (str.substr(index, 4) != "true")
			{
				throw runtime_error("Invalid format!");
			}
			if (now->type == node::type_list::DICTIONARY)
			{
				get<node::obj_map*>(*now->p)->emplace(move(key_now), new node(node::type_list::LEAF_BOOL, true));
				key_now = std::monostate();
			}
			else if (now->type == node::type_list::VECTOR)
			{
				get<node::obj_vec*>(*now->p)->emplace_back(new node(node::type_list::LEAF_BOOL, true));
			}
			else
			{
				throw runtime_error("Can't save true value in a leaf");
			}
			break;
		case' ':
		case'\t':
		case'\r':
		case'\n':
		case',':
			break;
		default:
			if (isdigit(str[index]) || str[index] == '+' || str[index] == '-')
			{
				size_t index_start = index++;
				bool is_double = false;
				while (isdigit(str[index]) || str[index] == '.')
				{
					is_double |= str[index] == '.';
					++index;
				}
				--index;
				if (is_double)
				{
					double temp;
					sscanf_s(&str.c_str()[index_start], "%lf", &temp);
					if (get_if<std::monostate>(&key_now) == nullptr)
					{
						if (now->type == node::type_list::DICTIONARY)
						{
							get<node::obj_map*>(*now->p)->emplace(move(key_now), new node(node::type_list::LEAF_DOUBLE, temp));
							key_now = std::monostate();
						}
						else
						{
							throw runtime_error("Can't use key:value in a container which is not a dictionary");
						}
					}
					else
					{
						if (now->type == node::type_list::VECTOR)
						{
							get<node::obj_vec*>(*now->p)->emplace_back(new node(node::type_list::LEAF_DOUBLE, temp));
						}
						else
						{
							key_now = temp;
						}
					}
				}
				else
				{
					int temp;
					sscanf_s(&str.c_str()[index_start], "%d", &temp);
					if (get_if<std::monostate>(&key_now) == nullptr)
					{
						if (now->type == node::type_list::DICTIONARY)
						{
							get<node::obj_map*>(*now->p)->emplace(move(key_now), new node(node::type_list::LEAF_INT, temp));
							key_now = std::monostate();
						}
						else
						{
							throw runtime_error("Can't use key:value in a container which is not a dictionary");
						}
					}
					else
					{
						if (now->type == node::type_list::VECTOR)
						{
							get<node::obj_vec*>(*now->p)->emplace_back(new node(node::type_list::LEAF_INT, temp));
						}
						else
						{
							key_now = temp;
						}
					}
				}
			}
			break;
		}
		++index;
	}
	return *this;
}

void Json::node::show(ostream& s, const obj_var& x)const	//根据key的类型显示不同的数据，index为硬编码
{
	switch (x.index())
	{
	case 1:
		cout << get<int>(x);
		break;
	case 2:
		cout << get<double>(x);
		break;
	case 3:
		cout << '\"' << get<string>(x) << '\"';
		break;
	default:
		break;
	}
}

void Json::node::show(ostream& out, size_t level)const
{
	auto get_blanks = [](string& sblanks, size_t level)
	{
		for (size_t i = 0; i < level; i++)
		{
			sblanks += "  ";
		}
	};
	switch (type)
	{
	case node::type_list::DICTIONARY:
	{
		string sblanks;
		get_blanks(sblanks, level);
		out << '{' << endl;
		size_t iindex = 0;
		for (auto& x : *get<obj_map*>(*p))
		{
			out << sblanks << "  ";
			show(out, x.first);
			out << ": ";
			x.second.get()->show(out, level + 1);
			if (++iindex < get<obj_map*>(*p)->size())
			{
				out << ',';
			}
			out << endl;
		}
		out << sblanks << "}";
	}
	break;
	case node::type_list::VECTOR:
	{
		string sblanks;
		get_blanks(sblanks, level);
		out << '[' << endl;
		size_t iindex = 0;
		for (auto& x : *get<obj_vec*>(*p))
		{
			out << sblanks << "  ";
			x.get()->show(out, level + 1);
			if (++iindex < get<obj_vec*>(*p)->size())
			{
				out << ',';
			}
			out << endl;
		}
		out << sblanks << ']';
	}
	break;
	case node::type_list::LEAF_INT:
		out << get<int>(*p);
		break;
	case node::type_list::LEAF_DOUBLE:
		out << get<double>(*p);
		break;
	case node::type_list::LEAF_STRING:
		out << '\"' << *get<string*>(*p) << '\"';
		break;
	case node::type_list::LEAF_BOOL:
		out << (get<bool>(*p) == false ? "false" : "true");
		break;
	default:
		break;
	}
}

void Json::output(ostream& out)const
{
	root->show(out, 0);
}