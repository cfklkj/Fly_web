#include "Fly_web.h"
#include "Fly_string.h"
#include "Fly_debug.h"
#include "Fly_file.h"

#include <Windows.h>
#include <wininet.h>
#pragma   comment   (lib,   "wininet.lib")
#pragma comment(lib, "Mpr")
#pragma warning(disable:4996)

#include <sstream> 
#include <iostream>
  
namespace Fly_web
{ 
	namespace HTMLs {
		const int MAXHTMLsBUFF = 1024;  
		//获取网址内容
		std::string GetURLInfo(const char* URL)
		{ 
			bool isGet = false;
			std::string rstStr = "";
			HINTERNET hSession = InternetOpen("RookIE/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
			if (hSession != NULL)
			{ 
				HINTERNET handle2 = InternetOpenUrl(hSession, URL, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
				if (handle2 != NULL)
				{
					char Temp[MAXHTMLsBUFF + 1];
					ULONG Number = 1;
					bool isgb2312 = false;
					while (Number > 0)
					{
						isGet = true;
						InternetReadFile(handle2, Temp, MAXHTMLsBUFF, &Number);
						if (!Number) //add -- 2017-11-17
						{
							break;
						}
						Temp[Number] = 0;	
						if (isgb2312 || Fly_string::FindSub(Temp, "charset=gb2312"))
						{
							isgb2312 = true; 
							rstStr += Temp; 
						}else
							rstStr += Fly_string::Utf8ToGBK(Temp);
					}
					InternetCloseHandle(handle2);
				}
				else
				{
					Fly_debug::Print("GetURLInfo error:%d\n", GetLastError());
				}
				InternetCloseHandle(hSession);
				hSession = NULL;
			}
			return rstStr;
		} 
		//php通信--不保存文件--可保存到保存buff 
		//fun_type = "POST" : "GET"
		//accept_type = "html/text"
		//connection_type =  "Close"
		std::string URLRequest(const char * URL, const char * body, const char * fun_type, const char * accept_type, const char * connection_type)
		{			 
			RAW::IniSocket();
			char mac[64] = { 0 }; 
			 
			auto IPPort = RAW::GetIpPortfromURL(URL, 80); //html  默认端口80
			std::string IP = IPPort.ip;
			int port = IPPort.port;  
			//connect
			struct sockaddr_in serv_socket;
			memset(&serv_socket, 0, sizeof(struct sockaddr_in));
			serv_socket.sin_family = AF_INET;
			serv_socket.sin_port = htons(port);
			serv_socket.sin_addr.s_addr = inet_addr(IP.c_str()); //　设置IP地址  
			SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			int	flag = connect(sockfd, (struct sockaddr *)&serv_socket, sizeof(serv_socket)); //建立和HTTP服务器的TCP链接 
			if (flag < 0) {
				Fly_debug::Print("connect error!!! ip:%s\tport:%d\tflag = %d\n", IP.c_str(), port, flag);
				closesocket(sockfd);
				RAW::IniSocket();
				return "";
			}
			Fly_debug::Print("connect OK %s\n", IP.c_str());  
			int bodyLen = body ? strlen(body) : 1;
			char * request = (char *)malloc(MAXHTMLsBUFF + bodyLen);
			const char * httpType = "HTTP";
			if (Fly_string::FindSub(URL, "https:"))
				httpType = "HTTPS"; 

			if (body)
				sprintf(request, "%s %s %s/1.1\r\nAccept: %s\r\nHost: %s:%d\r\nConnection: %s\r\nContent-Type: application/x-www-form-URLencoded\r\nContent-Length: %d\r\n\r\n%s",
					fun_type, URL, httpType, accept_type, IP.c_str(), port, connection_type, strlen(body), body);
			else
				sprintf(request, "%s %s %s/1.1\r\nAccept: %s\r\nHost: %s:%d\r\nConnection: %s\r\n\r\n",
					fun_type, URL, httpType, accept_type, IP.c_str(), port, connection_type);//以上是在组织请求的头部，打印出的结果就是文章开头所写

			send(sockfd, request, strlen(request), 0);//write  
			Fly_debug::Print("request---%s\r\n", request); 
			free(request); 

			return RAW::GetInfo(sockfd);  
		}
		std::string URLRequestV2(const char * URL, const char * requestHead, const char * body)
		{
			RAW::IniSocket();
			char mac[64] = { 0 }; 
			 
			auto IPPort = RAW::GetIpPortfromURL(URL, 80); //html  默认端口80 
			std::string IP = IPPort.ip;
			int port =IPPort.port;  
			//connect
			struct sockaddr_in serv_socket;
			memset(&serv_socket, 0, sizeof(struct sockaddr_in));
			serv_socket.sin_family = AF_INET;
			serv_socket.sin_port = htons(port);
			serv_socket.sin_addr.s_addr = inet_addr(IP.c_str()); //　设置IP地址  
			SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			int	flag = connect(sockfd, (struct sockaddr *)&serv_socket, sizeof(serv_socket)); //建立和HTTP服务器的TCP链接 
			if (flag < 0) {
				Fly_debug::Print("connect error!!! ip:%s\tport:%d\tflag = %d\n", IP.c_str(), port, flag);
				closesocket(sockfd);
				RAW::IniSocket();
				return "";
			}
			Fly_debug::Print("connect OK %s\n", IP.c_str());   
			std::string req = Fly_string::format("%s %s",URL, requestHead, body);
			send(sockfd, req.c_str(), req.length(), 0);//write  
			Fly_debug::Print("request---%s\r\n", req); 

			return RAW::GetInfo(sockfd);  
		} 

		std::string PostInfo(std::string URLs, const char* sendData, int datalength)
		{
			RAW::IniSocket();
			char mac[64] = { 0 };

			auto IPPort = RAW::GetIpPortfromURL(URLs.c_str(), 80); //html  默认端口80
			std::string IP = IPPort.ip;
			int port = IPPort.port;
			//connect
			struct sockaddr_in serv_socket;
			memset(&serv_socket, 0, sizeof(struct sockaddr_in));
			serv_socket.sin_family = AF_INET;
			serv_socket.sin_port = htons(port);
			serv_socket.sin_addr.s_addr = inet_addr(IP.c_str()); //　设置IP地址  
			SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			int	flag = connect(sockfd, (struct sockaddr *)&serv_socket, sizeof(serv_socket)); //建立和HTTP服务器的TCP链接 
			if (flag < 0) {
				Fly_debug::Print("connect error!!! ip:%s\tport:%d\tflag = %d\n", IP.c_str(), port, flag);
				closesocket(sockfd);
				RAW::IniSocket();
				return "";
			}
			Fly_debug::Print("connect OK %s\n", IP.c_str()); 
			send(sockfd, sendData, datalength, 0);//write  
			Fly_debug::Print("request---%s\r\n", sendData);

			return RAW::GetInfo(sockfd);
		} 
	}
}