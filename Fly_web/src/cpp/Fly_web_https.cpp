/*
2019-6-5
参考：  https://blog.csdn.net/ZRXSLYG/article/details/81395640
*/
#include "Fly_web.h"
#include "Fly_string.h"
 
#include <WINSOCK2.H> 


#include <openssl/ssl.h>

#include <openssl/err.h>
  
#include <iostream>

#include <sstream> 


#ifdef _WIN64
#define ROOTPATH "\\libX64" 
#else
#define ROOTPATH "lib"
#endif
#pragma comment(lib, ROOTPATH"\\libssl.lib")
#pragma comment(lib, ROOTPATH"\\openssl.lib")

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)





using namespace std;

namespace Fly_web
{
	namespace Https {

		std::string PostOrGet(const char* sendData, std::string IP, int port)
		{ 
			if (!Fly_web::RAW::isLinkInternet())
				return "link web error";

			if (!RAW::IniSocket())
				return "WSAStartup error ";
			//加载SSL错误信息 

			SSL_load_error_strings();

			//添加SSL的加密/HASH算法 

			SSLeay_add_ssl_algorithms();

			//客户端，服务端选择SSLv23_server_method() 

			const SSL_METHOD *meth = SSLv23_client_method();

			//建立新的SSL上下文 

			SSL_CTX* ctx = SSL_CTX_new(meth);

			if (ctx == NULL) 
			{ 

				return "SSL_CTX_new error !";

			}



			SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (client == INVALID_SOCKET)

			{ 
				return "socket error !";

			} 

			struct sockaddr_in serv_socket;
			memset(&serv_socket, 0, sizeof(struct sockaddr_in));
			serv_socket.sin_family = AF_INET;
			serv_socket.sin_port = htons(port);
			serv_socket.sin_addr.s_addr = inet_addr(IP.c_str()); //　设置IP地址  

			if (connect(client, (sockaddr*)&serv_socket, sizeof(serv_socket)) == SOCKET_ERROR) 
			{  
				return "connect error";

			} 

			//建立SSL 

			int ret;

			SSL*ssl = SSL_new(ctx);

			if (ssl == NULL)

			{ 
				closesocket(client);
				return  "SSL NEW error";

			}

			//将SSL与TCP SOCKET 连接 

			SSL_set_fd(ssl, client);

			//SSL连接 

			ret = SSL_connect(ssl);

			if (ret == -1) 
			{ 
				return  "SSL ACCEPT error ";

			} 

			ret = SSL_write(ssl, sendData, strlen(sendData));

			if (ret == -1)
			{
				return "SSL write error !";

			}

			char *rec = new char[1024 * 1024];

			int start = 0;

			while ((ret = SSL_read(ssl, rec + start, 1024)) > 0)

			{

				start += ret;

			}

			rec[start] = 0;

			std::string rst = rec;

			delete[] rec;


			//关闭SSL套接字 

			SSL_shutdown(ssl);

			//释放SSL套接字 

			SSL_free(ssl);

			//释放SSL会话环境 

			SSL_CTX_free(ctx);



			closesocket(client);
			Fly_web::RAW::UnInitSocket();

			return rst;
		}
		std::string Post(std::string URLs, std::string body)
		{

			auto ipInfo = RAW::GetIpPortfromURL(URLs.c_str(), 443);    //https default port 443 

			char*  bodyToUtf8 = Fly_string::GBKToUTF8_char(body.c_str());

			stringstream stream;

			stream << "POST " << URLs << " HTTP/1.0\r\n";

			stream << "Accept: */*\r\n";

			stream << "Accept-Language: zh-Hans-CN, zh-Hans; q=0.8, en-US; q=0.5, en; q=0.3\r\n";

			stream << "Host: " << ipInfo.ip.c_str() << "\r\n";

			stream << "Content-Type: application/x-www-form-URLencoded\r\n";

			stream << "Content-Type: charset=UTF-8\r\n";

			stream << "Content-Length: " << strlen(bodyToUtf8) << "\r\n";

			stream << "\r\n";

			stream << bodyToUtf8;
			 
			Fly_string::freeChar(bodyToUtf8);

			string s = stream.str();

			const char *sendData = s.c_str();

			return PostOrGet(sendData, ipInfo.ip, ipInfo.port);
		}
		std::string Post(std::string URLs, std::string httpHead, std::string body)
		{
			auto ipInfo = RAW::GetIpPortfromURL(URLs.c_str(), 443);  //https default port 443 
			std::string sendData = httpHead + "\r\n" + body;
			return PostOrGet(sendData.c_str(), ipInfo.ip, ipInfo.port);
		}

	}
}

