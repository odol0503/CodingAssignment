#include "client.hpp"

int main(int argc, char* argv[])
{
	Client client;
	
	if (client.init())
	{
		cout << "client init failed!!" << endl;
		client.close();
		cin.get();
		return 0;
	}
	
	cout << "test1: send integer(sync)" << endl;
	json in1, out1;
	in1["type"] = INTEGER_TYPE;
	in1["value"] = 55;
	client.sendMsgSync(in1, out1);
	cout << "intput=" << in1 << endl;
	cout << "output=" << out1 << endl;
	if (in1 == out1)
		cout << "result: passed" << endl;
	else
		cout << "result: failed" << endl;
	cout << endl;


	cout << "test2: send string(sync)" << endl;
	json in2, out2;
	in2["type"] = STRING_TYPE;
	in2["value"] = "Have a good day!";
	client.sendMsgSync(in2, out2);
	cout << "intput=" << in2 << endl;
	cout << "output=" << out2 << endl;
	if (in2 == out2)
		cout << "result: passed" << endl;
	else
		cout << "result: failed" << endl;
	cout << endl;


	cout << "test3: send structure(sync)" << endl;
	json in3, out3;
	in3["type"] = STRUCT_TYPE;
	person p3 { "Ned Flanders", "744 Evergreen Terrace", 60 };
	json js3 = p3;
	in3["value"] = serialize(js3);
	client.sendMsgSync(in3, out3);
	cout << "intput=" << in3 << endl;
	cout << "output=" << out3 << endl;
	if (in3 == out3)
		cout << "result: passed" << endl;
	else
		cout << "result: failed" << endl;
	cout << endl;


	cout << "test4: call function(sync)" << endl;
	json in4, out4;
	in4["type"] = CALLFUNC_TYPE;
	in4["class_name"] = CUSTOM_OBJ;
	in4["func_name"] = SUM_FUNC;
	json js4;
	vector<int> val4 = { 1, 2, 3, 4 };
	js4["val"] = val4;
	in4["args"] = serialize(js4);
	cout << "class_name=" << CUSTOM_OBJ << "::" << "func_name=" << SUM_FUNC << endl;
	client.sendMsgSync(in4, out4);
	cout << "intput= { ";
	for (int v : val4) cout << v << " ";
	cout << "}" << endl;
	cout << "output=" << out4.at("ret_val") << endl;
	if (out4.at("ret_val") == accumulate(val4.begin(), val4.end(), 0))
		cout << "result: passed" << endl;
	else
		cout << "result: failed" << endl;
	cout << endl;


	cout << "test5: send integer(async)" << endl;
	json in5, out5;
	in5["type"] = INTEGER_TYPE;
	in5["value"] = 72;
	client.sendMsgSync(in5, out5);
	cout << "intput=" << in5 << endl;
	cout << "output=" << out5 << endl;
	if (in5 == out5)
		cout << "result: passed" << endl;
	else
		cout << "result: failed" << endl;
	cout << endl;


	cout << "test6: send string(async)" << endl;
	json in6, out6;
	in6["type"] = STRING_TYPE;
	in6["value"] = "You too!";
	client.sendMsgSync(in6, out6);
	cout << "intput=" << in6 << endl;
	cout << "output=" << out6 << endl;
	if (in6 == out6)
		cout << "result: passed" << endl;
	else
		cout << "result: failed" << endl;
	cout << endl;


	cout << "test7: send structure(async)" << endl;
	json in7, out7;
	in7["type"] = STRUCT_TYPE;
	person p7{ "Jon Snow", "The Wall", 20 };
	json js7 = p7;
	in7["value"] = serialize(js7);
	client.sendMsgSync(in7, out7);
	cout << "intput=" << in7 << endl;
	cout << "output=" << out7 << endl;
	if (in7 == out7)
		cout << "result: passed" << endl;
	else
		cout << "result: failed" << endl;
	cout << endl;

	cout << "test8(async): call function" << endl;
	json in8, out8;
	in8["type"] = CALLFUNC_TYPE;
	in8["class_name"] = CUSTOM_OBJ;
	in8["func_name"] = REVERSE_FUNC;
	json js8;
	string val8 = "This is a test!";
	js8["val"] = val8;
	in8["args"] = serialize(js8);
	cout << "class_name=" << CUSTOM_OBJ << "::" << "func_name=" << REVERSE_FUNC << endl;
	client.sendMsgSync(in8, out8);
	cout << "intput=" << val8 << endl;
	cout << "output=" << out8.at("ret_val") << endl;
	reverse(val8.begin(), val8.end());
	if (out8.at("ret_val") == val8)
		cout << "result: passed" << endl;
	else
		cout << "result: failed" << endl;
	cout << endl;

	cin.get();
	client.close();

	return 0;
}