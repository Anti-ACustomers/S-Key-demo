// ISEMCourseDesignClientUserDlg.cpp: 实现文件
//

#include "pch.h"
#include "ISEMCourseDesignClientUserDlg.h"
#include "afxdialogex.h"
#include "ISEMCourseDesignClientDlg.h"
#include "ISEMCourseDesignClient.h"


//using namespace boost::json;

// ISEMCourseDesignClientUserDlg 对话框

IMPLEMENT_DYNAMIC(ISEMCourseDesignClientUserDlg, CDialogEx)

ISEMCourseDesignClientUserDlg::ISEMCourseDesignClientUserDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ISEMCOURSEDESIGNCLIENTUSER_DIALOG, pParent)
	, cSocket(NULL)
	, newPassword(_T(""))
	, confPassword(_T(""))
{

}

ISEMCourseDesignClientUserDlg::~ISEMCourseDesignClientUserDlg()
{
}

void ISEMCourseDesignClientUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NEW_PASSWORD, newPassword);
	DDX_Text(pDX, IDC_EDIT_CONFIRM_NEW_PASSWORD, confPassword);
}


BEGIN_MESSAGE_MAP(ISEMCourseDesignClientUserDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_PASSWORD, &ISEMCourseDesignClientUserDlg::OnBnClickedButtonChangePassword)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_BACK, &ISEMCourseDesignClientUserDlg::OnBnClickedButtonBack)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &ISEMCourseDesignClientUserDlg::OnBnClickedButtonUpdate)
END_MESSAGE_MAP()


// ISEMCourseDesignClientUserDlg 消息处理程序


void ISEMCourseDesignClientUserDlg::OnBnClickedButtonChangePassword()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_CHANGE_PASSWORD)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_UPDATE)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON_BACK)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_NEW_PASSWORD)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_CONFIRM_NEW_PASSWORD)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_NEW_PASSWORD)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_CONFIRM_NEW_PASSWORD)->ShowWindow(SW_SHOW);
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

void ISEMCourseDesignClientUserDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialogEx::OnOK();
}


BOOL ISEMCourseDesignClientUserDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	GetDlgItem(IDC_BUTTON_UPDATE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_BACK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_NEW_PASSWORD)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_CONFIRM_NEW_PASSWORD)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_NEW_PASSWORD)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_CONFIRM_NEW_PASSWORD)->ShowWindow(SW_HIDE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void ISEMCourseDesignClientUserDlg::OnBnClickedButtonBack()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_UPDATE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_BACK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_NEW_PASSWORD)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_CONFIRM_NEW_PASSWORD)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_NEW_PASSWORD)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_CONFIRM_NEW_PASSWORD)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_CHANGE_PASSWORD)->ShowWindow(SW_SHOW);
}


void ISEMCourseDesignClientUserDlg::OnBnClickedButtonUpdate()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (!(newPassword.IsEmpty() || confPassword.IsEmpty()) && newPassword == confPassword) {
		
		if (!CheckPassword(newPassword)) {
			MessageBox("密码应包含英文字母大小写和数字，不能包含符号，且长度在8到20之间");
			return;
		}

		if (newPassword != confPassword) {
			MessageBox("前后密码不一致");
			return;
		}

		int ret = UpdatePassword(newPassword, cSocket);
		if (ret == S_ACCESS) {
			AfxMessageBox("密码修改成功");
		}
		else if (ret == S_FAILED) {
			AfxMessageBox("密码与过去5次内的密码重复");
		}
	}
}
