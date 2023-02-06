
// ISEMCourseDesignClientDlg.h: 头文件
//

#pragma once
#include "ClientSocket.h"
#include <sstream>

#define CLIENT_SECURITY_REQUEST_RESET_PASSWORD 3001
#define CLIENT_SECURITY_REQUEST_UNFREEZE 3002

typedef struct {
	CString ID;
	CString name;
}ResultRow;

typedef struct {
	int number;
	ResultRow* rows;
}ResultRows;

UINT GetLogFile(LPVOID pParam);

CString GetJson(CString* p, int number, bool operate, int type);

ResultRows GetRequest(ClientSocket* cSocket, int requestType);

std::string PasswdToMD5(std::string password, std::string rand, int challenge);

int UpdatePassword(CString newPassword, ClientSocket* cSocket);

BOOL CheckPassword(CString password);

// CISEMCourseDesignClientDlg 对话框
class CISEMCourseDesignClientDlg : public CDialogEx
{
// 构造
public:
	CISEMCourseDesignClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ISEMCOURSEDESIGNCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
		virtual void OnOK();

public:
	ClientSocket* cSocket;
	CTabCtrl selectCard;
	CButton btnUserLogin;
	CButton btnAdminLogin;
	CString ID;
	CString password;
	CButton btnRegister;

	afx_msg void OnTcnSelchangeTab3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonRegister();
	afx_msg void OnBnClickedButtonUserLogin();
	afx_msg void OnClose();
	CString confPasswd;
	CString userName;
	afx_msg void OnBnClickedButtonAdminLogin();
	CButton btnResetPassword;
	CButton btnUnfreeze;
	afx_msg void OnBnClickedButtonRequestReset();
	afx_msg void OnBnClickedButtonRequestUnfreeze();
};
