#pragma once

#include "../../Tools/Generic/Tools.h"
#include "../../Tools/Debug/Debug.h"

struct Cred {
//	LPWSTR title;
	string login;
	string password;
	string smbPath;
	string share;
	HXUIOBJ focusItem;

	Cred() {};
	virtual ~Cred() {
//		delete title;
	};
};

class ScnGetCredentials : public CXuiSceneImpl
{
private:
	Cred* cred;
	bool bSaveToDb;
	XOVERLAPPED overlapped;
	WCHAR result[1024];
	WCHAR MessageText[1024];
	bool bIsPasswordEdit;
	wstring temp;
	string user;
	string password;

protected:
	CXuiControl m_back;
	CXuiTextElement m_title;
	CXuiEdit m_description;
	CXuiControl m_login;
	CXuiControl m_password;
	CXuiControl m_testBtn;
	CXuiTextElement m_testResults;
	CXuiCheckbox m_saveCheck;
	CXuiControl m_doneBtn;

	CXuiScene m_ChildScene;

	bool isNested;

	HRESULT InitializeChildren();

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );

public :
	ScnGetCredentials() : bSaveToDb(false) {};	
	XUI_IMPLEMENT_CLASS( ScnGetCredentials, L"ScnGetCredentials", XUI_CLASS_SCENE )

};

HRESULT GetCredentials(string smbPath, string login, string password, string share);
