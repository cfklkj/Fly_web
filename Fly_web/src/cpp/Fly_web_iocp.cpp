#pragma once
#include <string>
#include <winsock2.h>   
#include <Windows.h>  
#include <vector>
#include <iostream> 
#pragma warning(disable:4996)

using namespace std;
#pragma comment(lib, "Ws2_32.lib")      // Socket编程需用的动态链接库 

#include "Fly_string.h"
#include "Fly_web.h"
#include "Fly_sys.h"
#include "Fly_debug.h"

 

namespace Fly_web {
	namespace IOCPs {
		namespace server {
			const int MessMaxLen = 1024;
			const int DataBuffSize = 4 * 1024;
			/**
			* 结构体名称：PER_IO_DATA
			* 结构体功能：重叠I/O需要用到的结构体，临时记录IO数据
			**/
			typedef struct
			{
				OVERLAPPED overlapped;
				WSABUF databuff;
				char buffer[DataBuffSize];
				int BufferLen;
				int operationType;
				SOCKET socket;
			}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA, *LPPER_IO_DATA, PER_IO_DATA;

			/**
			* 结构体名称：PER_HANDLE_DATA
			* 结构体存储：记录单个套接字的数据，包括了套接字的变量及套接字的对应的客户端的地址。
			* 结构体作用：当服务器连接上客户端时，信息存储到该结构体中，知道客户端的地址以便于回访。
			**/
			typedef struct
			{
				SOCKET socket;
				SOCKADDR_STORAGE ClientAddr;
				LPPER_IO_OPERATION_DATA PerIoData;
			}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;


			HANDLE  m_completionPort = NULL;
			SOCKET m_srvSocket = 0;
			std::vector<LPPER_HANDLE_DATA> m_vclientGroup;
			/**************************
			服务器接收客户端消息，
			工作线程
			**************************/
			DWORD WINAPI   ServerWorkThread(LPVOID CompletionPortID)
			{
				HANDLE CompletionPort = (HANDLE)CompletionPortID;
				DWORD BytesTransferred;
				LPOVERLAPPED IpOverlapped;
				LPPER_HANDLE_DATA PerHandleData = NULL;
				LPPER_IO_DATA PerIoData = NULL;
				DWORD RecvBytes;
				DWORD Flags = 0;
				BOOL bRet = false;

				while (m_completionPort) {
					bRet = GetQueuedCompletionStatus(m_completionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);
					if (bRet == 0) {
						continue;
						//这里不能返回，返回子线程就结束了
						//return -1;  
					}
					if (!PerHandleData)
						continue;
					PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);

					// 检查在套接字上是否有错误发生   
					if (0 == BytesTransferred) {
						closesocket(PerHandleData->socket);
						GlobalFree(PerHandleData);
						GlobalFree(PerIoData);
						Fly_debug::Print("客户端退出 --%d \n", PerHandleData->socket);
						continue;
					}
					//得到客户端SOCKET信息
					SOCKET sClientSocket = PerHandleData->socket;
					Fly_debug::Print("lenth:%d, message is %s \n", PerIoData->databuff.len, PerIoData->databuff.buf);

					//添加处理自定义消息					 
					send(sClientSocket, PerIoData->databuff.buf, PerIoData->databuff.len, 0);  // 发送信息   

					// 为下一个重叠调用建立单I/O操作数据   
					ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // 清空内存   
					PerIoData->databuff.len = 1024;
					PerIoData->databuff.buf = PerIoData->buffer;
					PerIoData->operationType = 0;    // read   
					WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
				}
				return  0;
			}

			DWORD getNumberOfProcess()
			{
				SYSTEM_INFO si;
				GetSystemInfo(&si);
				return si.dwNumberOfProcessors;
			}

			/**************************
			// 创建IOCP的内核对象
			**************************/
			bool  Init()
			{
				Fly_web::RAW::IniSocket();
				// 创建IOCP的内核对象   
				/**
				* 需要用到的函数的原型：
				* HANDLE WINAPI CreateIoCompletionPort(
				*    __in   HANDLE FileHandle,     // 已经打开的文件句柄或者空句柄，一般是客户端的句柄
				*    __in   HANDLE ExistingCompletionPort, // 已经存在的IOCP句柄
				*    __in   ULONG_PTR CompletionKey,   // 完成键，包含了指定I/O完成包的指定文件
				*    __in   DWORD NumberOfConcurrentThreads // 真正并发同时执行最大线程数，一般推介是CPU核心数*2
				* );
				**/
				m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
				if (NULL == m_completionPort) {    // 创建IO内核对象失败   
					Fly_debug::Print("CreateIoCompletionPort failed. Error:\n");
					return false;
				}

				// 基于处理器的核心数量创建线程   
				int threadCounts = Fly_sys::CPU::getNumberOfProcess() * 2;
				for (DWORD i = 0; i < threadCounts; ++i) {
					// 创建服务器工作器线程，并将完成端口传递到该线程   
					HANDLE ThreadHandle = CreateThread(NULL, 0, &ServerWorkThread, m_completionPort, 0, NULL);
					if (NULL == ThreadHandle) {
						Fly_debug::Print("Create Thread Handle failed. Error::\n");
					}
					else
						CloseHandle(ThreadHandle);
				}
				return true;
			}
			//销毁资源 
			bool fIni()
			{
				if (!m_completionPort)
					return true;
				for (int i = 0; i < m_vclientGroup.size(); i++)
				{
					try {
						if (!closesocket(m_vclientGroup[i]->socket))
						{
							GlobalFree(m_vclientGroup[i]->PerIoData); //释放分配的资源
							GlobalFree(m_vclientGroup[i]);
						}
					}
					catch (...)
					{
						Fly_debug::Print("资源已释放--error");
					}
				}
				CloseHandle(m_completionPort);
				closesocket(m_srvSocket);
				m_completionPort = NULL;
				m_srvSocket = 0;
				return true;
			}
			/*************************
			初始化SOCKET信息
			IP=NULL  本地IP
			*************************/
			SOCKET InitSocket(int port, const char* IP = NULL)
			{
				// 建立流式套接字   
				SOCKET srvSocket = socket(AF_INET, SOCK_STREAM, 0);

				// 绑定SOCKET到本机   
				SOCKADDR_IN srvAddr;
				if (!IP)
					srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
				else
					srvAddr.sin_addr.S_un.S_addr = inet_addr(IP);
				srvAddr.sin_family = AF_INET;
				srvAddr.sin_port = htons(port);
				int bindResult = bind(srvSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
				if (SOCKET_ERROR == bindResult) {
					return -1;
				}
				return srvSocket;
			}
			//阻塞式
			bool createIOCPServer(int port)
			{
				if (m_completionPort)
					return true;
				if (!Init())
					return false;
				m_srvSocket = InitSocket(port);
				// 将SOCKET设置为监听模式    
				if (SOCKET_ERROR == listen(m_srvSocket, 10)) {
					closesocket(m_srvSocket);
					return false;
				}
				// 开始处理IO数据   
				Fly_debug::Print("本服务器已准备就绪，正在等待客户端的接入...");
				int icount = 0;
				while (m_completionPort) {
					PER_HANDLE_DATA * PerHandleData = NULL;
					SOCKADDR_IN saRemote;
					int RemoteLen;
					SOCKET acceptSocket;

					// 接收连接，并分配完成端，这儿可以用AcceptEx()   
					RemoteLen = sizeof(saRemote);
					acceptSocket = accept(m_srvSocket, (SOCKADDR*)&saRemote, &RemoteLen);
					if (SOCKET_ERROR == acceptSocket) {   // 接收客户端失败    
						Fly_debug::Print("Accept Socket Error: %d", GetLastError());
						continue;
					}

					icount = 0;

					// 创建用来和套接字关联的单句柄数据信息结构   
					PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));  // 在堆中为这个PerHandleData申请指定大小的内存   
					PerHandleData->socket = acceptSocket;
					memcpy(&PerHandleData->ClientAddr, &saRemote, RemoteLen);

					// 将接受套接字和完成端口关联   
					CreateIoCompletionPort((HANDLE)(PerHandleData->socket), m_completionPort, (DWORD)PerHandleData, 0);


					// 开始在接受套接字上处理I/O使用重叠I/O机制   
					// 在新建的套接字上投递一个或多个异步   
					// WSARecv或WSASend请求，这些I/O请求完成后，工作者线程会为I/O请求提供服务       
					// 单I/O操作数据(I/O重叠)   
					LPPER_IO_OPERATION_DATA PerIoData = NULL;
					PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
					ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
					PerIoData->databuff.len = 1024;
					PerIoData->databuff.buf = PerIoData->buffer;
					PerIoData->operationType = 0;    // read   

					DWORD RecvBytes;
					DWORD Flags = 0;
					PerHandleData->PerIoData = PerIoData;
					m_vclientGroup.push_back(PerHandleData);      // 将单个客户端数据指针放到客户端组中 
					WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
					Fly_debug::Print("客户端进入 --%d", PerHandleData->socket);
				}
				fIni();
				Fly_debug::Print("IOCP服务器-退出");
				return true;
			}
			bool deleteIOCPServer()
			{
				fIni();
				return true;
			}
		}
	}
}