#pragma once
#include <string> 
#include <vector>

namespace Fly_web {
	namespace RAW {
		//��ʼ������
		bool IniSocket();
		//�ͷŻ���
		void UnInitSocket();
		//�������� 
		std::string GetInfo(const int socket);
		//�Ƿ�����
		bool isLinkInternet(); 
		//����ַ��ȡIP Port �������:9080
		struct IPPORT {
			std::string ip;
			int port;
			std::string getIpPort();  //192.168.1.12:8090
		};
		IPPORT GetIpPortfromURL(const char* URL, int defaultPort);

	} 
	namespace system { 
		//����ϵͳʱ��
		//[ʱ��],[NTPIP|URL]   ʱ���ʽ 2018-4-24 0:00:08
		bool setTime(std::string& strCmd);
	}
	namespace Test {
		void Init();
	}

	namespace TCPs {
		namespace server {
			//���������׽���
			bool Init(int port);
			//�ȴ��ͻ���
			//�ȴ��ͻ���
			unsigned long WaitClientCome();
			//�ȴ��ͻ�������
			std::string WaitClientInfo(const int clientID, char* rstBuff, int rstBuffLen);
			//��������
			bool SendInfo(const int clientID, const char* msg, int lenth);
		}
		namespace client {
			//�����ͻ����׽���
			bool Init(int port, char* IP);
			//��������
			std::string WaitServerMsg();
			//��������  parentID --����init�߳�ID
			bool SendInfo(const int parentID, const char* msg, int lenth);
		}
	}
	namespace SETs {
		//�Ƿ�������IP
		// LAN IP's
		// -------------------------------------------
		//	0.*								"This" Network
		//	10.0.0.0 - 10.255.255.255		Class A
		//	172.16.0.0 - 172.31.255.255		Class B
		//	192.168.0.0 - 192.168.255.255	Class C
		bool IsLANIP(unsigned int nIP);
		//���û��������
		//���淽������ cmd.exe /c ipconfig /release | /renew 
		bool LinkWeb(bool isDiskconnect);
		//��ȡ������ַ
		std::string GetMac();
	}
	namespace HTMLs { 
		//��ȡ��ַ����
		std::string GetURLInfo(const char* URL);
		//��ַ����
		//body = "" �� eg:id=12&ip=1234   --""  ���ж� ��ַ�Ƿ���� ��
		std::string URLRequest(const char * URL, const char * body,const char * fun_type = "POST", const char * accept_type = "html/text", const char * connection_type = "Close");
		std::string URLRequestV2(const char* URL, const char* requestHead, const char* body); 
	}
	namespace Https {
		std::string Post(std::string URLs, std::string httpHead, std::string body);
		std::string Post(std::string URLs, std::string body);
	}
	//post �ļ�
	std::string PostFile(const char* Url, const char*  UploadName, const std::vector<std::string>& _FilePathList);

	namespace IOCPs {
		//ע��---�ͻ���ʹ�� TCPs::lient  ����
		namespace server {
			//����ʽ   ��������ֻ��ת����Ϣ---ת����Ϣ������һ���ͻ���ȥ�����������߼�
			bool createIOCPServer(int port);
		}
	}
}
 