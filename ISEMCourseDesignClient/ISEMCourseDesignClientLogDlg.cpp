// ISEMCourseDesignClientLogDlg.cpp: 实现文件
//

#include "pch.h"
#include "ISEMCourseDesignClientLogDlg.h"
#include "afxdialogex.h"
#include "ISEMCourseDesignClient.h"
#include "ISEMCourseDesignClientDlg.h"


// ISEMCourseDesignClientLogDlg 对话框

IMPLEMENT_DYNAMIC(ISEMCourseDesignClientLogDlg, CDialogEx)

ISEMCourseDesignClientLogDlg::ISEMCourseDesignClientLogDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ISEMCOURSEDESIGNCLIENTLOG_DIALOG, pParent)
	, cSocket(NULL)
{

}

ISEMCourseDesignClientLogDlg::~ISEMCourseDesignClientLogDlg()
{
}

void ISEMCourseDesignClientLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ISEMCourseDesignClientLogDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_GET_LOG_FILE, &ISEMCourseDesignClientLogDlg::OnBnClickedButtonGetLogFile)
END_MESSAGE_MAP()


// ISEMCourseDesignClientLogDlg 消息处理程序


void ISEMCourseDesignClientLogDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialogEx::OnOK();
}


void ISEMCourseDesignClientLogDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CWnd* parentDlg = this->GetParent();
	CDialogEx::OnClose();
	cSocket = NULL;
	((CISEMCourseDesignClientDlg*)parentDlg)->OnClose();
}

void ISEMCourseDesignClientLogDlg::SetSocket(ClientSocket* pSocket)
{
	cSocket = pSocket;
}

void ISEMCourseDesignClientLogDlg::OnBnClickedButtonGetLogFile()
{
	// TODO: 在此添加控件通知处理程序代码
	GetLogFile(cSocket);
}
