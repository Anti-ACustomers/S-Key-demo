#pragma once

#include <mysql.h>
#include <string>

#define DATABASE_RESET_PASSWORD "A_reset_password"
#define DATABASE_UNFREEZE "A_request_unfreeze"

class DataBase
{
public:
	DataBase();
	~DataBase();
	bool Connect(const char* ip, const char* name, const char* cypher, const char* database_name, const int port);
	void QueryAccount(const char* userId);
	void QueryAdmin(const char* userId);
	void QueryHistoryPw(const char* userId);
	MYSQL_RES* GetResult();
	//用于在登陆成功后更新Account表中挑战值，随机值和迭代值
	void UpdateChallengeAndRandom(std::string userId, std::string newIteration, bool changeRand, std::string newRandom, bool changePassword);
	//用于在管理员用户登陆成功后更新表项
	void UdpateAdminChallengeAndRandom(std::string userId, std::string newIteration, bool changeRand, std::string newRandom);
	int GetDefaultChallenge();
	//用于用户注册
	bool AddUser(std::string userName, std::string userId, std::string random, std::string iterate, int challenge);
	void SetFlag(std::string userId, const char* columnName);
	void LoginFailed(std::string userId, bool isFirstError);
	void GetPastPassword(const char* userId);
	void GetRequest(int requestType);
	int ProcessRequest(int requestType, int processMode, std::string userId);

private:
	MYSQL* mysql;
	bool isConnected;
	MYSQL_RES* result;
	char query[1024];
	int defaultChallenge;
};

