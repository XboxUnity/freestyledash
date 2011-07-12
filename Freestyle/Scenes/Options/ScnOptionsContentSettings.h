#pragma once

#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Generic/tools.h"

#define KBMODE_NONE				0x00
#define KBMODE_USERNAME			0x01
#define KBMODE_PASSWORD			0x02
#define KBMODE_SMBWORKGROUP		0x03
#define KBMODE_HOSTNAME			0x04

class CScnOptionsContentSettings : CXuiSceneImpl
{
public:

	// Marketplace Download Options
	CXuiCheckbox m_DownloadIcons;
	CXuiCheckbox m_DownloadBoxart;
	CXuiCheckbox m_DownloadBanners;
	CXuiCheckbox m_DownloadScreenshots;
	CXuiCheckbox m_DownloadBackgrounds;
	CXuiCheckbox m_DownloadVideos;

	CXuiSlider m_ScreenshotSlider;

	CXuiCheckbox m_AutoResumeAssets;
	CXuiCheckbox m_DisableAutoScan;
	CXuiControl m_StartManualScan;

	CXuiTextElement m_ScreenshotCountText;

	// Game List Setting Controls
	CXuiCheckbox m_UseBackgrounds;
	CXuiCheckbox m_UseScreenshots;

	// Game List Visual Controls
	CXuiList m_GameListVisual;
	CXuiList m_GameListTransition;

	// Cover Downloading controls
	CXuiCheckbox m_CoverDownload;
	CXuiSlider m_CoverPoll;
	CXuiControl m_CoverUser;
	CXuiControl m_CoverPass;
	CXuiTextElement m_CurrentPoll;
	CXuiCheckbox m_CoverLowRes;
	CXuiCheckbox m_CoverMedRes;
	CXuiCheckbox m_CoverHighRes;


	// Marketplace Locale
	CXuiTextElement m_CurrentLocale;
	CXuiList m_LocaleList;
	CXuiControl m_SetLocale;

	// General Buttons
	CXuiControl m_ManagePaths;

	// Implement Class
    XUI_IMPLEMENT_CLASS( CScnOptionsContentSettings, L"ScnOptionsContentSettings", XUI_CLASS_SCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_TIMER( OnTimer )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_NOTIFY_VALUE_CHANGED( OnNotifyValueChanged )
	XUI_END_MSG_MAP()

	// Constructor / Destructor
	CScnOptionsContentSettings();
	~CScnOptionsContentSettings();

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
    HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled );

	HRESULT InitializeChildren( void );
	HRESULT ReadUserSettings( void );

private:
	// Variables to hold the list of valid marketplace locales, with their codes
	map<string, string> m_LocaleMap;
	string m_szSelectedLocale;
	string m_szCurrentLocale;
	string m_szCurrentLocaleCode;

	// Strings for display in keyboard UI and messagebox UI
	wstring m_szKeyboardUsernameCaption;
	wstring m_szKeyboardUsernamePrompt;
	wstring m_szKeyboardPasswordCaption;
	wstring m_szKeyboardPasswordPrompt;
	
	// Variables to hold the username and password for manipulation
	string m_szUsername;
	string m_szPassword;
	
	// Variables and Buffers for our KeyboardUI
	wstring m_szTemp;
	DWORD m_dwKeyboardMode;
	LPWSTR m_kbBuffer;
	XOVERLAPPED m_kbOverlapped;
};