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
	const char* helps = "{-p=} {-f=} {-o=} \
eg:  -p urls httpHead body         is custom httphead to post info \n\
-p urls body      is simple to post info\n\
-f urls key path path paht ...  is push file\n\
-o filePath  out print to file ";
	printf(helps);
}

bool getPostFileValue(int argc, const char* argv[], std::vector<std::string>& rstPostValue)
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
			isFindPost = Fly_string::FindSub(argv[i], "-f");
		}
	}
	return rstPostValue.size() > 2;
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
	std::string rst = "";
	if (!getPostValue(argc, argv, postValue))
	{
		if (getPostFileValue(argc, argv, postValue))
		{
			std::vector<std::string> strlist;
			for (int i = 2; i < postValue.size(); i++)
			{
				strlist.push_back(postValue[i]);;
			}
			rst = Fly_web::PostFile(postValue[0].c_str(), postValue[1].c_str(), strlist);
		}
		else
		{
			help();
			return -1;
		}
	}
	else
	{
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
