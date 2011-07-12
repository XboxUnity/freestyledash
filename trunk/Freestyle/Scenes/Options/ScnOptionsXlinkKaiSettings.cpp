#include "stdafx.h"
#include "ScnOptionsXlinkKaiSettings.h"

#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Settings/Settings.h"

CScnOptionsXlinkKaiSettings::CScnOptionsXlinkKaiSettings()
{
	// Initialize our strings here
	m_szKeyboardCaption = L"Xlink Kai Server IP Address";
	m_szKeyboardPrompt = L"Please enter the IP Address for the machine that Xlink Kai Engine is located on.  Please note that this is used if IP autodetection ";
}

CScnOptionsXlinkKaiSettings::~CScnOptionsXlinkKaiSettings()
{
	// Clean up our allocated memory here
	// Clean up our allocated memory
	if( m_kbBuffer )
		delete [] m_kbBuffer;
}

HRESULT CScnOptionsXlinkKaiSettings::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{	
	// First, let's initialize all of our xui controls
	InitializeChildren();

	// After initialization, let's fill the controls with stored setting values
	ReadUserSettings();

	// Setup our Buffers and Variables for Keyboard UI
	m_kbBuffer = new WCHAR[512];

	return S_OK;
}

HRESULT CScnOptionsXlinkKaiSettings::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	switch (pTimer->nId) 
	{
	case TM_KEYBOARD:
		if(XHasOverlappedIoCompleted(&m_kbOverlapped)) {
			bHandled = TRUE;
			KillTimer(TM_KEYBOARD);

			if(m_kbOverlapped.dwExtendedError == ERROR_SUCCESS) {
				bool bValidDBEntry = true;
				string szServerIp = wstrtostr(m_kbBuffer);
				// If the entered server ip address is not valid, revert to stored ip value
				if(strcmp(make_lowercaseA(szServerIp).c_str(), "undefined") == 0) {
					m_szServerIp = szServerIp;
					SETTINGS::getInstance().setKaiSettingsIP(m_szServerIp);
			
				} else if( strcmp(make_lowercaseA(szServerIp).c_str(), "") == 0) {
					m_szServerIp = "Undefined";
					SETTINGS::getInstance().setKaiSettingsIP(m_szServerIp);

				} else if( IsValidIPAddressString(szServerIp) == false) {
					szServerIp = m_szServerIp;
					XNotifyQueueUICustom(L"Invalid IP Address Entered... Reverting back");
					bValidDBEntry = false;

				} else {
					m_szServerIp = szServerIp;
					SETTINGS::getInstance().setKaiSettingsIP(m_szServerIp);
				}

				if(bValidDBEntry == true) {
					m_ServerIP.SetText(strtowstr(m_szServerIp).c_str());
				}
			}
		}
		break;
	default:
		break;
	};

	// Return successfully
	return S_OK;
}

HRESULT CScnOptionsXlinkKaiSettings::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if( hObjPressed == m_EngineLogin ) {
		SETTINGS::getInstance().setKaiSettingsAutoLogin(1 - SETTINGS::getInstance().getKaiSettingsAutoLogin());
		m_EngineLogin.SetCheck(SETTINGS::getInstance().getKaiSettingsAutoLogin());
		bHandled = TRUE;

	} else if( hObjPressed == m_LinkInfo ) {
		SETTINGS::getInstance().setKaiSettingsRemember(1 - SETTINGS::getInstance().getKaiSettingsRemember());
		m_LinkInfo.SetCheck(SETTINGS::getInstance().getKaiSettingsRemember());
		bHandled = TRUE;

	} else if( hObjPressed == m_EditServerIP ) {
		memset(&m_kbOverlapped, 0, sizeof(m_kbOverlapped));
		memset(m_kbBuffer, 0, sizeof(m_kbBuffer));

		// Convert our string to a wstring
		m_szTemp = strtowstr(m_szServerIp);

		// Set Variables to display our Keyboard
		XShowKeyboardUI(0, VKBD_DEFAULT, m_szTemp.c_str(), m_szKeyboardCaption.c_str(), m_szKeyboardPrompt.c_str(),
			m_kbBuffer, 32, &m_kbOverlapped);
		SetTimer(TM_KEYBOARD, 50);
		bHandled = TRUE;

	} else if( hObjPressed == m_NotifyContactOnline ) {
		SETTINGS::getInstance().setKaiContactOnlineNotice(1 - SETTINGS::getInstance().getKaiContactOnlineNotice());
		m_NotifyContactOnline.SetCheck(SETTINGS::getInstance().getKaiContactOnlineNotice());
		bHandled = TRUE;

	} else if( hObjPressed == m_NotifyContactOffline ) {
		SETTINGS::getInstance().setKaiContactOfflineNotice(1 - SETTINGS::getInstance().getKaiContactOfflineNotice());
		m_NotifyContactOffline.SetCheck(SETTINGS::getInstance().getKaiContactOfflineNotice());
		bHandled = TRUE;

	} else if( hObjPressed == m_NotifyInviteToGame ) {
		SETTINGS::getInstance().setKaiInviteToGameNotice(1 - SETTINGS::getInstance().getKaiInviteToGameNotice());
		m_NotifyInviteToGame.SetCheck(SETTINGS::getInstance().getKaiInviteToGameNotice());
		bHandled = TRUE;

	} else if( hObjPressed == m_NotifyContactPM ) {
		SETTINGS::getInstance().setKaiContactPMNotice(1 - SETTINGS::getInstance().getKaiContactPMNotice());
		m_NotifyContactPM.SetCheck(SETTINGS::getInstance().getKaiContactPMNotice());
		bHandled = TRUE;

	} else if( hObjPressed == m_NotifyOpponentPM ) {
		SETTINGS::getInstance().setKaiOpponentPMNotice(1 - SETTINGS::getInstance().getKaiOpponentPMNotice());
		m_NotifyOpponentPM.SetCheck(SETTINGS::getInstance().getKaiOpponentPMNotice());
		bHandled = TRUE;

	} else {
		// Unknown control pressed.

	}
	
	// Return successfully
	return S_OK;
}

HRESULT CScnOptionsXlinkKaiSettings::ReadUserSettings( void )
{
	// Initalize login Setting controls
	m_EngineLogin.SetCheck(SETTINGS::getInstance().getKaiSettingsAutoLogin());
	m_LinkInfo.SetCheck(SETTINGS::getInstance().getKaiSettingsRemember());
	
	// Initialize Server IP controls
	string szServerIp = SETTINGS::getInstance().getKaiSettingsIP();
	if(strcmp(szServerIp.c_str(), "") == 0) SETTINGS::getInstance().setKaiSettingsIP("Undefined");
	if(strcmp(szServerIp.c_str(), "Undefined") != 0) {
		m_szServerIp = szServerIp;
		m_ServerIP.SetText(strtowstr(m_szServerIp).c_str());
	}

	// Initialize Notice Controls
	m_NotifyContactOnline.SetCheck(SETTINGS::getInstance().getKaiContactOnlineNotice());
	m_NotifyContactOffline.SetCheck(SETTINGS::getInstance().getKaiContactOfflineNotice());
	m_NotifyInviteToGame.SetCheck(SETTINGS::getInstance().getKaiInviteToGameNotice());
	m_NotifyContactPM.SetCheck(SETTINGS::getInstance().getKaiContactPMNotice());
	m_NotifyOpponentPM.SetCheck(SETTINGS::getInstance().getKaiOpponentPMNotice());

	// Set initial state for other stuff here
	// *** Future

	return S_OK;
}

HRESULT CScnOptionsXlinkKaiSettings::InitializeChildren( void )
{	

	// Login Setting Controls
	GetChildById(L"EngineLogin", &m_EngineLogin);
	GetChildById(L"LinkInfo", &m_LinkInfo);

	// Server Ip Controls
	GetChildById(L"ServerIP", &m_ServerIP);
	GetChildById(L"EditServerIP", &m_EditServerIP);

	// Notification Controls
	GetChildById(L"NotifyContactOnline", &m_NotifyContactOnline);
	GetChildById(L"NotifyContactOffline", &m_NotifyContactOffline);
	GetChildById(L"NotifyInviteToGame", &m_NotifyInviteToGame);
	GetChildById(L"NotifyContactPM", &m_NotifyContactPM);
	GetChildById(L"NotifyOpponentPM", &m_NotifyOpponentPM);

	// Return Successfully
	return S_OK;
}