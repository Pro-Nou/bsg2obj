#pragma once 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#include <string>
#include <iostream>

#pragma comment(lib, "WS2_32.lib")
using namespace std;
class TCPconnect
{
public:
	TCPconnect()
	{
	}
	~TCPconnect()
	{
	}

	string serverlocal="127.0.0.1";
	char szRecive[11] = { 0 };
	char szSend[11] = { 0 };
	char szCheck[11] = { 0 };
	SOCKET sServer;
	SOCKET sClient;
	SOCKET sLocal;
	char szAnswerClient[2] = "K";

	void serverSetUp()
	{

		// 启动并初始化Winsock库  
		WSADATA data;
		WORD wVersionRequested = MAKEWORD(2, 0);
		WSAStartup(wVersionRequested, &data);

		// 创建一个空的流式套接字（基于TCP）  

		sLocal = socket(AF_INET, SOCK_STREAM, 0);
		// 设定本地服务器的地址  
		sockaddr_in addrLocal;
		addrLocal.sin_family = AF_INET;
		addrLocal.sin_port = htons(75); // 随意设一个端口  
		addrLocal.sin_addr.S_un.S_addr = INADDR_ANY; // 设定成可以接受任何计算机的请求  

													 // 绑定地址并监听  
		bind(sLocal, (sockaddr*)&addrLocal, sizeof(addrLocal));
		listen(sLocal, 5); // 表示服务器已建立好，现等待请求连接  

																		   // 应答的同时获取客户端的套接字  

		sockaddr_in addrClient;
		int nSockAddrSize = sizeof(addrClient);
		sClient = accept(sLocal, (sockaddr*)&addrClient, &nSockAddrSize);
		send(sClient, szAnswerClient, sizeof(szAnswerClient), 0);
		//MessageBox(_T("connected1"));
		WSAStartup(wVersionRequested, &data);
		// 指定目标服务器端的地址并建立一个空的套接字，之后用于绑定改地址  
		sServer = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in addrServer;
		addrServer.sin_family = AF_INET;
		addrServer.sin_port = htons(76);
		addrServer.sin_addr.S_un.S_addr = inet_addr(serverlocal.c_str()); // 本地测试使用本地回环地址  																	 // 请求连接的同时绑定套接字  
		connect(sServer, (sockaddr*)&addrServer, sizeof(addrServer));
		// 接受客户端的回送消息并打印  
		recv(sServer, szRecive, sizeof(szRecive), 0); // TODO: 在此添加控件通知处理程序代码
		//MessageBox(_T("connected2"));

	};

	bool clientSetUp()
	{
		WSADATA data;
		WORD wVersionRequested;
		while (1)
		{
			wVersionRequested = MAKEWORD(2, 0);
			WSAStartup(wVersionRequested, &data);
			// 指定目标服务器端的地址并建立一个空的套接字，之后用于绑定改地址  
			sServer = socket(AF_INET, SOCK_STREAM, 0);
			sockaddr_in addrServer;
			addrServer.sin_family = AF_INET;
			addrServer.sin_port = htons(75);
			addrServer.sin_addr.S_un.S_addr = inet_addr(serverlocal.c_str()); // 本地测试使用本地回环地址  																	 // 请求连接的同时绑定套接字  
			connect(sServer, (sockaddr*)&addrServer, sizeof(addrServer));
			// 接受客户端的回送消息并打印  
			recv(sServer, szRecive, sizeof(szRecive), 0);
			cout << szRecive << endl;
			int times = 0;
			while (strcmp(szRecive, szAnswerClient) != 0)
			{
				times++;
				cout << "try connect to " + serverlocal << endl;
				Sleep(1000);
				connect(sServer, (sockaddr*)&addrServer, sizeof(addrServer));
				// 接受客户端的回送消息并打印  
				recv(sServer, szRecive, sizeof(szRecive), 0);
				cout << szRecive << endl;


				if (times >= 4)
				{
					int msgReturn = MessageBox(NULL, "could not connect to server after 4 calls", "Oops!", MB_ICONERROR | MB_RETRYCANCEL);
					if (msgReturn == 2)
						return false;
					else if (msgReturn == 4)
						break;
				}
			}
			if (strcmp(szRecive, szAnswerClient) == 0)
				break;
		}
		//static const char szAnswerClient[] = "K";
		// 启动并初始化Winsock库  
		WSAStartup(wVersionRequested, &data);

		// 创建一个空的流式套接字（基于TCP）  

		sLocal = socket(AF_INET, SOCK_STREAM, 0);
		// 设定本地服务器的地址  
		sockaddr_in addrLocal;
		addrLocal.sin_family = AF_INET;
		addrLocal.sin_port = htons(76); // 随意设一个端口  
		addrLocal.sin_addr.S_un.S_addr = INADDR_ANY; // 设定成可以接受任何计算机的请求  

													 // 绑定地址并监听  
		bind(sLocal, (sockaddr*)&addrLocal, sizeof(addrLocal));
		listen(sLocal, 5); // 表示服务器已建立好，现等待请求连接  

						   // 应答的同时获取客户端的套接字  

		sockaddr_in addrClient;
		int nSockAddrSize = sizeof(addrClient);
		sClient = accept(sLocal, (sockaddr*)&addrClient, &nSockAddrSize);
		send(sClient, szAnswerClient, sizeof(szAnswerClient), 0);
		//MessageBox(_T("connected"));
		// TODO: 在此添加控件通知处理程序代码
		return true;
	}
	void TCPshutdown()
	{
		closesocket(sLocal);
		closesocket(sClient);
		closesocket(sServer);
		WSACleanup();
	}
	int TCPsend(string str)
	{
		int TCPresult;
		strcpy(szSend, str.c_str());
		cout << szSend << endl;
		send(sServer, szSend, sizeof(szSend), 0);
		TCPresult = recv(sClient, szRecive, sizeof(szRecive), 0);
		if (TCPresult == 0)
			return 0;
		else if (TCPresult == SOCKET_ERROR)
			return SOCKET_ERROR;
		Sleep(50);
		cout << szRecive << endl;
		return 1;
	}
	int TCPrecv()
	{
		int TCPresult;
		TCPresult = recv(sClient, szRecive, sizeof(szRecive), 0);
		if (TCPresult == 0)
			return 0;
		else if (TCPresult == SOCKET_ERROR)
			return SOCKET_ERROR;
		Sleep(50);
		send(sServer, szAnswerClient, sizeof(szAnswerClient), 0);
		return 1;
	}
};