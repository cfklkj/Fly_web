#include "Fly_web.h"
#include "Fly_string.h" 
#include "FlyDefine.h" 


#pragma warning(disable:4996) 

#include <WinSock2.h>
#include <Mstcpip.h> 
#include <WS2TCPIP.H>
#pragma comment(lib, "ws2_32") 
#include <map>
 


namespace Fly_web {
	namespace Test {
		struct IP_HEADER
		{
			char cVer : 4;//版本号  
			char cHeadLen : 4;//IP包头长度  
			char cTOS;//服务类型  
			unsigned short usLen;//IP包总长  
			unsigned short usIdent;//标识符  
			unsigned short usFlagOffset;//标记&片偏移  
			char cTTL;//生存时间  
			char cProtocol;//协议  
			unsigned short usChkSum;//校验和  
			unsigned int uiSrcIP;//源地址  
			unsigned int uiDstIP;//目标地址  
								 //数据类型的字节数对应IP头部各个内容所需的字节数  
		};

		/*
		协议类型：
		1  ICMP
		2  IGMP
		6  TCP
		17 UDP
		88 IGRP
		89 OSPF
		*/

		std::string sourceIP = "192.168.69.148";
#include <netfw.h>

		BOOL PortManagement(NET_FW_IP_PROTOCOL  protocol, LONG port, bool isopen)
		{
			CoInitialize(NULL);
			INetFwOpenPorts *pIFirewallOpenPort = NULL;

			CoCreateInstance(__uuidof(NetFwOpenPort), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwOpenPort), (LPVOID*)&pIFirewallOpenPort);


			if (!isopen)
			{
				if (pIFirewallOpenPort->Remove(port, protocol) != S_OK)
				{
					printf("关闭端口失败！");
				}
			}

			CoUninitialize();//释放COM
			return true;
		}

		bool protMgr(NET_FW_IP_PROTOCOL  protocol, LONG port)
		{
			//1. 获得防火墙接口 INetFwProfile
			INetFwProfile * m_pIFirewall = NULL;
			INetFwMgr * pIFirewallMgr = NULL;
			INetFwPolicy * pIFirewallPolicy = NULL;

			CoInitialize(NULL);
			CoCreateInstance(__uuidof(NetFwMgr), NULL, CLSCTX_INPROC_SERVER,
				__uuidof(INetFwMgr), (void**)&pIFirewallMgr);

			pIFirewallMgr->get_LocalPolicy(&pIFirewallPolicy);

			pIFirewallPolicy->get_CurrentProfile(&m_pIFirewall);

			//7.判断一个端口是否被阻止
			INetFwOpenPorts* pIFirewallOpenPorts = NULL;
			INetFwOpenPort * pIFirewallOpenPort = NULL;

		//	m_pIFirewall->get_GloballyOpenPorts(&pIFirewallOpenPorts);

		//	pIFirewallOpenPorts->Item(port, protocol, &pIFirewallOpenPort);
		//	VARIANT_BOOL varbEnabled;
		//	pIFirewallOpenPort->get_Enabled(&varbEnabled);

			/*//8. 解除一个端口的阻止
			INetFwOpenPorts* pIFirewallOpenPorts = NULL;
			INetFwOpenPort * pIFirewallOpenPort = NULL;

			m_pIFirewall->get_GloballyOpenPorts(&pIFirewallOpenPorts);

			CoCreateInstance(__uuidof(NetFwOpenPort), NULL, CLSCTX_INPROC_SERVER,
				__uuidof(INetFwOpenPort), (void**)&pIFirewallOpenPort);

			pIFirewallOpenPort->put_Port(port);
			pIFirewallOpenPort->put_Protocol(protocol);
			pIFirewallOpenPort->put_Name(bstrShowName);

			pIFirewallOpenPorts->Add(pIFirewallOpenPort);
			*/
			//阻止端口 

			m_pIFirewall->get_GloballyOpenPorts(&pIFirewallOpenPorts);
			 

			HRESULT HR = pIFirewallOpenPorts->Remove(port, protocol);
			return HR == S_OK;
		}
		void PrintData(char szBuf[], int iLen)
		{
			protMgr(NET_FW_IP_PROTOCOL_TCP, 80);
			//printf("%s\n",szBuf);  
			//捕获的数据一定不是字符串   
			IP_HEADER *pHeader = NULL;
			SOCKADDR_IN stAddr = { 0 };
			pHeader = (IP_HEADER*)szBuf;
			stAddr.sin_addr.s_addr = pHeader->uiSrcIP;
			if (inet_ntoa(stAddr.sin_addr) != sourceIP)
			{
				 printf("%s:\n", inet_ntoa(stAddr.sin_addr));
				 return;
			} 
			printf("\nRCV:\n");
			printf("Version:%d\n", pHeader->cVer);
			printf("IP Len:%d\n", pHeader->cHeadLen);
			printf("Type Of Service:%d\n", pHeader->cTOS);
			printf("Total Length:%d\n", pHeader->usLen);
			printf("Identification:%d\n", pHeader->usIdent);
			printf("TTL:%d\n", pHeader->cTTL);
			printf("Protocol:%d\n", pHeader->cProtocol);

			//stAddr.sin_addr.s_addr存的是网络字节序的int ip地址  
			//inet_ntoa把in_addr转成字符串的ip地址  
			//返回点分十进制的字符串在静态内存中的指针  
			printf("Source IP:%s\n", inet_ntoa(stAddr.sin_addr));

			stAddr.sin_addr.s_addr = pHeader->uiDstIP;
			printf("Dest IP:%s\n", inet_ntoa(stAddr.sin_addr));

			puts("");
			return;

		}
		void Init()
		{
			if (!Fly_web::RAW::IniSocket())
				return;
			SOCKET ServerSock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
			printf("socket error --%d\n", GetLastError());
			char mname[128];
			struct hostent* pHostent;
			sockaddr_in myaddr;
			//Get the hostname of the local machine
			if (-1 == gethostname(mname, sizeof(mname)))
			{
				closesocket(ServerSock);
				printf("%d", WSAGetLastError());
				exit(-1);
			}
			else
			{

			//	BOOL blnFlag = TRUE;
			//	setsockopt(ServerSock, IPPROTO_IP, IP_HDRINCL, (char *)&blnFlag, sizeof(blnFlag));

				//Get the IP adress according the hostname and save it in pHostent  
				pHostent = gethostbyname((char*)mname);
				//填充sockaddr_in结构
				myaddr.sin_addr.S_un.S_addr = inet_addr(sourceIP.c_str());// *(in_addr *)pHostent->h_addr_list[0];  //注意多网卡 ip地址
				myaddr.sin_family = AF_INET;
				myaddr.sin_port = htons(8888);//对于IP层可随意填
											  //bind函数创建的套接字句柄绑定到本地地址
				if (SOCKET_ERROR == bind(ServerSock, (struct sockaddr *)&myaddr, sizeof(myaddr)))
				{
					printf("bind error --%d\n", GetLastError());
					closesocket(ServerSock); 
					return;
				}
			}
			//设置该SOCKET为接收所有流经绑定的IP的网卡的所有数据，包括接收和发送的数据包
			u_long sioarg = 1;
			DWORD dwValue = 0;
			if (SOCKET_ERROR == WSAIoctl(ServerSock, SIO_RCVALL, &sioarg, sizeof(sioarg), NULL, 0, &dwValue, NULL, NULL))
			{
				closesocket(ServerSock); 
				exit(-1);
			}
			//获取分析数据报文
			char buf[65535];
			int len = 0;
			listen(ServerSock, 5);
			do
			{
				len = recv(ServerSock, buf, sizeof(buf) -1, 0);
				if (len > 0)
				{
					buf[len] = 0;
					PrintData(buf,len);
					//报文处理
				}
			} while (len > 0);

		}
	}
} 