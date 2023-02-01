#pragma once
#include "afxdialogex.h"
#include "ClientSocket.h"


// ISEMCourseDesignClientSecurityDlg 对话框

class ISEMCourseDesignClientSecurityDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ISEMCourseDesignClientSecurityDlg)

public:
	ISEMCourseDesignClientSecurityDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ISEMCourseDesignClientSecurityDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ISEMCOURSEDESIGNCLIENTSECURITY_DIALOG };
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
};
