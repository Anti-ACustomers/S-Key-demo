#include <iostream>
#include <fstream>
#include "Server.h"
#include "ServerProcessed.h"
#include "OperationSignals.h"


int Server::InitDataBase(const char* ip, const char* name, const char* cypher, const char* database_name, const int port)
{
	if(!database.Connect(ip, name, cypher, database_name, port))
		return 1;
	return 0;
}

int Server::StartListen(int port)
{
	int ret;
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	ret = WSAStartup(sockVersion, &wsaData);
	if (ret != 0) //如果找不到合适的winsock.dll文件，返回。
	{
		printf("WSAStartup error !");
		return 1;
	}

	slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		printf("Invalid socket !");
		WSACleanup();
		return 1;
	}

	sockaddr_in sin;    //定义sockaddr_in变量
	sin.sin_family = AF_INET;  //为该socket变量个成员赋值
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;  //设置本地IP地址

	//调用socket API时强制类型转换sockaddr
	ret = bind(slisten, (sockaddr*)&sin, sizeof(sockaddr_in));
	if (ret == SOCKET_ERROR)
	{
		printf("bind error !");
		closesocket(slisten);
		WSACleanup();
		return 1;
	}

	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		printf("listen error !");
		closesocket(slisten);
		WSACleanup();
		return 1;
	}

	return 0;
}

void Server::Accept()
{
	SOCKET* sClient = new SOCKET;
	*sClient = accept(slisten, NULL, NULL);
	if (*sClient == INVALID_SOCKET)
	{
		printf("accept error !");
		return;
	}

	pParam p = new param{ sClient, this };

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Response, p, 0, NULL);
}

void Server::QueryAccount(const char* userId)
{
	database.QueryAccount(userId);
}

void Server::QueryAdmin(const char* userId)
{
	database.QueryAdmin(userId);
}

void Server::QueryHistoryPw(const char* userId)
{
	database.QueryHistoryPw(userId);
}

MYSQL_RES* Server::GetResult()
{
	return database.GetResult();
}

void Server::UpdateValue(int userType, std::string userId, std::string newIteration, bool changeRand, std::string newRandom, bool changePassword)
{
	if (userType == SERVER_USER_NORMAL) {
		database.UpdateChallengeAndRandom(userId, newIteration, changeRand, newRandom, changePassword);
	}
	else if (userType == SERVER_USER_ADMIN) {
		database.UdpateAdminChallengeAndRandom(userId, newIteration, changeRand, newRandom);
	}
}

int Server::GetDefaultChallenge()
{
	return database.GetDefaultChallenge();
}

bool Server::AddUser(std::string userName, std::string userId, std::string random, std::string iterate, int challenge)
{
	return database.AddUser(userName, userId, random, iterate, challenge);
}

void Server::SetFlag(std::string userId, const char* columnName)
{
	database.SetFlag(userId, columnName);
}

void Server::LoginFailed(std::string userId, bool isFirstError)
{
	database.LoginFailed(userId, isFirstError);
}

void Server::GetPastPassword(const char* userId)
{
	database.GetPastPassword(userId);
}

void Server::GetRequest(int requestType)
{
	database.GetRequest(requestType);
}

int Server::ProcessRequest(int requestType, int processMode, std::string userId)
{
	return database.ProcessRequest(requestType, processMode, userId);
}

void Server::WriteRecord(std::string subject, std::string operate, bool result, std::string reason, bool existObject, std::string object)
{
	time_t sysTime = time(0);
	tm* localTime = localtime(&sysTime);
	char* timeStr = ctime(&sysTime);
	*(timeStr + strlen(timeStr) - 1) = '\0';

	std::ofstream logFile;
	logFile.open("./record.log", std::ios::app);
	logFile << '[' << timeStr << ']' << ' ' << subject << ' ' << operate << ' ';
	if (existObject) {
		logFile << object << ' ';
	}
	if (result == true) {
		logFile << "success\n";
	}
	else {
		logFile << "fail" << ' ' << reason << std::endl;
	}
	logFile.close();
}