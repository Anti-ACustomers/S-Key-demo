// ISEMCourseDesignClientUserDlg.cpp: 实现文件
//

#include "pch.h"
#include "ISEMCourseDesignClient.h"
#include "afxdialogex.h"
#include "ISEMCourseDesignClientUserDlg.h"
#include "ISEMCourseDesignClientDlg.h"


// ISEMCourseDesignClientUserDlg 对话框

IMPLEMENT_DYNAMIC(ISEMCourseDesignClientUserDlg, CDialogEx)

ISEMCourseDesignClientUserDlg::ISEMCourseDesignClientUserDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ISEMCOURSEDESIGNCLIENTUSER_DIALOG, pParent)
{

}

ISEMCourseDesignClientUserDlg::~ISEMCourseDesignClientUserDlg()
{
}

void ISEMCourseDesignClientUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ISEMCourseDesignClientUserDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_PASSWORD, &ISEMCourseDesignClientUserDlg::OnBnClickedButtonChangePassword)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// ISEMCourseDesignClientUserDlg 消息处理程序


void ISEMCourseDesignClientUserDlg::OnBnClickedButtonChangePassword()
{
	// TODO: 在此添加控件通知处理程序代码
	
}


void ISEMCourseDesignClientUserDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CWnd* parentDlg = this->GetParent();
	CDialogEx::OnClose();
	cSocket = NULL;
	((CISEMCourseDesignClientDlg*)parentDlg)->OnClose();
}


void ISEMCourseDesignClientUserDlg::SetSocket(ClientSocket* pSocket)
{
	cSocket = pSocket;
}