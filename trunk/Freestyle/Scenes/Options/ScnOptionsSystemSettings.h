#pragma once

#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Generic/tools.h"

class CScnOptionsSystemSettings : CXuiSceneImpl
{
public:
	
	// Date/Time Setting Controls
	CXuiList m_DateFormat, m_TimeFormat;
	
	CXuiControl m_FTPUser, m_FTPPass, m_SetFormat;
	CXuiControl m_HTTPUser, m_HTTPPass;
	// Overscan Controls
	CXuiSlider m_VerticalOverscan;
	CXuiSlider m_HorizontalOverscan;
	CXuiTextElement m_VertOverscanText;
	CXuiTextElement m_HorizOverscanText;
	CXuiCheckbox m_EnableLetterbox, m_FTPEnable, m_HTTPEnable;
	CXuiCheckbox m_HTTPAuth;

	// Fan Speed Controls
	CXuiSlider m_FanSpeed;
	CXuiTextElement m_FanSpeedText;

	// Implement Class
    XUI_IMPLEMENT_CLASS( CScnOptionsSystemSettings, L"ScnOptionsSystemSettings", XUI_CLASS_SCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_NOTIFY_VALUE_CHANGED( OnNotifyValueChanged )
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
	XUI_END_MSG_MAP()

	// Constructor / Destructor
	CScnOptionsSystemSettings() {}
	~CScnOptionsSystemSettings() {}

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled );

	HRESULT InitializeChildren( void );
	HRESULT ReadUserSettings( void );
	HRESULT UpdateDateAndTimeLists( void );

private:
	// Private Variables
	vector<string> m_TimeMap;
	int m_szSelectedTime;
	int m_szCurrentTime;

	vector<string> m_DateMap;
	int m_szSelectedDate;
	int m_szCurrentDate;

	XOVERLAPPED keyboard;
	WCHAR * buffer;

	int UserOrPass;  // 1 ftp user, 2 ftp pass, 3 http user, 4 http pass

	wstring FTPUser;
	wstring FTPPass;

	wstring HTTPUser;
	wstring HTTPPass;
};