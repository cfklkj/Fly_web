#include "Fly_web.h"
#include "Fly_string.h"
#include "Fly_time.h"
#include <Windows.h>
#include <time.h>

#pragma warning(disable:4996)

//设置系统时间
//[时间],[NTPIP|URL]   时间格式 2018-4-24 0:00:08
bool Fly_web::system::setTime(std::string & strCmd)
{
	time_t		ntp_time;
	SYSTEMTIME	newtime;
	float		Splitseconds;
	struct		tm	*lpLocalTime;

	const char* tempStr = strCmd.c_str();
	std::string localTime = Fly_string::GetSubStr(tempStr, ',', 1);
	if (!Fly_Time::TIME::stringToTime(localTime, ntp_time))
	{
		lpLocalTime = localtime(&ntp_time);
		if (lpLocalTime == NULL)
		{
			return FALSE;
		}
		// 获取新的时间
		newtime.wYear = lpLocalTime->tm_year + 1900;
		newtime.wMonth = lpLocalTime->tm_mon + 1;
		newtime.wDayOfWeek = lpLocalTime->tm_wday;
		newtime.wDay = lpLocalTime->tm_mday;
		newtime.wHour = lpLocalTime->tm_hour;
		newtime.wMinute = lpLocalTime->tm_min;
		newtime.wSecond = lpLocalTime->tm_sec;

		// 设置时间精度
		Splitseconds = (float)ntohl(0);
		Splitseconds = (float)0.000000000200 * Splitseconds;
		Splitseconds = (float)1000.0 * Splitseconds;
		newtime.wMilliseconds = (unsigned   short)Splitseconds;

		// 修改本机系统时间
		SetLocalTime(&newtime);
		return true;
	}

	if (Fly_web::RAW::IniSocket())
		return false;

	std::string NTP = Fly_string::GetSubStr(tempStr, ',', 2);
	std::string IP = Fly_web::RAW::GetIpPortfromURL(NTP.c_str(), 80).ip;
	if (IP.empty())
		return false;

	struct   NTP_Packet
	{
		int      Control_Word;
		int      root_delay;
		int      root_dispersion;
		int      reference_identifier;
		__int64	reference_timestamp;
		__int64 originate_timestamp;
		__int64 receive_timestamp;
		int      transmit_timestamp_seconds;
		int      transmit_timestamp_fractions;
	};

	// 这个IP是中国大陆时间同步服务器地址，可自行修改
	SOCKET soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	struct sockaddr_in addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr(IP.c_str());
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(123);

	NTP_Packet NTP_Send, NTP_Recv;
	NTP_Send.Control_Word = htonl(0x0B000000);
	NTP_Send.root_delay = 0;
	NTP_Send.root_dispersion = 0;
	NTP_Send.reference_identifier = 0;
	NTP_Send.reference_timestamp = 0;
	NTP_Send.originate_timestamp = 0;
	NTP_Send.receive_timestamp = 0;
	NTP_Send.transmit_timestamp_seconds = 0;
	NTP_Send.transmit_timestamp_fractions = 0;

	if (SOCKET_ERROR == sendto(soc, (const char*)&NTP_Send, sizeof(NTP_Send),
		0, (struct sockaddr*)&addrSrv, sizeof(addrSrv)))
	{
		return FALSE;
	}
	int sockaddr_Size = sizeof(addrSrv);
	if (SOCKET_ERROR == recvfrom(soc, (char*)&NTP_Recv, sizeof(NTP_Recv),
		0, (struct sockaddr*)&addrSrv, &sockaddr_Size))
	{
		closesocket(soc);
		return FALSE;
	}
	closesocket(soc); 
	Fly_web::RAW::UnInitSocket();


	// 获取时间服务器的时间
	ntp_time = ntohl(NTP_Recv.transmit_timestamp_seconds) - 2208988800;
	lpLocalTime = localtime(&ntp_time);
	if (lpLocalTime == NULL)
	{
		return FALSE;
	}
	// 获取新的时间
	newtime.wYear = lpLocalTime->tm_year + 1900;
	newtime.wMonth = lpLocalTime->tm_mon + 1;
	newtime.wDayOfWeek = lpLocalTime->tm_wday;
	newtime.wDay = lpLocalTime->tm_mday;
	newtime.wHour = lpLocalTime->tm_hour;
	newtime.wMinute = lpLocalTime->tm_min;
	newtime.wSecond = lpLocalTime->tm_sec;

	// 设置时间精度
	Splitseconds = (float)ntohl(NTP_Recv.transmit_timestamp_fractions);
	Splitseconds = (float)0.000000000200 * Splitseconds;
	Splitseconds = (float)1000.0 * Splitseconds;
	newtime.wMilliseconds = (unsigned   short)Splitseconds;

	// 修改本机系统时间
	SetLocalTime(&newtime);
	return TRUE;
}