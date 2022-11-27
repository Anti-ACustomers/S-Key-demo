#pragma once
#include <afxsock.h>

class CISEMCourseDesignClientDlg;

class ClientSocket :
    public CSocket
{
public:
    CISEMCourseDesignClientDlg* pDlg;

public:
    ClientSocket();
    ~ClientSocket();
    void SetDialog(CISEMCourseDesignClientDlg* p);
    virtual void OnClose(int nErrorCode);
    virtual void OnReceive(int nErrorCode);
    virtual void OnSend(int nErrorCode);
};

