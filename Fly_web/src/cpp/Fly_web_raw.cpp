#include "Fly_web.h"
#include "Fly_string.h" 
#include "Fly_debug.h" 
#include "FlyDefine.h" 

#include <fstream>
#include <stdio.h> 

#include <WinSock2.h>
#pragma comment(lib, "ws2_32") 

#include <wininet.h>
#pragma   comment   (lib,   "wininet.lib")
#include <wlanapi.h>  
#pragma comment(lib, "wlanapi.lib")  
#include <map>
 
#pragma warning(disable:4996)

namespace Fly_web {
	namespace RAW { 
		std::map<DWORD, int> L_map;
		//初始化环境
		bool IniSocket()
		{
			DWORD id = GetCurrentThreadId();
			if (!L_map[id])
			{
				WSADATA wSaData;
				WORD dwSockVersion = MAKEWORD(2, 2);
				if (0 != WSAStartup(dwSockVersion, &wSaData)) //协商版本号  
				{
					return false;
				}
				L_map[id] = 1;
			}
			return true;
		}
		//释放环境
		void UnInitSocket()
		{
			DWORD id = GetCurrentThreadId();
			if (!L_map[id])
			{
				L_map[id] = 0;
				WSACleanup();
			}
		}
		//接收数据 
		std::string GetInfo(const int socket)
		{
			const int rstBuffLen = 1024;
			char readBuff[rstBuffLen];				
			std::string rstBuff;
			int readLen = 0;
			do
			{
				readLen = recv(socket, readBuff, rstBuffLen, 0);
				if (readLen < 1)
				{
					return rstBuff;
				}
				else
					rstBuff.append(readBuff, readLen);
			} while (readLen == rstBuffLen);
			return rstBuff;
		}
		//是否联网
		bool isLinkInternet()
		{
			DWORD dwd = 0;
			return InternetGetConnectedState(&dwd, dwd);   //InternetCheckConnection
		}
		//从网址提取IP Port
		IPPORT GetIpPortfromURL(const char* URL, int defaultPort)
		{
			IPPORT rst;			
			if (URL[0] >= '0' && URL[0] <= '9')
			{
				rst.ip = URL;
				return rst;
			}
			if (!isLinkInternet()) //提取网址ip需要连接网络
			{
				return rst;
			}
			std::string rstStr = "";
			std::string Port = ""; 
			std::string strTemp = Fly_string::pFindSub(URL, "://"); //开头 
			strTemp = Fly_string::GetSubStr(strTemp.c_str(), '/', 1);
			const char* infoEnd = strTemp.c_str();
			if (Fly_string::FindSub(infoEnd, ":"))  //www.baidu.com:8090
			{
				rstStr = Fly_string::GetSubStr(infoEnd, ':', 1);
				Port = Fly_string::GetSubStr(infoEnd, ':', 2);
			}
			else
				rstStr = strTemp;
			struct hostent * hp = ::gethostbyname(rstStr.c_str());//("www.kqidong.com");//2  
			Fly_debug::Print("URL:%s\tport:%s\n", rstStr.c_str(), Port.c_str());
			if (!hp)
				return rst;
			rst.ip = inet_ntoa(*((struct in_addr *)hp->h_addr_list[0]));
			if (!Port.empty()) 
			{
				rst.port = atoi(Port.c_str());
			}
			else
			{
				rst.port = defaultPort;
			}
			Fly_debug::Print("rst:%s\tport:%d\n", rst.ip.c_str(), rst.port);
			return rst;  
		}
	}
}
