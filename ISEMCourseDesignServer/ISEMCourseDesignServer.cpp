// ISEMCourseDesignServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include <io.h>
#include "Server.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libmysql.lib")

using namespace std;

#define INI_PATH ".\\setting.ini"

char databaseAddr[16];
char databaseUser[20];
char databasePasswd[256];
char databaseName[70];
int databasePort = 3306;
int noLoginPeriod;
int passwordChangePeriod;
int serverPort;

void ReadINI()
{
	if (_access(INI_PATH, 0) == -1) {

		WritePrivateProfileStringA(
			"database",
			"db_user",
			"root",
			INI_PATH
		);
		WritePrivateProfileStringA(
			"database",
			"db_password",
			"159753",
			INI_PATH
		);
		WritePrivateProfileStringA(
			"database",
			"db_port",
			"3306",
			INI_PATH
		);
		WritePrivateProfileStringA(
			"database",
			"db_database",
			"isem",
			INI_PATH
		);
		WritePrivateProfileStringA(
			"database",
			"db_server",
			"127.0.0.1",
			INI_PATH
		);
		WritePrivateProfileStringA(
			"server",
			"s_port",
			"10086",
			INI_PATH
		); WritePrivateProfileStringA(
			"function",
			"no_login_period",
			"30",
			INI_PATH
		);
		WritePrivateProfileStringA(
			"function",
			"password_change_period",
			"15",
			INI_PATH
		);
		
	}

	GetPrivateProfileStringA("database", "db_user", "", databaseUser, 20, INI_PATH);
	GetPrivateProfileStringA("database", "db_password", "", databasePasswd, 256, INI_PATH);
	databasePort = GetPrivateProfileIntA("database", "db_port", 3306, INI_PATH);
	GetPrivateProfileStringA("database", "db_database", "", databaseName, 70, INI_PATH);
	GetPrivateProfileStringA("database", "db_server", "127.0.0.1", databaseAddr, 16, INI_PATH);
	serverPort = GetPrivateProfileIntA("server", "s_port", 10086, INI_PATH);
	noLoginPeriod = GetPrivateProfileIntA("function", "no_login_period", 30, INI_PATH);
	passwordChangePeriod = GetPrivateProfileIntA("function", "password_change_period", 15, INI_PATH);

	if (strlen(databaseUser) == 0 || strlen(databasePasswd) == 0 || strlen(databaseName) == 0) {
		printf("The database user name, password, or database name is not set\n");
		exit(-1);
	}
}

int main()
{
	int ret;
	Server s;

	HANDLE handle = CreateMutex(NULL, FALSE, "useDatabase");
	HANDLE handleRecord = CreateMutex(NULL, FALSE, "writeRecord");

	ReadINI();

	ret = s.InitDataBase(
		databaseAddr, 
		databaseUser, 
		databasePasswd, 
		databaseName, 
		databasePort
	);
	if (ret) {
		printf("数据库连接失败");
		return -1;
	}
    
	ret = s.StartListen(serverPort);
	if (ret) {
		printf("启动监听服务失败");
		return -1;
	}
	
	while(true)
		s.Accept();

	WSACleanup();
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
