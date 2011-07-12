#pragma once

#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Generic/tools.h"

class CScnOptionsXlinkKaiSettings : CXuiSceneImpl
{
public:

	// Login Setting Controls
	CXuiCheckbox m_EngineLogin;
	CXuiCheckbox m_LinkInfo;

	// Server Ip Controls
	CXuiTextElement m_ServerIP;
	CXuiControl m_EditServerIP;

	// Notification Controls
	CXuiCheckbox m_NotifyContactOnline;
	CXuiCheckbox m_NotifyContactOffline;
	CXuiCheckbox m_NotifyInviteToGame;
	CXuiCheckbox m_NotifyContactPM;
	CXuiCheckbox m_NotifyOpponentPM;

	// Implement Class
    XUI_IMPLEMENT_CLASS( CScnOptionsXlinkKaiSettings, L"ScnOptionsXlinkKaiSettings", XUI_CLASS_SCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_TIMER( OnTimer )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
	XUI_END_MSG_MAP()

	// Constructor / Destructor
	CScnOptionsXlinkKaiSettings();
	~CScnOptionsXlinkKaiSettings();

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );

	HRESULT InitializeChildren( void );
	HRESULT ReadUserSettings( void );

private:
	// Strings for display in keyboard UI
	wstring m_szKeyboardCaption;
	wstring m_szKeyboardPrompt;

	// Variable to hold server ip pulled from settings for comparison
	string m_szServerIp;

	// Variables and Buffers for our KeyboardUI
	wstring m_szTemp;
	LPWSTR m_kbBuffer;
	XOVERLAPPED m_kbOverlapped;
};