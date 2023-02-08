#include "ServerProcessed.h"
#include "OperationSignals.h"
#include "Server.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <openssl/md5.h>
#include <boost/json/src.hpp>
#include <boost\date_time\posix_time\posix_time.hpp>
#include <boost\date_time\gregorian\gregorian.hpp>

using namespace boost::json;
using namespace boost::posix_time;
using namespace boost::gregorian;

extern int noLoginPeriod;
extern int passwordChangePeriod;

std::string GetRandomString()
{
	srand(time(0));
	char randomStr[9];
	randomStr[8] = 0;
	char charPool[62] = {
		'0','1','2','3','4','5','6','7','8','9',
		'a','b','c','d','e','f','g','h','i','j','k','l','m',
		'n','o','p','q','r','s','t','u','v','w','x','y','z',
		'A','B','C','D','E','F','G','H','I','J','K','L','M',
		'N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
	};
	for (int i = 0; i < 8; i++) {
		int index = rand() % 62;
		randomStr[i] = charPool[index];
	}
	return std::string(randomStr);
}

std::string PasswdToMD5(std::string password, std::string rand, int challenge)
{
	unsigned char md5[MD5_DIGEST_LENGTH + 1];

	md5[MD5_DIGEST_LENGTH] = 0;

	std::string md5str;
	std::stringstream ss;
	md5str.append(password);
	md5str.append(rand);

	for (int i = 0; i < challenge; i++) {
		MD5((unsigned char*)md5str.c_str(), md5str.length(), md5);
		for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
			ss << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)md5[i];
		}
		md5str = ss.str();
		ss.clear();
		ss.str("");
	}

	return md5str;
}

void Login(SOCKET* sClient, object data, Server* server, std::string &userId, int &userType)
{
	char buff[1024 + 1] = {0};
	int ret;
	string id = data.at("ID").as_string();
	object root, sendData;
	std::string json;
	std::string newRandom;

	//占用临界资源数据库
	HANDLE handle = OpenMutex(NULL, FALSE, "useDatabase");
	HANDLE handleRecord = OpenMutex(NULL, FALSE, "writeRecord");
	WaitForSingleObject(handle, INFINITE);

	//查询数据库
	server->QueryAccount(id.c_str());
	MYSQL_RES* pResult = server->GetResult();

	//判断查询是否有结果
	int resNum = mysql_num_rows(pResult);
	if (resNum == 0) {
		//此处为账号不存在的处理
		root["operate"] = S_FAILED;
		root["data"] = sendData;
		json.append(serialize(root).c_str());
		
		WaitForSingleObject(handleRecord, INFINITE);
		server->WriteRecord(id.c_str(), "login", false, "User not exist");
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}

		ReleaseMutex(handle);
		return;
	}

	//取出查询结果中有效内容
	MYSQL_ROW row = mysql_fetch_row(pResult);
	std::string userName(row[1]);
	std::string blockTimeStr(row[2]);
	std::string errorTimeStr(row[3]);
	std::string lastLoginDateStr(row[4]);
	std::string updateDateStr(row[5]);
	std::string expectIterate(row[7]);
	int challenge = std::stoi(row[8]);
	std::string random(row[9]);
	std::string currentTimeStr(row[12]);

	//释放数据库
	ReleaseMutex(handle);

	//添加时间的比较，实现冻结和短时间阻止登录功能
	date updateDate = from_string(updateDateStr) + days(passwordChangePeriod);
	ptime unblockTime = time_from_string(blockTimeStr) + minutes(5);
	date lastLoginDate = from_string(lastLoginDateStr) + days(noLoginPeriod);
	ptime currentTime = time_from_string(currentTimeStr);
	ptime errorTime = time_from_string(errorTimeStr) + minutes(5);
	if (unblockTime > currentTime) {
		root["operate"] = S_BLOCK;
		root["data"] = sendData;
		json.append(serialize(root).c_str());

		WaitForSingleObject(handleRecord, INFINITE);
		server->WriteRecord(id.c_str(), "login", false, "User blocked");
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}

		return;
	}
	if (lastLoginDate < currentTime.date()) {
		root["operate"] = S_FROZEN;
		root["data"] = sendData;
		json.append(serialize(root).c_str());

		WaitForSingleObject(handleRecord, INFINITE);
		server->WriteRecord(id.c_str(), "login", false, "User frozen");
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}

		return;
	}

	//构造json发送挑战值和随机数
	if (challenge <= 1) {
		root["operate"] = S_NEW_RANDOM;
		sendData["newChallenge"] = server->GetDefaultChallenge();
		newRandom = GetRandomString();
		sendData["newRandom"] = newRandom;
	}
	else {
		root["operate"] = S_NEXTSTEP;
	}
	sendData["random"] = random;
	sendData["challenge"] = challenge;
	
	root["data"] = sendData;

	json.append(serialize(root).c_str());
	
	ret = send(*sClient, json.c_str(), json.length(), 0);
	if (SOCKET_ERROR == ret)
	{
		printf("send error !");
	}
	root.clear();
	sendData.clear();
	json.clear();

	//接收并处理客户端发送的迭代值
	ret = recv(*sClient, buff, 1024, 0);
	if (ret > 0) {
		buff[ret] = '\0';
		object iterateGroup = parse(buff).as_object();
		std::string iterate = iterateGroup.at("iterate").as_string().c_str();

		std::string recvIterate = PasswdToMD5(
			iterate,
			"",
			1
		);

		//收到迭代值后的处理
		if (strcmp(expectIterate.c_str(), recvIterate.c_str())) {
			//迭代值不正确
			WaitForSingleObject(handle, INFINITE);
			//server->UpdateValue(SERVER_USER_NORMAL, userId, iterate);
			server->LoginFailed(id.c_str(), errorTime < currentTime);
			ReleaseMutex(handle);

			WaitForSingleObject(handleRecord, INFINITE);
			server->WriteRecord(id.c_str(), "login", false, "Password wrong");
			ReleaseMutex(handleRecord);

			root["operate"] = S_FAILED;
			root["data"] = sendData;
			json.append(serialize(root).c_str());
			ret = send(*sClient, json.c_str(), json.length(), 0);
			if (SOCKET_ERROR == ret)
			{
				printf("send error !");
			}

			return;
		}
		else {
			//迭代值正确
			if (updateDate > currentTime.date()) {
				root["operate"] = S_ACCESS;
			}
			else {
				root["operate"] = S_UPDATE_PASSWORD;
			}
			
			sendData["userName"] = userName;
			root["data"] = sendData;
			json.append(serialize(root).c_str());

			WaitForSingleObject(handleRecord, INFINITE);
			server->WriteRecord(id.c_str(), "login", true);
			ReleaseMutex(handleRecord);

			ret = send(*sClient, json.c_str(), json.length(), 0);
			if (SOCKET_ERROR == ret)
			{
				printf("send error !");
			}
			userId = id.c_str();
			userType = 0;

			//更新account相关表项
			if (challenge > 1) {
				WaitForSingleObject(handle, INFINITE);
				server->UpdateValue(SERVER_USER_NORMAL, userId, iterate);
				ReleaseMutex(handle);
			}
			else {
				//获取新的迭代值，并更新到表中
				WaitForSingleObject(handle, INFINITE);
				std::string newIterate = iterateGroup.at("newIterate").as_string().c_str();
				server->UpdateValue(SERVER_USER_NORMAL, userId, newIterate, true, newRandom);
				ReleaseMutex(handle);
			}
		}
	}
	else {
		printf("receive error");
		return;
	}
}

void Register(SOCKET* sClient, object data, Server* server)
{
	std::string userId = data.at("ID").as_string().c_str();
	char recvBuff[2048 + 1];
	int ret = 0, challenge = 0;
	std::string iterate, random, name;

	object root, sData;
	std::string json;

	HANDLE handle = OpenMutex(NULL, FALSE, "useDatabase");
	HANDLE handleRecord = OpenMutex(NULL, FALSE, "writeRecord");
	WaitForSingleObject(handle, INFINITE);

	server->QueryAccount(userId.c_str());
	MYSQL_RES* pResult = server->GetResult();
	int resNum = mysql_num_rows(pResult);
	ReleaseMutex(handle);
	if (resNum != 0) {
		//此处为账号已存在的处理
		root["operate"] = S_USER_EXIST;
		root["data"] = sData;
		json.append(serialize(root).c_str());

		WaitForSingleObject(handleRecord, INFINITE);
		server->WriteRecord(userId.c_str(), "register", false, "User existed");
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}
		
		return;
	}

	random = GetRandomString();
	challenge = server->GetDefaultChallenge();

	root["operate"] = S_NEXTSTEP;
	sData["random"] = random;
	sData["challenge"] = challenge;
	root["data"] = sData;

	json.append(serialize(root).c_str());
	ret = send(*sClient, json.c_str(), json.length(), 0);
	if (SOCKET_ERROR == ret)
	{
		printf("send error !");
	}

	root.clear();
	sData.clear();
	json.clear();

	ret = recv(*sClient, recvBuff, 2048, 0);
	if (ret > 0) {
		recvBuff[ret] = 0;
		//printf("%s\n", recvBuff);
		root = parse(recvBuff).as_object();
		iterate = root.at("iterate").as_string().c_str();
		name = root.at("name").as_string().c_str();

		root.clear();

		if (server->AddUser(name, userId, random, iterate, challenge)) {
			root["operate"] = S_ACCESS;
			root["data"] = sData;

			json.append(serialize(root).c_str());

			WaitForSingleObject(handleRecord, INFINITE);
			server->WriteRecord(userId.c_str(), "register", true);
			ReleaseMutex(handleRecord);

			ret = send(*sClient, json.c_str(), json.length(), 0);
			if (SOCKET_ERROR == ret)
			{
				printf("send error !");
			}
		}
		else {
			root["operate"] = S_FAILED;
			root["data"] = sData;

			json.append(serialize(root).c_str());

			WaitForSingleObject(handleRecord, INFINITE);
			server->WriteRecord(userId.c_str(), "register", false, "Add user fail");
			ReleaseMutex(handleRecord);

			ret = send(*sClient, json.c_str(), json.length(), 0);
			if (SOCKET_ERROR == ret)
			{
				printf("send error !");
			}
		}
	}
}

void AdminLogin(SOCKET* sClient, object data, Server* server, std::string& userId, int& userType)
{
	char buff[1024 + 1] = { 0 };
	int ret;
	string id = data.at("ID").as_string();
	object root, sendData;
	std::string json;
	std::string newRandom;

	//占用临界资源数据库
	HANDLE handle = OpenMutex(NULL, FALSE, "useDatabase");
	HANDLE handleRecord = OpenMutex(NULL, FALSE, "writeRecord");
	WaitForSingleObject(handle, INFINITE);

	//查询数据库
	server->QueryAdmin(id.c_str());
	MYSQL_RES* pResult = server->GetResult();

	//判断查询是否有结果
	int resNum = mysql_num_rows(pResult);
	if (resNum == 0) {
		//此处为账号不存在的处理
		root["operate"] = S_FAILED;
		root["data"] = sendData;
		json.append(serialize(root).c_str());

		WaitForSingleObject(handleRecord, INFINITE);
		server->WriteRecord(id.c_str(), "adminLogin", false, "User not exist");
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}

		ReleaseMutex(handle);
		return;
	}

	//取出查询结果中有效内容
	MYSQL_ROW row = mysql_fetch_row(pResult);
	std::string userName(row[1]);
	std::string expectIterate(row[2]);
	int challenge = std::stoi(row[3]);
	std::string random(row[4]);
	int adminType = std::stoi(row[5]);

	//释放数据库
	ReleaseMutex(handle);

	//构造json发送挑战值和随机数
	if (challenge <= 1) {
		root["operate"] = S_NEW_RANDOM;
		sendData["newChallenge"] = server->GetDefaultChallenge();
		newRandom = GetRandomString();
		sendData["newRandom"] = newRandom;
	}
	else {
		root["operate"] = S_NEXTSTEP;
	}
	sendData["random"] = random;
	sendData["challenge"] = challenge;

	root["data"] = sendData;

	json.append(serialize(root).c_str());

	ret = send(*sClient, json.c_str(), json.length(), 0);
	if (SOCKET_ERROR == ret)
	{
		printf("send error !");
	}
	root.clear();
	sendData.clear();
	json.clear();

	//接收并处理客户端发送的迭代值
	ret = recv(*sClient, buff, 1024, 0);
	if (ret > 0) {
		buff[ret] = '\0';
		object iterateGroup = parse(buff).as_object();
		std::string iterate = iterateGroup.at("iterate").as_string().c_str();

		std::string recvIterate = PasswdToMD5(
			iterate,
			"",
			1
		);
		
		//收到迭代值后的处理
		if (strcmp(expectIterate.c_str(), recvIterate.c_str())) {
			//迭代值不正确
			root["operate"] = S_FAILED;
			root["data"] = sendData;
			json.append(serialize(root).c_str());

			WaitForSingleObject(handleRecord, INFINITE);
			server->WriteRecord(id.c_str(), "adminLogin", false, "Password wrong");
			ReleaseMutex(handleRecord);

			ret = send(*sClient, json.c_str(), json.length(), 0);
			if (SOCKET_ERROR == ret)
			{
				printf("send error !");
			}

			return;
		}
		else {
			//迭代值正确
			root["operate"] = S_ACCESS;
			sendData["userName"] = userName;
			sendData["adminType"] = adminType;
			root["data"] = sendData;
			json.append(serialize(root).c_str());

			WaitForSingleObject(handleRecord, INFINITE);
			server->WriteRecord(id.c_str(), "adminLogin", true);
			ReleaseMutex(handleRecord);

			ret = send(*sClient, json.c_str(), json.length(), 0);
			if (SOCKET_ERROR == ret)
			{
				printf("send error !");
			}
			userId = id.c_str();
			userType = adminType;

			//更新account相关表项
			if (challenge > 1) {
				WaitForSingleObject(handle, INFINITE);
				server->UpdateValue(SERVER_USER_ADMIN, userId, iterate);
				ReleaseMutex(handle);
			}
			else {
				//获取新的迭代值，并更新到表中
				WaitForSingleObject(handle, INFINITE);
				std::string newIterate = iterateGroup.at("newIterate").as_string().c_str();
				server->UpdateValue(SERVER_USER_ADMIN, userId, newIterate, true, newRandom);
				ReleaseMutex(handle);
			}
		}

	}
	else {
		printf("receive error");
		return;
	}
}

void ResetPassword(SOCKET* sClient, object data, Server* server)
{
	char buff[1024 + 1] = { 0 };
	int ret;
	string id = data.at("ID").as_string();
	object root, sendData;
	std::string json;

	//占用临界资源数据库
	HANDLE handle = OpenMutex(NULL, FALSE, "useDatabase");
	HANDLE handleRecord = OpenMutex(NULL, FALSE, "writeRecord");
	WaitForSingleObject(handle, INFINITE);

	//查询数据库
	server->QueryAccount(id.c_str());
	MYSQL_RES* pResult = server->GetResult();

	//判断查询是否有结果
	int resNum = mysql_num_rows(pResult);
	if (resNum == 0) {
		//此处为账号不存在的处理
		root["operate"] = S_FAILED;
		root["data"] = sendData;
		json.append(serialize(root).c_str());

		WaitForSingleObject(handleRecord, INFINITE);
		server->WriteRecord(id.c_str(), "requestResetPassword", false, "User not exist");
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}

		ReleaseMutex(handle);
		return;
	}

	//取出查询结果中有效内容
	MYSQL_ROW row = mysql_fetch_row(pResult);
	bool hadRequested = std::stoi(row[10]);

	ReleaseMutex(handle);

	if (hadRequested) {
		root["operate"] = S_REPEAT;
		root["data"] = sendData;
		json.append(serialize(root).c_str());

		WaitForSingleObject(handleRecord, INFINITE);
		server->WriteRecord(id.c_str(), "requestResetPassword", false, "Request repeat");
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}
		return;
	}
	else {
		WaitForSingleObject(handle, INFINITE);
		server->SetFlag(id.c_str(), DATABASE_RESET_PASSWORD);
		ReleaseMutex(handle);
		root["operate"] = S_ACCESS;
		root["data"] = sendData;
		json.append(serialize(root).c_str());

		WaitForSingleObject(handleRecord, INFINITE);
		server->WriteRecord(id.c_str(), "requestResetPassword", true);
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}
		return;
	}
}

void Unfreeze(SOCKET* sClient, object data, Server* server)
{
	//printf("ss");
	char buff[1024 + 1] = { 0 };
	int ret;
	string id = data.at("ID").as_string();
	object root, sendData;
	std::string json;

	//占用临界资源数据库
	HANDLE handle = OpenMutex(NULL, FALSE, "useDatabase");
	HANDLE handleRecord = OpenMutex(NULL, FALSE, "writeRecord");
	WaitForSingleObject(handle, INFINITE);

	//查询数据库
	server->QueryAccount(id.c_str());
	MYSQL_RES* pResult = server->GetResult();

	//判断查询是否有结果
	int resNum = mysql_num_rows(pResult);
	if (resNum == 0) {
		//此处为账号不存在的处理
		root["operate"] = S_FAILED;
		root["data"] = sendData;
		json.append(serialize(root).c_str());

		WaitForSingleObject(handleRecord, INFINITE);
		server->WriteRecord(id.c_str(), "requestUnfreeze", false, "User not exist");
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}

		ReleaseMutex(handle);
		return;
	}

	//取出查询结果中有效内容
	MYSQL_ROW row = mysql_fetch_row(pResult);
	std::string lastLoginDateStr(row[4]);
	bool hadRequested = std::stoi(row[11]);
	std::string currentTimeStr(row[12]);

	ReleaseMutex(handle);

	date lastLoginDate = from_string(lastLoginDateStr) + days(30);
	ptime currentTime = time_from_string(currentTimeStr);

	if (lastLoginDate > currentTime.date()) {
		root["operate"] = S_FAILED;
		root["data"] = sendData;
		json.append(serialize(root).c_str());

		WaitForSingleObject(handleRecord, INFINITE);
		server->WriteRecord(id.c_str(), "requestUnfreeze", false, "User not frozen");
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}
		return;
	}
	else if (hadRequested) {
		root["operate"] = S_REPEAT;
		root["data"] = sendData;
		json.append(serialize(root).c_str());

		WaitForSingleObject(handleRecord, INFINITE);
		server->WriteRecord(id.c_str(), "requestUnfreeze", false, "Request repeat");
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}
		return;
	}
	else {
		WaitForSingleObject(handle, INFINITE);
		server->SetFlag(id.c_str(), DATABASE_UNFREEZE);
		ReleaseMutex(handle);
		root["operate"] = S_ACCESS;
		root["data"] = sendData;
		json.append(serialize(root).c_str());

		WaitForSingleObject(handleRecord, INFINITE);
		server->WriteRecord(id.c_str(), "requestUnfreeze", true);
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);
		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}
		return;
	}
}

void UpdatePassword(SOCKET* sClient, object data, Server* server, std::string& userId, int& userType)
{
	if (userType != 0) {
		printf("user type error\n");
		return;
	}
	else {
		char buff[1024 + 1] = { 0 };
		int ret;
		object root, sendData;
		std::string iteration[5];
		std::string json, newRandom;
		char index[5] = { 0 };

		HANDLE handle = OpenMutex(NULL, FALSE, "useDatabase");
		HANDLE handleRecord = OpenMutex(NULL, FALSE, "writeRecord");
		WaitForSingleObject(handle, INFINITE);

		server->QueryAccount(userId.c_str());
		MYSQL_RES* pResult = server->GetResult();
		MYSQL_ROW row = mysql_fetch_row(pResult);

		object special;
		iteration[0] = std::string(row[7]);
		special["challenge"] = std::stoi(row[8]);
		special["random"] = std::string(row[9]);
		
		sendData["0"] = special;
		special.clear();

		server->GetPastPassword(userId.c_str());
		pResult = server->GetResult();

		//判断查询是否有结果
		int resNum = mysql_num_rows(pResult);
		
		for (int i = 1; i <= resNum; i++) {
			row = mysql_fetch_row(pResult);
			object jsonData;
			sprintf_s(index, "%d", i);
			iteration[i] = std::string(row[1]);
			jsonData["challenge"] = std::stoi(row[2]);
			jsonData["random"] = std::string(row[3]);
			sendData[index] = jsonData;
		}

		//释放数据库
		ReleaseMutex(handle);

		newRandom = GetRandomString();

		special["challenge"] = server->GetDefaultChallenge();
		special["random"] = newRandom;

		sendData["new"] = special;

		root["number"] = resNum + 1;
		root["data"] = sendData;

		json.append(serialize(root).c_str());
		ret = send(*sClient, json.c_str(), json.length(), 0);

		json.clear();
		root.clear();
		sendData.clear();

		if (SOCKET_ERROR == ret)
		{
			printf("send error !");
		}

		ret = recv(*sClient, buff, 1024, 0);
		if (ret > 0) {
			bool repeat = false;
			buff[ret] = 0;

			root = parse(buff).as_object();
			for (int i = 0; i < resNum + 1; i++) {
				sprintf_s(index, "%d", i);
				string recvIteration = root.at(index).as_string();
				if (!strcmp(recvIteration.c_str(), iteration[i].c_str())) {
					repeat = true;
					break;
				}
			}

			if (!repeat) {
				server->UpdateValue(
					SERVER_USER_NORMAL,
					userId, 
					root.at("new").as_string().c_str(),
					true,
					newRandom,
					true
				);

				root["operate"] = S_ACCESS;
				root["data"] = sendData;
				json.append(serialize(root).c_str());

				WaitForSingleObject(handleRecord, INFINITE);
				server->WriteRecord(userId.c_str(), "updatePassword", true);
				ReleaseMutex(handleRecord);

				ret = send(*sClient, json.c_str(), json.length(), 0);
				if (SOCKET_ERROR == ret)
				{
					printf("send error !");
				}
			}
			else {
				root["operate"] = S_FAILED;
				root["data"] = sendData;
				json.append(serialize(root).c_str());

				WaitForSingleObject(handleRecord, INFINITE);
				server->WriteRecord(userId.c_str(), "updatePassword", false, "Repeat the historical password");
				ReleaseMutex(handleRecord);

				ret = send(*sClient, json.c_str(), json.length(), 0);
				if (SOCKET_ERROR == ret)
				{
					printf("send error !");
				}
			}
		}
	}
}

void GetRequest(SOCKET* sClient, object data, Server* server, std::string& userId, int& userType)
{
	if (userType != 1) {
		printf("user type error\n");
		return;
	}
	else {
		char buff[1024 + 1] = { 0 };
		char index[5] = { 0 };
		int ret;
		object root, sendData;
		std::string json, newRandom;

		int requestType = data.at("requestType").as_int64();
		
		HANDLE handle = OpenMutex(NULL, FALSE, "useDatabase");
		HANDLE handleRecord = OpenMutex(NULL, FALSE, "writeRecord");
		WaitForSingleObject(handle, INFINITE);

		server->GetRequest(requestType);
		MYSQL_RES* pResult = server->GetResult();
		int resNum = mysql_num_rows(pResult);

		for (int i = 0; i < resNum; i++) {
			MYSQL_ROW row = mysql_fetch_row(pResult);
			object jsonData;
			sprintf_s(index, "%d", i);

			jsonData["id"] = std::string(row[0]);
			jsonData["name"] = std::string(row[1]);
			sendData[index] = jsonData;
		}

		//释放数据库
		ReleaseMutex(handle);

		root["number"] = resNum;
		root["data"] = sendData;

		json.append(serialize(root).c_str());

		WaitForSingleObject(handleRecord, INFINITE);
		if (requestType == 1) server->WriteRecord(userId.c_str(), "getResetPasswordRequest", true);
		else if (requestType == 2) server->WriteRecord(userId.c_str(), "getUnfreezeRequest", true);
		ReleaseMutex(handleRecord);

		ret = send(*sClient, json.c_str(), json.length(), 0);

		json.clear();
		root.clear();
		sendData.clear();
	}

}

void PassRequest(SOCKET* sClient, object data, Server* server, std::string& userId, int& userType)
{
	if (userType == 1) {
		int type = data.at("type").as_int64();
		int number = data.at("number").as_int64();
		char index[5] = { 0 };

		HANDLE handle = OpenMutex(NULL, FALSE, "useDatabase");
		HANDLE handleRecord = OpenMutex(NULL, FALSE, "writeRecord");
		WaitForSingleObject(handle, INFINITE);
		WaitForSingleObject(handleRecord, INFINITE);

		for (int i = 0; i < number; i++) {
			sprintf_s(index, "%d", i);
			std::string id = data.at(index).as_string().c_str();
			int ret = server->ProcessRequest(type, 1, id);

			if (ret == 0) {
				if (type == 1) server->WriteRecord(userId.c_str(), "resetPassword", true, "", true, id);
				else if(type == 2) server->WriteRecord(userId.c_str(), "unfreeze", true, "", true, id);
			}
			else {
				if (type == 1) server->WriteRecord(userId.c_str(), "resetPassword", false, "Unknown error", true, id);
				else if (type == 2) server->WriteRecord(userId.c_str(), "unfreeze", false, "Unknown error", true, id);
			}
		}

		ReleaseMutex(handle);
		ReleaseMutex(handleRecord);
	}
	else {
		printf("user type error\n");
		return;
	}
}

void RejectRequest(SOCKET* sClient, object data, Server* server, std::string& userId, int& userType)
{
	if (userType == 1) {
		int type = data.at("type").as_int64();
		int number = data.at("number").as_int64();
		char index[5] = { 0 };

		HANDLE handle = OpenMutex(NULL, FALSE, "useDatabase");
		HANDLE handleRecord = OpenMutex(NULL, FALSE, "writeRecord");
		WaitForSingleObject(handle, INFINITE);
		WaitForSingleObject(handleRecord, INFINITE);

		for (int i = 0; i < number; i++) {
			sprintf_s(index, "%d", i);
			std::string id = data.at(index).as_string().c_str();
			int ret = server->ProcessRequest(type, 2, id);

			if (ret == 0) {
				if (type == 1) server->WriteRecord(userId.c_str(), "rejectResetPassword", true, "", true, id);
				else if (type == 2) server->WriteRecord(userId.c_str(), "rejectUnfreeze", true, "", true, id);
			}
			else {
				if (type == 1) server->WriteRecord(userId.c_str(), "rejectResetPassword", false, "Unknown error", true, id);
				else if (type == 2) server->WriteRecord(userId.c_str(), "rejectUnfreeze", false, "Unknown error", true, id);
			}
		}

		ReleaseMutex(handle);
		ReleaseMutex(handleRecord);
	}
	else {
		printf("user type error\n");
		return;
	}
}

void GetLog(SOCKET* sClient, object data, Server* server, std::string& userId, int& userType)
{
	if (userType == 2) {
		char buff[1024 + 1] = { 0 };
		bool sendSuccess = true;
		object root;
		std::string json;

		HANDLE handleRecord = OpenMutex(NULL, FALSE, "writeRecord");
		WaitForSingleObject(handleRecord, INFINITE);

		std::ifstream inFile;
		inFile.open("record.log", std::ios::binary);
		if (!inFile.is_open()) {
			printf("File: record.log Not Found!\n");
			sendSuccess = false;
			server->WriteRecord(userId.c_str(), "getLogFile", false, "File not find");
		}
		else {
			inFile.seekg(0, std::ios::end);
			int length = (int)inFile.tellg();

			int totalNo = length % 1024 ? length / 1024 + 1 : length / 1024;

			root["length"] =length;
			root["totalNo"] = totalNo;

			json.append(serialize(root).c_str());
			send(*sClient, json.c_str(), json.length(), 0);

			root.clear();

			int ret = recv(*sClient, buff, 1024, 0);
			if (ret < 0) {
				printf("recv error\n");
			}
			buff[ret] = 0;
			
			root = parse(buff).as_object();
			if (root.at("ready").as_int64() != 1) return;

			inFile.seekg(0, std::ios::beg);

			memset(buff, 0, 1025);
			int fileBlockLength = 0;
			for (int i = 0; i < totalNo; i++)
			{
				inFile.read(buff, 1024);

				fileBlockLength = strlen(buff);

				// 发送buff中的字符串到sClient,实际上就是发送给客户端  
				if (send(*sClient, buff, fileBlockLength, 0) < 0)
				{
					sendSuccess = false;
					break;
				}

				memset(buff, 0, 1025);
			}
			inFile.close();

			if (sendSuccess == true) {
				server->WriteRecord(userId.c_str(), "getLogFile", true);
			}
			else {
				server->WriteRecord(userId.c_str(), "getLogFile", false, "Send file failed");
			}
		}
		
		ReleaseMutex(handleRecord);
	}
	else {
		printf("user type error\n");
		return;
	}
}

void Response(void* p)
{
	pParam param = (pParam)p;
	std::string userId;
	int userType = 0;

	SOCKET* sClient = param->sClient;
	Server* server = param->server;

	char buff[1024 + 1];

	while (true) {

		int ret = recv(*sClient, buff, 1024, 0);
		if (ret > 0) {
			buff[ret] = '\0';
			printf("%s\n", buff);
		}

		auto message = parse(buff).as_object();
		int operate = message.at("operate").as_int64();
		object data = message.at("data").as_object();

		switch (operate) {
		case C_REGISTER: {
			Register(sClient, data, server);
			break;
		}
		case C_LOGIN: {
			Login(sClient, data, server, userId, userType);
			break;
		}
		case C_RESET_PASSWORD: {
			ResetPassword(sClient, data, server);
			break;
		}
		case C_LOGOUT: {
			closesocket(*sClient);
			server = NULL;
			sClient = NULL;
			delete param;
			return;
		}
		case C_UNFREEZE: {
			Unfreeze(sClient, data, server);
			break;
		}
		case C_ADMIN_LOGIN: {
			//对于管理员登陆的处理
			AdminLogin(sClient, data, server, userId, userType);
			break;
		}
		case C_UPDATE_PASSWORD:
		{
			UpdatePassword(sClient, data, server, userId, userType);
			break;
		}
		case C_GET_REQUEST: {
			GetRequest(sClient, data, server, userId, userType);
			break;
		}
		case C_REQUEST_PASS: {
			PassRequest(sClient, data, server, userId, userType);
			break;
		}
		case C_REQUEST_REJECT: {
			RejectRequest(sClient, data, server, userId, userType);
			break;
		}
		case C_GET_LOG: {
			GetLog(sClient, data, server, userId, userType);
			break;
		}
		default: {
			printf("unknown error\n");
		}
		}
	}

}