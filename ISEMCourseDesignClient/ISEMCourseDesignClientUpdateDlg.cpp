// ISEMCourseDesignClientUpdateDlg.cpp: 实现文件
//

#include "pch.h"
#include "ISEMCourseDesignClientUpdateDlg.h"
#include "afxdialogex.h"
#include "ISEMCourseDesignClient.h"
#include "ISEMCourseDesignClientDlg.h"
#include "ISEMCourseDesignClientUserDlg.h"


// ISEMCourseDesignClientUpdateDlg 对话框

IMPLEMENT_DYNAMIC(ISEMCourseDesignClientUpdateDlg, CDialogEx)

ISEMCourseDesignClientUpdateDlg::ISEMCourseDesignClientUpdateDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ISEMCOURSEDESIGNCLIENTUPDATE_DIALOG, pParent)
	, confPassword(_T(""))
	, newPassword(_T(""))
{

}

ISEMCourseDesignClientUpdateDlg::~ISEMCourseDesignClientUpdateDlg()
{
}

void ISEMCourseDesignClientUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_MUST_CONFIRM_NEW_PASSWORD, confPassword);
	DDX_Text(pDX, IDC_EDIT_MUST_NEW_PASSWORD, newPassword);
}


BEGIN_MESSAGE_MAP(ISEMCourseDesignClientUpdateDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_MUST_UPDATE, &ISEMCourseDesignClientUpdateDlg::OnBnClickedButtonMustUpdate)
END_MESSAGE_MAP()


// ISEMCourseDesignClientUpdateDlg 消息处理程序


void ISEMCourseDesignClientUpdateDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialogEx::OnOK();
}


void ISEMCourseDesignClientUpdateDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CWnd* parentDlg = this->GetParent();
	CDialogEx::OnClose();
	cSocket = NULL;
	((CISEMCourseDesignClientDlg*)parentDlg)->OnClose();
}

void ISEMCourseDesignClientUpdateDlg::SetSocket(ClientSocket* pSocket)
{
	cSocket = pSocket;
}

void ISEMCourseDesignClientUpdateDlg::OnBnClickedButtonMustUpdate()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (!(newPassword.IsEmpty() || confPassword.IsEmpty()) && newPassword == confPassword) {
		int ret = UpdatePassword(newPassword, cSocket);
		if (ret == S_ACCESS) {
			AfxMessageBox("密码修改成功");

			ISEMCourseDesignClientUserDlg* dlg = new ISEMCourseDesignClientUserDlg;
			dlg->SetSocket(cSocket);
			dlg->Create(IDD_ISEMCOURSEDESIGNCLIENTUSER_DIALOG, this->GetParent());
			dlg->ShowWindow(SW_SHOW);
			this->ShowWindow(SW_HIDE);
		}
		else if (ret == S_FAILED) {
			AfxMessageBox("密码与过去5次内的密码重复");
		}
	}
}
