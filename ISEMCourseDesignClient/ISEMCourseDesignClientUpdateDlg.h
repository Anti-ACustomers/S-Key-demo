#pragma once
#include "afxdialogex.h"
#include "ClientSocket.h"


// ISEMCourseDesignClientUpdateDlg 对话框

class ISEMCourseDesignClientUpdateDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ISEMCourseDesignClientUpdateDlg)

public:
	ISEMCourseDesignClientUpdateDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ISEMCourseDesignClientUpdateDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ISEMCOURSEDESIGNCLIENTUPDATE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual void OnOK();
	void SetSocket(ClientSocket* pSocket);
private:
	ClientSocket* cSocket;
public:
	afx_msg void OnBnClickedButtonMustUpdate();
	CString confPassword;
	CString newPassword;
};
