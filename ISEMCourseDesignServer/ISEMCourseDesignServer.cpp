// ISEMCourseDesignServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include <winsock2.h>
#include <openssl/md5.h>
#include <mysql.h>
#include "OperationSignals.h"
#pragma comment(lib, "WS2_32") // 链接到WS2_32.lib
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libmysql.lib")


#define BUFFERLEN 1024 //数据缓冲区大小 

char sendBuf[BUFFERLEN];//用户发送缓冲区
char recvBuf[BUFFERLEN];//用户接收缓冲区
int sendlen, recvlen;//发送\接收数据长度	
int connectNum = 0;

fd_set fdSocket, fdRead, fdWrite; //声明套接字集合

void handleListen(SOCKET  sListen) {
	sockaddr_in  clientAddr;
	int nAddrLen = sizeof(clientAddr);
	SOCKET sNewClient = accept(sListen, (SOCKADDR*)&clientAddr, &nAddrLen);

	u_long ul = 1; //设置套接字为非阻塞模式
	ioctlsocket(sNewClient, FIONBIO, (unsigned long*)&ul);

	//输出客户端的IP地址和端口
	printf("来自[%s],端口[%d]的一个客户访问服务器\n",
		inet_ntoa(clientAddr.sin_addr),
		ntohs(clientAddr.sin_port));

	FD_SET(sNewClient, &fdSocket);//增加到临时集合

	fdWrite = fdSocket;//设置写集合
	//回应客户端的连接，并告知其访问序号
	sprintf(sendBuf, "欢迎来自[%s],端口[%d]的客户访问!您是第[%d]位访问者!",
		inet_ntoa(clientAddr.sin_addr),
		ntohs(clientAddr.sin_port),
		connectNum);
	if (FD_ISSET(sNewClient, &fdWrite)) {
		send(sNewClient, sendBuf, strlen(sendBuf) + 1, 0);
	}
}

void handleWrite(int i) {
	sprintf(sendBuf, "I am a server!!!");
	sendlen = send(fdSocket.fd_array[i], sendBuf, strlen(sendBuf) + 1, 0);
	if (SOCKET_ERROR == sendlen)// 连接关闭
{
		printf("发送数据错误.\n");
	}
	else {
		printf(">>>成功发送数据: [% s] ; 发送字节[% d]\n", sendBuf, sendlen);
	}
}

void handleRead(int i) {
	//获取客户端的地址及端口号
	sockaddr_in  clientAddr;
	int nAddrLen = sizeof(clientAddr);
	getpeername(fdSocket.fd_array[i], (sockaddr*)&clientAddr, &nAddrLen);
	//判断套接字在读集合，处理接收过程
	recvlen = recv(fdSocket.fd_array[i], recvBuf, sizeof(recvBuf), 0);
	if (SOCKET_ERROR == recvlen)// 连接关闭
	{
		//输出关闭连接的客户端的信息
		printf("客户端[%s:%d]已关闭.\n",
			inet_ntoa(clientAddr.sin_addr),
			ntohs(clientAddr.sin_port));
		//关闭响应套接字
		closesocket(fdSocket.fd_array[i]);
		FD_CLR(fdSocket.fd_array[i], &fdSocket);
	}
	else// 可读
	{
		recvBuf[recvlen] = '\0';
		//输出消息发送方的地址信息和所发送的消息
		printf("<<<收到客户机[%s:%d]说:[%s] ; 接收字节[%d]\n",
			inet_ntoa(clientAddr.sin_addr),
			ntohs(clientAddr.sin_port),
			recvBuf,
			recvlen);
		if (FD_ISSET(fdSocket.fd_array[i], &fdWrite)) {
			handleWrite(i);
		} //判断套接字在写集合，处理发送过程
	}//可读

}



void main() {
	//载入Winsock库
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	int ret = WSAStartup(version, &wsaData);
	if (ret != 0) {
		printf(" 加载Winsock库错误! \n");
		return;
	}


	// 创建监听套接字
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sListen) {
		printf(" 创建套接字失败：%d\n", WSAGetLastError());
		return;
	}

	u_long ul = 1;
	if (SOCKET_ERROR == ioctlsocket(sListen, FIONBIO, (unsigned long*)&ul)) {
		printf("设置套接字为非阻塞模式失败!\n");
	}

	//设置服务器端地址
	sockaddr_in addrSrv;//声明服务器端地址
	addrSrv.sin_family = AF_INET;//Internet协议地址族
	addrSrv.sin_addr.S_un.S_addr = INADDR_ANY;//系统自动指定IP
	addrSrv.sin_port = htons(SERVER_PORT);//把16位端口转换为网络字节

	// 绑定套接字到本地机器
	if (SOCKET_ERROR == bind(sListen, (sockaddr*)&addrSrv, sizeof(addrSrv))) {
		printf(" 绑定失败\n");
		return;
	}
	listen(sListen, 5); // 进入监听模式


	// select模型处理过程
	FD_ZERO(&fdSocket);// 初始化一个套接字集合fdSocket
	FD_ZERO(&fdRead);// 初始化一个套接字集合fdRead
	FD_ZERO(&fdWrite);// 初始化一个套接字集合fdWrite

	FD_SET(sListen, &fdSocket);//添加监听套接字到这个集合

	printf("select模型服务器正在运行......\n");
	while (TRUE) {//进入无限循环，等待客户的连接请求
		fdRead = fdSocket;
		fdWrite = fdSocket;//设置读写集合
		int i, nRet;
		timeval myTimeout;
		myTimeout.tv_sec = 10;
		myTimeout.tv_usec = 100;
		nRet = select(0, &fdRead, &fdWrite, NULL, NULL);
		if (FD_ISSET(sListen, &fdRead)) {//说明有新连接到来读集合
			//记录连接总次数
			connectNum++;
			handleListen(sListen);
			continue;
		}
		for (i = 0; i < (int)fdSocket.fd_count; i++) {
			if (FD_ISSET(fdSocket.fd_array[i], &fdRead)) {
				handleRead(i);
			} //判断套接字在读集合，处理接收过程
}
		//for end 
		Sleep(2000);

	}//while end	
	closesocket(sListen);//关闭监听套接字
	WSACleanup();//释放Winsock库
	return;
} //main end



// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
