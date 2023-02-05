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
	, tableType(1)
{

}

ISEMCourseDesignClientSecurityDlg::~ISEMCourseDesignClientSecurityDlg()
{
}

void ISEMCourseDesignClientSecurityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SHOW, showTable);
	DDX_Control(pDX, IDC_TAB1, selectType);
}


BEGIN_MESSAGE_MAP(ISEMCourseDesignClientSecurityDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &ISEMCourseDesignClientSecurityDlg::OnTcnSelchangeTab1)
	ON_BN_CLICKED(IDC_BUTTON_PASS, &ISEMCourseDesignClientSecurityDlg::OnBnClickedButtonPass)
	ON_BN_CLICKED(IDC_BUTTON_REJECT, &ISEMCourseDesignClientSecurityDlg::OnBnClickedButtonReject)
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

BOOL ISEMCourseDesignClientSecurityDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	selectType.InsertItem(0, "重置密码");
	selectType.InsertItem(1, "请求解冻");

	DWORD dwStyle = showTable.GetExtendedStyle();
	dwStyle |= LVS_EX_CHECKBOXES;
	showTable.SetExtendedStyle(dwStyle);

	showTable.InsertColumn(0, "选取", LVCFMT_CENTER, 46);
	showTable.InsertColumn(1, "用户ID", LVCFMT_CENTER, 130);
	showTable.InsertColumn(2, "用户昵称", LVCFMT_CENTER, 130);

	ResultRows res = GetRequest(cSocket, CLIENT_SECURITY_REQUEST_RESET_PASSWORD);
	//GetRequest(cSocket, CLIENT_SECURITY_REQUEST_UNFREEZE);

	for (int i = 0; i < res.number; i++) {
		int row = showTable.InsertItem(i, "");

		showTable.SetItemText(row, 1, res.rows[i].ID);
		showTable.SetItemText(row, 2, res.rows[i].name);
	}

	delete[] res.rows;
	res.rows = NULL;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void ISEMCourseDesignClientSecurityDlg::SwitchTable(int requestType)
{
	showTable.DeleteAllItems();

	ResultRows res = GetRequest(cSocket, requestType);

	for (int i = 0; i < res.number; i++) {
		int row = showTable.InsertItem(i, "");

		showTable.SetItemText(row, 1, res.rows[i].ID);
		showTable.SetItemText(row, 2, res.rows[i].name);
	}

	delete[] res.rows;
	res.rows = NULL;
}

void ISEMCourseDesignClientSecurityDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	int select = selectType.GetCurSel();
	switch (select) {
	case 0: {
		SwitchTable(CLIENT_SECURITY_REQUEST_RESET_PASSWORD);
		tableType = 1;
		break;
	}
	case 1: {
		SwitchTable(CLIENT_SECURITY_REQUEST_UNFREEZE);
		tableType = 2;
		break;
	}
	}
	*pResult = 0;
}


void ISEMCourseDesignClientSecurityDlg::OnBnClickedButtonPass()
{
	// TODO: 在此添加控件通知处理程序代码
	int chooseCount = 0;
	for (int i = 0; i < showTable.GetItemCount(); i++) {
		if (showTable.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED || showTable.GetCheck(i)) {
			chooseCount++;
		}
	}

	if (chooseCount == 0) return;

	CString* p = new CString[chooseCount];
	for (int i = 0, j = 0; i < showTable.GetItemCount(); i++) {
		if (showTable.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED || showTable.GetCheck(i)) {
			CString str = showTable.GetItemText(i, 1);
			p[j] = str;
			j++;
		}
	}
	CString json = GetJson(p, chooseCount, true, tableType);

	cSocket->Send(json, json.GetLength());
	//AfxMessageBox(json);
	if(tableType == 1){
		SwitchTable(CLIENT_SECURITY_REQUEST_RESET_PASSWORD);
	}
	else if (tableType == 2) {
		SwitchTable(CLIENT_SECURITY_REQUEST_UNFREEZE);
	}
}


void ISEMCourseDesignClientSecurityDlg::OnBnClickedButtonReject()
{
	// TODO: 在此添加控件通知处理程序代码
	int chooseCount = 0;
	for (int i = 0; i < showTable.GetItemCount(); i++) {
		if (showTable.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED || showTable.GetCheck(i)) {
			chooseCount++;
		}
	}

	if (chooseCount == 0) return;

	CString* p = new CString[chooseCount];
	for (int i = 0, j = 0; i < showTable.GetItemCount(); i++) {
		if (showTable.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED || showTable.GetCheck(i)) {
			CString str = showTable.GetItemText(i, 1);
			p[j] = str;
			j++;
		}
	}
	CString json = GetJson(p, chooseCount, false, tableType);

	cSocket->Send(json, json.GetLength());
	//AfxMessageBox(json);
	if (tableType == 1) {
		SwitchTable(CLIENT_SECURITY_REQUEST_RESET_PASSWORD);
	}
	else if (tableType == 2) {
		SwitchTable(CLIENT_SECURITY_REQUEST_UNFREEZE);
	}
}
