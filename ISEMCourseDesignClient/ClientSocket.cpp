#include "pch.h"
#include "ClientSocket.h"

ClientSocket::ClientSocket()
	: pDlg(NULL)
{

}

ClientSocket::~ClientSocket() {
	pDlg = NULL;
}

void ClientSocket::SetDialog(CISEMCourseDesignClientDlg* p) {
	pDlg = p;
}

void ClientSocket::OnClose(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类

	CAsyncSocket::OnClose(nErrorCode);
}


void ClientSocket::OnReceive(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类

	CAsyncSocket::OnReceive(nErrorCode);
}


void ClientSocket::OnSend(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类

	CAsyncSocket::OnSend(nErrorCode);
}
