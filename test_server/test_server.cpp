#include "server.hpp"

static void sum_func(string &args, string &ret_val)
{
	cout << "sum_func called!" << endl;
	json j = deserialize(args);
	vector<int> val = j.at("val");
	json j2;
	j2["ret_val"] = accumulate(val.begin(), val.end(), 0);
	ret_val = serialize(j2);
}

static void reverse_func(string &args, string &ret_val)
{
	cout << "reverse_func called!" << endl;
	json j = deserialize(args);
	string val = j.at("val");
	json j2;
	reverse(val.begin(), val.end());
	j2["ret_val"] = val;
	ret_val = serialize(j2);
}

int main(int argc, char* argv[])
{
	Server sv;

	shared_ptr<CustObj> obj = make_shared<CustObj>(CUSTOM_OBJ);
	obj->register_func(make_shared<CustFunc>(SUM_FUNC, sum_func));
	obj->register_func(make_shared<CustFunc>(REVERSE_FUNC, reverse_func));
	sv.register_obj(obj);

	sv.init();
	sv.run();

	return 0;
}