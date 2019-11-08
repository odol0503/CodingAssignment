#ifndef COMMON_H
#define COMMON_H

#include <strsafe.h>
#include <string>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

#define PIPE_NAME "\\\\.\\pipe\\pipe_test"
#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096

#define INTEGER_TYPE 1
#define STRING_TYPE 2
#define STRUCT_TYPE 3
#define CALLFUNC_TYPE 4

#define CUSTOM_OBJ "custom_obj"

#define SUM_FUNC "sum_func"
#define REVERSE_FUNC "reverse_func"

struct person {
	std::string name;
	std::string address;
	int age;
};

void to_json(json& j, const person& p) {
	j = json{ { "name", p.name },{ "address", p.address },{ "age", p.age } };
}

void from_json(const json& j, person& p) {
	j.at("name").get_to(p.name);
	j.at("address").get_to(p.address);
	j.at("age").get_to(p.age);
}

string serialize(json obj)
{
	return obj.dump();
}

json deserialize(string str)
{
	return json::parse(str);
}



typedef void(*funcCB)(string &args, string &ret_val);

class CustFunc {
private:
	string m_name;
	funcCB m_funcCB;

public:
	CustFunc(string name, funcCB pFunc)
	{
		m_name = name;
		m_funcCB = pFunc;
	}

	string get_name() { return m_name; }

	void call(string &args, string &ret_val)
	{
		if (m_funcCB)
		{
			return m_funcCB(args, ret_val);
		}
	}
};

class CustObj {
private:
	string m_name;
	unordered_map<string, shared_ptr<CustFunc>> m_func;

public:
	CustObj(const string &name) : m_name(name) {}
	
	string get_name() { return m_name; }
	
	int register_func(shared_ptr<CustFunc> func)
	{
		string func_name = func->get_name();
		if (m_func.count(func_name)) return -1;

		m_func[func_name] = func;

		return 0;
	}

	shared_ptr<CustFunc> get_func(const string &name)
	{
		if (m_func.count(name) == 0) return nullptr;
		return m_func[name];
	}
};

#endif