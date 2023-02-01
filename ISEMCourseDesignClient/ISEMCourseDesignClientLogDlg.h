#pragma once
#include "afxdialogex.h"
#include "ClientSocket.h"


// ISEMCourseDesignClientLogDlg 对话框

class ISEMCourseDesignClientLogDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ISEMCourseDesignClientLogDlg)

public:
	ISEMCourseDesignClientLogDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ISEMCourseDesignClientLogDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ISEMCOURESEDESIGNCLIENTLOG_DIALOG };
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
