#include "Fly_web.h"
#include "Fly_string.h" 
#include "FlyDefine.h" 


#pragma warning(disable:4996) 

#include <WinSock2.h>
#pragma comment(lib, "ws2_32") 
#include <map>
 

namespace Fly_web {
	namespace TCPs {
		namespace server {
			struct  S_TCPs
			{
				std::string PeerAddress;
				int  PeerPort; 
			}; 
			std::map<DWORD, S_TCPs>L_map;
			std::map<DWORD, SOCKET>L_mapSocket;
			//创建服务套接字
			bool Init(int port)
			{
				Fly_web::RAW::IniSocket();
				DWORD id = GetCurrentThreadId(); 
				SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (sockfd == INVALID_SOCKET)
					return false;
				//set type of SO_REUSEADDR
				int nOptValue = 1;
				if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&nOptValue, sizeof(nOptValue)) == SOCKET_ERROR)
				{
					closesocket(sockfd);
					return false;
				}
				char Server_name[128];
				struct hostent *hp;
				struct sockaddr_in addrServer;
				gethostname(Server_name, 128);
				hp = gethostbyname(Server_name);           //获取本机名   

				memset(&addrServer, 0, sizeof(addrServer)); //初始化  
				memcpy(&(addrServer.sin_addr), hp->h_addr, hp->h_length); //设置服务器的IP地址  
				addrServer.sin_family = AF_INET;            //设置版本  
				addrServer.sin_port = htons((unsigned short)port); //设置端口号 
																   //bind....绑定ip地址和端口号  
				if (SOCKET_ERROR == bind(sockfd, (LPSOCKADDR)&addrServer, sizeof(addrServer)))
				{
					closesocket(sockfd);
					return false;
				}
				if (listen(sockfd, SOMAXCONN) == SOCKET_ERROR)
				{
					closesocket(sockfd);
					return false;
				}
				L_mapSocket[id] = sockfd;
				return true;
			}

			//等待客户端
			DWORD WaitClientCome()
			{
				SOCKADDR_IN sockAddr = { 0 };
				int nAddrLen = sizeof(SOCKADDR);
				DWORD id = GetCurrentThreadId();
				SOCKET socket = accept(L_mapSocket[id], (SOCKADDR*)&sockAddr, &nAddrLen);
				if (socket != INVALID_SOCKET)
				{ 
					S_TCPs rstClientInfo;
					rstClientInfo.PeerAddress = inet_ntoa(sockAddr.sin_addr); //ip地址
					rstClientInfo.PeerPort = ntohs(sockAddr.sin_port);  //保存外部端口信息 
					L_map[socket] = rstClientInfo;
					return socket;
				}
				return 0;
			}

			//等待客户端数据
			std::string WaitClientInfo(const int clientID, char* rstBuff, int rstBuffLen)
			{
				WSANETWORKEVENTS NetworkEvents;
				HANDLE	socketEvent = WSACreateEvent();
				WSAEventSelect(clientID, socketEvent, FD_READ | FD_CLOSE);  //监听读取事件和关闭事件
				WSAWaitForMultipleEvents(1, &socketEvent, FALSE, WSA_INFINITE, FALSE);

				WSAEnumNetworkEvents(clientID, socketEvent, &NetworkEvents);
				WSAResetEvent(socketEvent);
				if (NetworkEvents.lNetworkEvents & FD_READ)
				{ 
					return  RAW::GetInfo(clientID);
				}
				else
					return "";
			}
			//发送数据
			bool SendInfo(const int clientID, const char* msg, int lenth)
			{ 
				return send(clientID, msg, lenth, 0) != SOCKET_ERROR;
			}

		}
		//客户端 TCP
		namespace client
		{ 

			std::map<DWORD, SOCKET>L_mapSocket;
			//创建客户端套接字
			bool Init(int port, char* IP)
			{
				Fly_web::RAW::IniSocket();
				auto ipPort = RAW::GetIpPortfromURL(IP, port);
				if (ipPort.ip.empty())
					return false;
				SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (sockfd == INVALID_SOCKET)
					return false;
				//set type of SO_REUSEADDR
				int nOptValue = 1;
				if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&nOptValue, sizeof(nOptValue)) == SOCKET_ERROR)
				{
					closesocket(sockfd);
					return false;
				}
				struct sockaddr_in addrServer;
				addrServer.sin_addr.S_un.S_addr = inet_addr(ipPort.ip.c_str()); //192.168.1.12
				addrServer.sin_family = AF_INET;            //设置版本  
				addrServer.sin_port = htons((unsigned short)port); //设置端口号 
																   //向服务器发出连接请求  
				if (connect(sockfd, (struct sockaddr*)&addrServer, sizeof(addrServer)) == INVALID_SOCKET) {
					return false;
				}
				DWORD id = GetCurrentThreadId();
				L_mapSocket[id] = sockfd;
				return true;
			}
			//接收数据
			std::string WaitServerMsg()
			{
				DWORD id = GetCurrentThreadId(); 
				if (!L_mapSocket[id])
					return "";
				WSANETWORKEVENTS NetworkEvents;
				HANDLE	socketEvent = WSACreateEvent();
				WSAEventSelect(L_mapSocket[id], socketEvent, FD_READ | FD_CLOSE);  //监听读取事件和关闭事件
				WSAWaitForMultipleEvents(1, &socketEvent, FALSE, WSA_INFINITE, FALSE);

				WSAEnumNetworkEvents(L_mapSocket[id], socketEvent, &NetworkEvents);
				WSAResetEvent(socketEvent);
				if (NetworkEvents.lNetworkEvents & FD_READ)
				{
					return RAW::GetInfo(L_mapSocket[id]);
				}
				else
					return false;
			}
			//发送数据
			bool SendInfo(const int parentID, const char* msg, int lenth)
			{ 
				if (!L_mapSocket[parentID])
					return "";
				return send(L_mapSocket[parentID], msg, lenth, 0) != SOCKET_ERROR;
			}
		}
	}
} 