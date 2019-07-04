#include "Fly_web.h"
#include "Fly_string.h"
#include <Afxinet.h>


#pragma warning(disable : 4996)

std::string Fly_web::RAW::IPPORT::getIpPort()
{ 
	return Fly_string::format("%s:%d", ip.c_str(), port); 
}


std::string Fly_web::PostFile(const char* Url, const char*  UploadName,  const std::vector<std::string>& _FilePathList)
{
	CInternetSession session("Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.87 Safari/537.36", 0);
	CHttpConnection* pConnection = NULL;
	CHttpFile* pFile = NULL;
	try {
		DWORD ServerType = 0;
		CString StrServer;
		CString StrObj;
		INTERNET_PORT ServerPort = 0;
		if(AfxParseURL(Url, ServerType, StrServer, StrObj, ServerPort))
		{
			if (AFX_INET_SERVICE_HTTP != ServerType && AFX_INET_SERVICE_HTTPS != ServerType)
			{
				return "erro -2";
			}
			if (ServerType == AFX_INET_SERVICE_HTTPS)
			{
				pConnection = session.GetHttpConnection(StrServer, INTERNET_FLAG_SECURE, ServerPort, NULL, NULL);//ȡ��һ��Https����
			}

			if (ServerType == AFX_INET_SERVICE_HTTP)
			{
				pConnection = session.GetHttpConnection(StrServer, ServerPort);//ȡ��һ��Https����
			}


			if (pConnection)
			{
				if (ServerType == AFX_INET_SERVICE_HTTPS)
				{
					pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST, StrObj, NULL, 1,
						NULL, NULL, INTERNET_FLAG_SECURE |
						INTERNET_FLAG_EXISTING_CONNECT |
						INTERNET_FLAG_RELOAD |
						INTERNET_FLAG_NO_CACHE_WRITE |
						INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
						INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
						INTERNET_FLAG_NO_COOKIES);//������ ���⼸����ʶ��Ҫ����
				}

				if (ServerType == AFX_INET_SERVICE_HTTP)
				{
					pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST, StrObj);
				}
				if (pFile)
				{
					/*szBoundary,�����һ���������Ա�ʶbody�е�����*/
					//�ô���Ҫ�����ӣ���������ǰ������е�����
					const char * szBoundary = "----------------------------096304854221961038114493";

					/*����ͷ��*/
					pFile->AddRequestHeaders("Accept:*/*");
					pFile->AddRequestHeaders("Accept-Encoding:gzip, deflate");
					//pFile->AddRequestHeaders(L"Accept-Language:zh-CN");
					//pFile->AddRequestHeaders(L"X-Requested-With:XMLHttpRequest");
					//pFile->AddRequestHeaders(L"Connection: keep-alive");
					//pFile->AddRequestHeaders(L"content-length: 11337");
					//pFile->AddRequestHeaders(L"Connection: keep-alive");

					//����ͷ�е�Content-Type�������Ҫ����boundary
					CString szContentType = "Content-Type:multipart/form-data; boundary=";
					szContentType += szBoundary;
					pFile->AddRequestHeaders(szContentType.GetBuffer(0));

					/*���ļ�,�ϳ�ÿ���ļ�ͷ������,ȡ���ļ����ݣ��Լ��ϳ�β�����ݣ������ܳ���*/
					struct DataInfo {
						char* pBuf;
						DWORD BufLen;
						DataInfo() {
							ZeroMemory(this, sizeof(DataInfo));
						}
					};
					std::vector<DataInfo> DataInfoList;




					DWORD dwTotalBodyLen = 0;
					std::vector<std::string>::const_iterator it = _FilePathList.begin();
					for (; it != _FilePathList.end(); it++)
					{
						CFile _File;
						if (_File.Open(it->c_str(), CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary))
						{

							CString FileName = _File.GetFileName();
							CString PreContent;
							/*		if (FileName.Right(3) == L"txt")
									{
										PreContent = L"text/plain";
									}
									else if (FileName.Right(3) == L"jpg")
									{
										PreContent =  L"image/jpeg";
									}
									else {
										PreContent = L"application/zip";
									}*/

							char strconnet[1024];
							//application/zip
							sprintf(strconnet, "\r\n------------------------------096304854221961038114493\r\n"
								"Content-Disposition:form-data;name=\"%s\";filename=\"%s\"\r\n"
								"Content-Type:image/jpeg\r\n\r\n", UploadName, FileName);

							//ÿ���ļ���ͷ����Ϣ
							DataInfo preDi;
							preDi.BufLen = strlen(strconnet);
							preDi.pBuf = new char[preDi.BufLen];
							memcpy(preDi.pBuf, strconnet, preDi.BufLen);
							DataInfoList.push_back(preDi);

							dwTotalBodyLen += preDi.BufLen;

							//�ļ�����
							DataInfo ContDi;
							ContDi.BufLen = (DWORD)_File.GetLength();
							ContDi.pBuf = new char[ContDi.BufLen];
							_File.Read(ContDi.pBuf, ContDi.BufLen);
							DataInfoList.push_back(ContDi);

							dwTotalBodyLen += ContDi.BufLen;
						}
					}

					//β����
					const char* szEndContent =
						"\r\n"
						"------------------------------096304854221961038114493--\r\n";
					//CString szEnd;
					//szEnd.Format(szEndContent, szBoundary);
					DataInfo EndDi;
					EndDi.BufLen = strlen(szEndContent);
					EndDi.pBuf = (char*)malloc(EndDi.BufLen);
					memcpy(EndDi.pBuf, szEndContent, EndDi.BufLen);
					DataInfoList.push_back(EndDi);

					dwTotalBodyLen += EndDi.BufLen;


					/*��������*/
					pFile->SendRequestEx(dwTotalBodyLen, HSR_SYNC | HSR_INITIATE);
					std::vector<DataInfo>::iterator it3 = DataInfoList.begin();
					for (; it3 != DataInfoList.end(); it3++)
					{
						pFile->Write(it3->pBuf, it3->BufLen);
						delete[]it3->pBuf;
					}
					pFile->EndRequest(HSR_SYNC);

					/*��ȡ״̬*/
					DWORD dwStatusCode = -1;
					pFile->QueryInfoStatusCode(dwStatusCode);
					if (dwStatusCode == HTTP_STATUS_OK)
					{
						//������
						char*	_RetStr = (char*)malloc((DWORD)pFile->GetLength()); // new char[(DWORD)pFile->GetLength()];
						int lenth = pFile->Read(_RetStr, (UINT)pFile->GetLength());
						if (pFile)
							delete pFile;
						if (pConnection)
							delete pConnection;
						session.Close();
						std::string rst = _RetStr;
						return rst;
					}
				}
			}
		}
	}
	catch (CInternetException* e)
	{
		delete e;
	}
	if (pFile)
		delete pFile;
	if (pConnection)
		delete pConnection;
	session.Close();

	return "error -1";
}