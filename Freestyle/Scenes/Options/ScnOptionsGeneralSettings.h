#pragma once

#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Generic/tools.h"

#define KBMODE_NONE				0x00
#define KBMODE_USERNAME			0x01
#define KBMODE_PASSWORD			0x02
#define KBMODE_SMBWORKGROUP		0x03
#define KBMODE_HOSTNAME			0x04

#define MSGMODE_NONE			0x10
#define MSGMODE_CLEARDATA		0x11
#define MSGMODE_DEFAULTSETTINGS 0x12
#define MSGMODE_RESETSKIN		0x13

class CScnOptionsGeneralSettings : CXuiSceneImpl
{
public:

	// General Setting Controls
	CXuiCheckbox m_EnableAvatars;
	CXuiCheckbox m_DisplayCelsius;
	CXuiCheckbox m_EnableNTPAtStart;
	CXuiCheckbox m_EnableUpdateNotices;
	CXuiCheckbox m_EnableBetaNotices;
	CXuiCheckbox m_ShowStatus;
	CXuiCheckbox m_EnableSmbClient;
	CXuiCheckbox m_EnableSmbServer;
	CXuiCheckbox m_EnableAdvancedMode;
	CXuiControl m_SmbClientWorkgroup;
	CXuiControl m_Hostname;


	// TeamFSD.com Credentials
	CXuiControl m_WebUsername;
	CXuiControl m_WebPassword;
	CXuiCheckbox m_LinkConsole;

	//  General Buttons
	CXuiControl m_CheckForUpdates;
	CXuiControl m_ClearData;
	CXuiControl m_DefaultSettings;
	CXuiControl m_ResetSkin;

	// Implement Class
    XUI_IMPLEMENT_CLASS( CScnOptionsGeneralSettings, L"ScnOptionsGeneralSettings", XUI_CLASS_SCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_MSG_RETURN( OnMsgReturn )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
	XUI_END_MSG_MAP()

	// Constructor / Destructor
	CScnOptionsGeneralSettings();
	~CScnOptionsGeneralSettings();

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
    HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnMsgReturn(XUIMessageMessageBoxReturn *pXUIMessageMessageBoxReturn, BOOL &bHandled);
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );

	HRESULT InitializeChildren( void );
	HRESULT ReadUserSettings( void );

private:

	// Strings for display in keyboard UI and messagebox UI
	wstring m_szKeyboardUsernameCaption;
	wstring m_szKeyboardUsernamePrompt;
	wstring m_szKeyboardPasswordCaption;
	wstring m_szKeyboardPasswordPrompt;
	wstring m_szMsgboxWarningCaption;
	wstring m_szMsgboxClearDataPrompt;
	wstring m_szMsgboxDefaultSettingsPrompt;
	wstring m_szMsgboxResetSkin;
	wstring m_szWorkgroupCaption;
	wstring m_szWorkgroupPrompt;
	wstring m_szHostnameCaption;
	wstring m_szHostnamePrompt;

	// Variables to hold the username and password for manipulation
	string m_szWebUsername;
	string m_szWebPassword;
	string m_szSambaClientWorkgroup;
	string m_szHostname;

	// Variables and Buffers for our KeyboardUI
	wstring m_szTemp;
	DWORD m_dwKeyboardMode;
	LPWSTR m_kbBuffer;
	XOVERLAPPED m_kbOverlapped;

	// Variables and Buffers for our MessageBoxUI
	LPCWSTR * m_msgButtons;
	WAIT_INFO m_WaitInfo;
	DWORD m_dwMsgBoxMode;

};