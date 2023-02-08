#include "DataBase.h"
#include <stdio.h>

DataBase::DataBase() {
    isConnected = false;
    mysql = new MYSQL;
    result = NULL;
    memset(query, 0, sizeof(query));
    defaultChallenge = 255;
 }

DataBase::~DataBase() {
    mysql_close(mysql);
    delete mysql;
    mysql = NULL;
}

bool DataBase::Connect(const char* ip, const char* name, const char* cypher, const char* database_name, const int port) {
    if (true == isConnected)
    {
        printf("Database connected\n");
        return false;
    }
    //初始化mysql  
    mysql_init(mysql);
    //返回false则连接失败，返回true则连接成功  
    if (!(mysql_real_connect(mysql, ip, name, cypher, database_name, port, NULL, 0))) //中间分别是主机，用户名，密码，数据库名，端口号（可以写默认0或者3306等），可以先写成参数再传进去  
    {
        printf("Error connecting to database:%s\n", mysql_error(mysql));
        return false;
    }
    else
    {
        isConnected = true;
        printf("Connected succeed\n\n");
        return true;
    }

    //读取配置文件中的默认挑战值
    defaultChallenge = 255;

    return true;
}

void DataBase::QueryAccount(const char* userId)
{
    sprintf_s(query, "select *, now() from Account where A_uid = '%s'", userId);
    mysql_query(mysql, query);
    if (result != NULL) {
        mysql_free_result(result);
        result = NULL;
    }
    result = mysql_store_result(mysql);
}

void DataBase::QueryAdmin(const char* userId)
{
    sprintf_s(query, "select * from administrator where AD_id = '%s'", userId);
    mysql_query(mysql, query);
    if (result != NULL) {
        mysql_free_result(result);
        result = NULL;
    }
    result = mysql_store_result(mysql);
}

void DataBase::QueryHistoryPw(const char* userId)
{
    sprintf_s(query, "select * from password where A_uid = '%s'", userId);
    mysql_query(mysql, query);
    if (result != NULL) {
        mysql_free_result(result);
        result = NULL;
    }
    result = mysql_store_result(mysql);
}

MYSQL_RES* DataBase::GetResult()
{
    return result;
}

void DataBase::UpdateChallengeAndRandom(std::string userId, std::string newIteration, bool changeRand = false, std::string newRandom = "", bool changePassword = false)
{
    if (changePassword) {
        sprintf_s(query, "insert into password(P_iteration_value, P_challenge_value, P_random_num, A_uid) select A_iteration_value, A_challenge_value, A_random_num, A_uid from account where A_uid = '%s'", userId.c_str());
        mysql_query(mysql, query);

        sprintf_s(query, "update account set A_update_time = now() where A_uid = '%s'", userId.c_str());
        mysql_query(mysql, query);
    }
    if (changeRand) {
        sprintf_s(query, "update account set A_reset_password = false, A_login_time = date(now()), A_random_num = '%s', A_iteration_value = '%s', A_challenge_value = %d where A_uid = '%s'", newRandom.c_str(), newIteration.c_str(), defaultChallenge, userId.c_str());
        mysql_query(mysql, query);
    }
    else {
        sprintf_s(query, "update account set A_reset_password = false, A_login_time = date(now()), A_iteration_value = '%s', A_challenge_value = A_challenge_value - 1 where A_uid = '%s'", newIteration.c_str(), userId.c_str());
        mysql_query(mysql, query);
    }
}

void DataBase::UdpateAdminChallengeAndRandom(std::string userId, std::string newIteration, bool changeRand = false, std::string newRandom = "")
{
    if (changeRand) {
        sprintf_s(query, "update administrator set AD_random_num = '%s', AD_iteration_value = '%s', AD_challenge_value = %d where AD_id = '%s'", newRandom.c_str(), newIteration.c_str(), defaultChallenge, userId.c_str());
        mysql_query(mysql, query);
    }
    else {
        sprintf_s(query, "update administrator set AD_iteration_value = '%s', AD_challenge_value = AD_challenge_value - 1 where AD_id = '%s'", newIteration.c_str(), userId.c_str());
        mysql_query(mysql, query);
    }
}

int DataBase::GetDefaultChallenge()
{
    return defaultChallenge;
}

bool DataBase::AddUser(std::string userName, std::string userId, std::string random, std::string iterate, int challenge)
{
    sprintf_s(query, "insert into account(A_name, A_uid, A_random_num, A_iteration_value, A_challenge_value, A_login_time, A_update_time) values('%s', '%s', '%s', '%s', %d, date(now()), date(now()))", userName.c_str(), userId.c_str(), random.c_str(), iterate.c_str(), challenge);
    int ret = mysql_query(mysql, query);
    if (ret == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void DataBase::SetFlag(std::string userId, const char* columnName)
{
    sprintf_s(query, "update account set  %s = true where A_uid = '%s'", columnName, userId.c_str());
    mysql_query(mysql, query);
}

void DataBase::LoginFailed(std::string userId, bool isFirstError)
{
    if (isFirstError) {
        sprintf_s(query, "update account set A_error_time = now(), A_error_times = 1 where A_uid = '%s'", userId.c_str());
        mysql_query(mysql, query);
    }
    else {
        sprintf_s(query, "update account set A_error_times = A_error_times + 1 where A_uid = '%s'", userId.c_str());
        mysql_query(mysql, query);
    }
}

void DataBase::GetPastPassword(const char* userId)
{
    sprintf_s(query, "select * from password where A_uid = '%s' order by P_no desc limit 0, 4", userId);
    mysql_query(mysql, query);
    if (result != NULL) {
        mysql_free_result(result);
        result = NULL;
    }
    result = mysql_store_result(mysql);
}

void DataBase::GetRequest(int requestType)
{
    if(requestType == 1){
        sprintf_s(query, "select A_uid, A_name from account where A_reset_password = 1");
    }
    else if(requestType == 2){
        sprintf_s(query, "select A_uid, A_name from account where A_request_unfreeze = 1");
    }
    else {
        return;
    }
    mysql_query(mysql, query);
    if (result != NULL) {
        mysql_free_result(result);
        result = NULL;
    }
    result = mysql_store_result(mysql);
}

int DataBase::ProcessRequest(int requestType, int processMode, std::string userId)
{
    if (requestType == 1) {
        if (processMode == 1) {
            //52af3ce8a82f62707789fe00899ed3f0
            sprintf_s(query, "insert into password(P_iteration_value, P_challenge_value, P_random_num, A_uid) select A_iteration_value, A_challenge_value, A_random_num, A_uid from account where A_uid = '%s'", userId.c_str());
            mysql_query(mysql, query);

            sprintf_s(query, "update account set A_update_time = '2000-1-1', A_iteration_value = '52af3ce8a82f62707789fe00899ed3f0', A_random_num = '123456', A_challenge_value = 1, A_reset_password = false where A_uid = '%s'", userId.c_str());
        }
        else if (processMode == 2) {
            sprintf_s(query, "update account set A_reset_password = false where A_uid = '%s'", userId.c_str());
        }
        else {
            return 1;
        }
    }
    else if (requestType == 2) {
        if (processMode == 1) {
            sprintf_s(query, "update account set A_login_time = now(), A_request_unfreeze = false where A_uid = '%s'", userId.c_str());
        }
        else if (processMode == 2) {
            sprintf_s(query, "update account set A_request_unfreeze = false where A_uid = '%s'", userId.c_str());
        }
        else {
            return 1;
        }
    }
    else {
        return 1;
    }

    int ret = mysql_query(mysql, query);
    return ret;
}
