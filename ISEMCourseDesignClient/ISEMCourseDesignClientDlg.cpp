
// ISEMCourseDesignClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "ISEMCourseDesignClient.h"
#include "ISEMCourseDesignClientDlg.h"
#include "ISEMCourseDesignClientUserDlg.h"
#include "afxdialogex.h"
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>

#include <boost/json/src.hpp>

using namespace std;
using namespace boost::json;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::string PasswdToMD5(std::string password, std::string rand, int challenge)
{
	unsigned char md5[MD5_DIGEST_LENGTH + 1];

	md5[MD5_DIGEST_LENGTH] = 0;

	std::string md5str;
	std::stringstream ss;
	md5str.append(password);
	md5str.append(rand);

	for (int i = 0; i < challenge; i++) {
		MD5((unsigned char*)md5str.c_str(), md5str.length(), md5);
		for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
			ss << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)md5[i];
		}
		md5str = ss.str();
		ss.clear();
		ss.str("");
	}

	return md5str;
}

BOOL CheckPassword(CString password)
{
	BOOL upper = FALSE, lower = FALSE, num = FALSE;
	for (int i = 0; i < password.GetLength(); i++) {
		CHAR chr = password[i];
		if (chr >= 'a' && chr <= 'z') {
			lower = TRUE;
		}
		else if (chr >= 'A' && chr <= 'z') {
			upper = TRUE;
		}
		else if (chr >= '0' && chr <= '9') {
			num = TRUE;
		}
		else {
			return FALSE;
		}

		if (lower && upper && num) {
			return TRUE;
		}
	}

	return FALSE;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CISEMCourseDesignClientDlg 对话框



CISEMCourseDesignClientDlg::CISEMCourseDesignClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ISEMCOURSEDESIGNCLIENT_DIALOG, pParent)
	, ID(_T(""))
	, password(_T(""))
	, confPasswd(_T(""))
	, userName(_T(""))
	, cSocket(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CISEMCourseDesignClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB3, selectCard);
	DDX_Control(pDX, IDC_BUTTON_USER_LOGIN, btnUserLogin);
	DDX_Control(pDX, IDC_BUTTON_ADMIN_LOGIN, btnAdminLogin);
	DDX_Text(pDX, IDC_EDIT_ID, ID);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, password);
	DDX_Control(pDX, IDC_BUTTON_REGISTER, btnRegister);
	DDX_Text(pDX, IDC_EDIT_CONFIRM_PASSWORD, confPasswd);
	DDX_Text(pDX, IDC_EDIT_USER_NAME, userName);
}

BEGIN_MESSAGE_MAP(CISEMCourseDesignClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB3, &CISEMCourseDesignClientDlg::OnTcnSelchangeTab3)
	ON_BN_CLICKED(IDC_BUTTON_USER_LOGIN, &CISEMCourseDesignClientDlg::OnBnClickedButtonUserLogin)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_REGISTER, &CISEMCourseDesignClientDlg::OnBnClickedButtonRegister)
END_MESSAGE_MAP()


// CISEMCourseDesignClientDlg 消息处理程序

BOOL CISEMCourseDesignClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	cSocket = new ClientSocket;

	cSocket->Create();
	int ret = cSocket->Connect("127.0.0.1", SERVER_PORT);
	if (!ret) {
		MessageBox("无法连接到服务器！");
		cSocket = NULL;
		OnCancel();
	}
	selectCard.InsertItem(0, "用户");
	selectCard.InsertItem(1, "管理员");
	selectCard.InsertItem(2, "新用户注册");

	btnAdminLogin.ShowWindow(SW_HIDE);
	btnRegister.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_CONFIRM_PASSWORD)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_CONFIRM_PASSWORD)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_USER_NAME)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_USER_NAME)->ShowWindow(SW_HIDE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CISEMCourseDesignClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CISEMCourseDesignClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CISEMCourseDesignClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CISEMCourseDesignClientDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialogEx::OnOK();
}


void CISEMCourseDesignClientDlg::OnTcnSelchangeTab3(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	int select = selectCard.GetCurSel();
	switch (select) {
	case 0: {
		btnUserLogin.ShowWindow(SW_SHOW);
		btnAdminLogin.ShowWindow(SW_HIDE);
		btnRegister.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_CONFIRM_PASSWORD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_CONFIRM_PASSWORD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_USER_NAME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_USER_NAME)->ShowWindow(SW_HIDE);
		ID.Empty();
		password.Empty();
		confPasswd.Empty();
		userName.Empty();
		UpdateData(FALSE);
		break;
	}
	case 1: {
		btnUserLogin.ShowWindow(SW_HIDE);
		btnAdminLogin.ShowWindow(SW_SHOW);
		btnRegister.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_CONFIRM_PASSWORD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_CONFIRM_PASSWORD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_USER_NAME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_USER_NAME)->ShowWindow(SW_HIDE);
		ID.Empty();
		password.Empty();
		confPasswd.Empty();
		userName.Empty();
		UpdateData(FALSE);
		break;
	}
	case 2: {
		btnUserLogin.ShowWindow(SW_HIDE);
		btnAdminLogin.ShowWindow(SW_HIDE);
		btnRegister.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_CONFIRM_PASSWORD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_CONFIRM_PASSWORD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_USER_NAME)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_USER_NAME)->ShowWindow(SW_SHOW);
		ID.Empty();
		password.Empty();
		confPasswd.Empty();
		userName.Empty();
		UpdateData(FALSE);
		break;
	}
	}
	*pResult = 0;
}


void CISEMCourseDesignClientDlg::OnBnClickedButtonUserLogin()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (!(ID.IsEmpty() || password.IsEmpty())) {

		char recvBuff[2048 + 1];
		int ret = 0;
		CString iterate, newIterate;

		object root, data;
		CString json;

		/*
		* 构造第一次发送的json
		* 包括进行的操作标识符，ID
		*/
		root["operate"] = C_LOGIN;
		data["ID"] = ID.GetString();
		root["data"] = data;

		json.Append(serialize(root).c_str());
		cSocket->Send(json, json.GetLength());

		//使用后清空，预备下次使用
		root.clear();
		data.clear();
		json.Empty();

		ret = cSocket->Receive(recvBuff, 2048);
		if (ret <= 0) {
			MessageBox("连接出错");
		}
		recvBuff[ret] = 0;

		//对接受的内容进行处理
		root = parse(recvBuff).as_object();
		data = root.at("data").as_object();
		if (root.at("operate").as_int64() == S_LOGIN_FAILED) {
			MessageBox("账号或密码错误");
			return;
		}
		else if (root.at("operate").as_int64() == S_NEXTSTEP) {
			root.clear();

			iterate = PasswdToMD5(
				password.GetString(),
				data.at("random").as_string().c_str(),
				data.at("challenge").as_int64()
			).c_str();

			root["iterate"] = iterate.GetString();
		}
		else if (root.at("operate").as_int64() == S_NEW_RANDOM) {
			root.clear();

			iterate = PasswdToMD5(
				password.GetString(),
				data.at("random").as_string().c_str(),
				data.at("challenge").as_int64()
			).c_str();

			newIterate = PasswdToMD5(
				password.GetString(), 
				data.at("newRandom").as_string().c_str(),
				data.at("newChallenge").as_int64() + 1
			).c_str();

			root["iterate"] = iterate.GetString();
			root["newIterate"] = newIterate.GetString();
		}

		json.Append(serialize(root).c_str());
		cSocket->Send(json, json.GetLength());

		//使用后清空，预备下次使用
		root.clear();
		data.clear();
		json.Empty();

		ret = cSocket->Receive(recvBuff, 2048);
		if (ret <= 0) {
			MessageBox("连接出错");
		}
		recvBuff[ret] = 0;

		//接收解析结果
		root = parse(recvBuff).as_object();
		data = root.at("data").as_object();
		if (root.at("operate").as_int64() == S_LOGIN_FAILED) {
			MessageBox("账号或密码错误");
			return;
		}
		else {
			//TODO 登陆成功后的处理
			std::string userName(data.at("userName").as_string().c_str());
			//MessageBox(userName.c_str());

			ISEMCourseDesignClientUserDlg *dlg = new ISEMCourseDesignClientUserDlg;
			dlg->Create(IDD_ISEMCOURSEDESIGNCLIENTUSER_DIALOG, this);
			dlg->ShowWindow(SW_SHOW);
			this->ShowWindow(SW_HIDE);
		}

		/*重构前
		char recvBuff[1024 + 1] = { 0 };
		int ret;
		std::string newIterate;
		std::string newIteration;

		CString json;
		object root;
		root["operate"] = C_LOGIN;

		object data;
		data["ID"] = ID.GetString();
		data["password"] = password.GetString();

		root["data"] = data;

		json.Append(serialize(root).c_str());
		//MessageBox(json);
		cSocket.Send(json, json.GetLength());

		ret = cSocket.Receive(recvBuff, 1024);
		if (ret <= 0) {
			MessageBox("连接中断");
		}
		recvBuff[ret] = 0;
		//CString msg(recvBuff);
		//MessageBox(msg);
		object recv;
		recv = parse(recvBuff).as_object();
		object sData = recv.at("data").as_object();
		if (recv.at("operate").as_int64() == S_LOGIN_FAILED)
		{
			MessageBox("账号或密码错误");
			return;
		}
		else if (recv.at("operate").as_int64() == S_NEW_RANDOM) {
			int newChallenge = sData.at("newChallenge").as_int64();
			std::string newRandom = sData.at("newRandom").as_string().c_str();
			newIterate = PasswdToMD5(password.GetString(), newRandom, newChallenge + 1);
		}

		//TODO 将新的随机值和挑战值得出新迭代值，并连带旧的迭代值一起发送至服务器


		int challenge = sData.at("challenge").as_int64();
		std::string random(sData.at("random").as_string().c_str());
		std::string iterate = PasswdToMD5(password.GetString(), random, challenge);
		cSocket.Send(iterate.c_str(), iterate.length());

		ret = cSocket.Receive(recvBuff, 1024);
		if (ret <= 0) {
			MessageBox("连接中断");
		}
		recvBuff[ret] = 0;

		recv.clear();
		sData.clear();

		recv = parse(recvBuff).as_object();
		sData = recv.at("data").as_object();
		if (recv.at("operate").as_int64() == S_LOGIN_FAILED) {
			MessageBox("账号或密码错误");
			return;
		}
		else {
			std::string userName(sData.at("userName").as_string().c_str());
			MessageBox(userName.c_str());
		}
		*/

	}
	else {
		MessageBox("请输入账号和密码");
	}

}


void CISEMCourseDesignClientDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	object root, data;
	root["operate"] = C_LOGOUT;
	root["data"] = data;
	CString json;
	json.Append(serialize(root).c_str());
	cSocket->Send(json, json.GetLength());
	cSocket->Close();
	cSocket = NULL;

	CDialogEx::OnClose();
}


void CISEMCourseDesignClientDlg::OnBnClickedButtonRegister()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (!(userName.IsEmpty() || ID.IsEmpty() || password.IsEmpty() || confPasswd.IsEmpty())) {

		if (!CheckPassword(password)) {
			MessageBox("密码应包含英文字母大小写和数字，不能包含符号");
			return;
		}

		if (password != confPasswd) {
			MessageBox("前后密码不一致");
			return;
		}

		char recvBuff[2048 + 1];
		int ret = 0;
		CString iterate, newIterate;

		object root, data;
		CString json;

		root["operate"] = C_REGISTER;
		data["ID"] = ID.GetString();
		root["data"] = data;

		json.Append(serialize(root).c_str());
		cSocket->Send(json, json.GetLength());

		//使用后清空，预备下次使用
		root.clear();
		data.clear();
		json.Empty();

		ret = cSocket->Receive(recvBuff, 2048);
		if (ret <= 0) {
			MessageBox("连接出错");
		}
		recvBuff[ret] = 0;

		root = parse(recvBuff).as_object();
		data = root.at("data").as_object();
		if (root.at("operate").as_int64() == S_USER_EXIST) {
			MessageBox("该账号已存在");
			return;
		}
		else if (root.at("operate").as_int64() == S_NEXTSTEP) {
			iterate = PasswdToMD5(
				password.GetString(),
				data.at("random").as_string().c_str(),
				data.at("challenge").as_int64() + 1
			).c_str();

			root.clear();
			data.clear();

			root["iterate"] = iterate;
			root["name"] = userName.GetString();
			json.Append(serialize(root).c_str());
			cSocket->Send(json, json.GetLength());

			root.clear();
			json.Empty();

			ret = cSocket->Receive(recvBuff, 2048);
			if (ret <= 0) {
				MessageBox("连接出错");
			}
			recvBuff[ret] = 0;
			
			root = parse(recvBuff).as_object();
			if (root.at("operate").as_int64() == S_REGISTER_SUCCESS) {
				MessageBox("注册成功");
			}
			else if (root.at("operate").as_int64() == S_REGISTER_FAIL) {
				MessageBox("注册失败，请重试");
			}
		}
	}
	else {
		MessageBox("请输入账号和密码");
	}
}


ClientSocket* CISEMCourseDesignClientDlg::GetSocket()
{
	return cSocket;
}