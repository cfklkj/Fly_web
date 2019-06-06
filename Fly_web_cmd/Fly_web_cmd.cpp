// Fly_web_cmd.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "Fly_web.h"
#include "Fly_string.h"
#include "Fly_file.h"
#include <vector>



void help()
{
	const char* helps = "{-p=} {-o=} \
eg:  -p urls httpHead body         is custom httphead to post info \n\
-p urls body      is simple to post info\
-o filePath  out print to file ";
	printf(helps);
}

bool getPostValue(int argc,const char* argv[], std::vector<std::string>& rstPostValue)
{
	bool isFindPost = false;
	for (int i = 1; i < argc; i++)
	{
		if (isFindPost)
		{ 
			if (argv[i][0] == '-')
				break;
			rstPostValue.push_back(argv[i]);
		}
		else
		{ 
			isFindPost = Fly_string::FindSub(argv[i], "-p");
		}
	}
	return rstPostValue.size() > 0;
}
bool getOutFilePath(int argc,const char* argv[], std::string & filePath)
{
	bool isFindPost = false;
	for (int i = 1; i < argc; i++)
	{
		if (isFindPost)
		{
			if (argv[i][0] == '-')
				break;
			filePath = argv[i];
			break;
		}
		else
		{
			isFindPost = Fly_string::FindSub(argv[i], "-o");
		}
	}
	return filePath.length() > 3;
}


int main(int argc,const char* argv[])
{  
	std::vector<std::string> postValue;
	if (!getPostValue(argc, argv, postValue))
	{
		help();
		return -1;
	}
	std::string rst = "";
	if (postValue.size() == 1)
	{
		rst = Fly_web::Https::Post(postValue.at(0), "");
	}
	else
	if (postValue.size() == 2)
	{
		rst = Fly_web::Https::Post(postValue.at(0), postValue.at(1));
	}
	else
	if (postValue.size() == 3)
	{
		rst = Fly_web::Https::Post(argv[2], argv[3], argv[4]);
	}
	std::string outPath = "";
	if (getOutFilePath(argc, argv, outPath))
	{
		Fly_file::File::echoToFile(outPath, rst, false);
	}
	else
	{
		printf("%s", rst.c_str());
	}
	return -1;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
