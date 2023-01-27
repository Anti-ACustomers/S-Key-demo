#pragma once
#include "ClientSocket.h"

// ISEMCourseDesignClientUserDlg 对话框

class ISEMCourseDesignClientUserDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ISEMCourseDesignClientUserDlg)

public:
	ISEMCourseDesignClientUserDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ISEMCourseDesignClientUserDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ISEMCOURSEDESIGNCLIENTUSER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonChangePassword();
	afx_msg void OnClose();
	void SetSocket(ClientSocket* pSocket);

private:
	ClientSocket* cSocket;
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonBack();
};
