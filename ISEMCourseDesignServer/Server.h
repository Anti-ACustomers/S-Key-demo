#pragma once
#include <winsock2.h>
#include "DataBase.h"

#define SERVER_USER_ADMIN 10001
#define SERVER_USER_NORMAL 10002

class Server
{
public:
	int InitDataBase(const char* ip, const char* name, const char* cypher, const char* database_name, const int port);
	int StartListen(int port);
	void Accept();
	void QueryAccount(const char* userId);
	void QueryAdmin(const char* userId);
	void QueryHistoryPw(const char* userId);
	MYSQL_RES* GetResult();
	void UpdateValue(int userType, std::string userId, std::string newIteration, bool changeRand = false, std::string newRandom = "", bool changePassword = false);
	int GetDefaultChallenge();
	bool AddUser(std::string userName, std::string userId, std::string random, std::string iterate, int challenge);
	void SetFlag(std::string userId, const char* columnName);
	void LoginFailed(std::string userId, bool isFirstError);
	void GetPastPassword(const char* userId);
	void GetRequest(int requestType);
	int ProcessRequest(int requestType, int processMode, std::string userId);
	void WriteRecord(std::string subject, std::string operate, bool result, std::string reason = "", bool existObject = false, std::string object = "");

private:
	SOCKET slisten;
	DataBase database;
};

typedef struct {
	SOCKET* sClient;
	Server* server;
}param;

typedef param* pParam;
