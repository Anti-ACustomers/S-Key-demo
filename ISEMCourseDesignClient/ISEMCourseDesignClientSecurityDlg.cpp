// ISEMCourseDesignClientSecurityDlg.cpp: 实现文件
//

#include "pch.h"
#include "ISEMCourseDesignClientSecurityDlg.h"
#include "afxdialogex.h"
#include "ISEMCourseDesignClient.h"
#include "ISEMCourseDesignClientDlg.h"


// ISEMCourseDesignClientSecurityDlg 对话框

IMPLEMENT_DYNAMIC(ISEMCourseDesignClientSecurityDlg, CDialogEx)

ISEMCourseDesignClientSecurityDlg::ISEMCourseDesignClientSecurityDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ISEMCOURSEDESIGNCLIENTSECURITY_DIALOG, pParent)
	, cSocket(NULL)
{

}

ISEMCourseDesignClientSecurityDlg::~ISEMCourseDesignClientSecurityDlg()
{
}

void ISEMCourseDesignClientSecurityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ISEMCourseDesignClientSecurityDlg, CDialogEx)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// ISEMCourseDesignClientSecurityDlg 消息处理程序


void ISEMCourseDesignClientSecurityDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CWnd* parentDlg = this->GetParent();
	CDialogEx::OnClose();
	cSocket = NULL;
	((CISEMCourseDesignClientDlg*)parentDlg)->OnClose();
}


void ISEMCourseDesignClientSecurityDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialogEx::OnOK();
}

void ISEMCourseDesignClientSecurityDlg::SetSocket(ClientSocket* pSocket)
{
	cSocket = pSocket;
}