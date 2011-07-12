#include "stdafx.h"
#include "ScnOptionsGeneralSettings.h"

#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/SQLite/FSDSql.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Managers/SambaClient/SambaClient.h"
#include "../../Tools/Managers/SambaServer/SambaServer.h"


CScnOptionsGeneralSettings::CScnOptionsGeneralSettings()
{
	// Initialize our strings
	m_szKeyboardUsernameCaption = L"TeamFSD.com Username";
	m_szKeyboardUsernamePrompt = L"Enter your teamfsd.com username...";
	m_szKeyboardPasswordCaption = L"TeamFSD.com Password";
	m_szKeyboardPasswordPrompt = L"Enter your teamfsd.com password...";
	m_szMsgboxWarningCaption = L"Warning";
	m_szMsgboxClearDataPrompt = L"This will remove all parsed game data. Are you sure you want to continue?";
	m_szMsgboxDefaultSettingsPrompt = L"This will reset all of your settings and restore them to default. Are you sure you want to continue?";
	m_szMsgboxResetSkin = L"This will remove any skin customize settings. Are you sure you want to continue?";
	m_szWorkgroupCaption = L"Samba Client Workgroup";
	m_szWorkgroupPrompt = L"Enter the Samba workgroup...";
	m_szHostnameCaption = L"This XBOX's Hostname";
	m_szHostnamePrompt = L"Enter the Hostname...";

}

CScnOptionsGeneralSettings::~CScnOptionsGeneralSettings()
{
	// Clean up our allocated memory
	if( m_kbBuffer )
		delete [] m_kbBuffer;

	if( m_msgButtons )
		delete [] m_msgButtons;

}

HRESULT CScnOptionsGeneralSettings::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	// First, let's initialize all of our xui controls
	InitializeChildren();

	// After initialization, let's fill the controls with stored setting values
	ReadUserSettings();

	// Setup our Buffers and Variables for Keyboard UI
	m_kbBuffer = new WCHAR[512];
	m_dwKeyboardMode = KBMODE_NONE;

	// Setup our Buffers and Variables for Messagebox UI
	m_msgButtons = new LPCWSTR[2];
	m_msgButtons[0] = L"OK"; m_msgButtons[1] = L"Cancel";
	m_dwMsgBoxMode = MSGMODE_NONE;

	if(SkinManager::getInstance().isCurrentCompressed())
	{
		m_ResetSkin.SetShow(true);
	}
	else
	{
		m_ResetSkin.SetShow(false);
	}

	// We are initialized, let's return successfully
	return S_OK;
}

HRESULT CScnOptionsGeneralSettings::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	switch (pTimer->nId) 
	{
	case TM_KEYBOARD:
		if(XHasOverlappedIoCompleted(&m_kbOverlapped)) {
			bHandled = TRUE;
			KillTimer(TM_KEYBOARD);

			if(m_kbOverlapped.dwExtendedError == ERROR_SUCCESS) {
				if(m_dwKeyboardMode == KBMODE_USERNAME) {
					m_dwKeyboardMode = KBMODE_NONE;
					m_WebUsername.SetText(m_kbBuffer);
					string m_szWebUsername = wstrtostr(m_kbBuffer);
					SETTINGS::getInstance().setFsdUsername(m_szWebUsername);

				} else if(m_dwKeyboardMode == KBMODE_PASSWORD) {
					m_dwKeyboardMode = KBMODE_NONE;
					string m_szWebPassword = wstrtostr(m_kbBuffer);
					m_WebPassword.SetText(strtowstr(GetPasswordMask(m_szWebPassword.length(), "*")).c_str());					
					SETTINGS::getInstance().setFsdPassword(m_szWebPassword);

				} else if(m_dwKeyboardMode == KBMODE_SMBWORKGROUP) {
					m_dwKeyboardMode = KBMODE_NONE;
					m_SmbClientWorkgroup.SetText(m_kbBuffer);
					string workgroup = wstrtostr(m_kbBuffer);
					SETTINGS::getInstance().setSambaClientWorkgroup(workgroup);
					SambaClient::getInstance().SetSambaWorkgroup(workgroup);
				} else if(m_dwKeyboardMode == KBMODE_HOSTNAME) {
					m_dwKeyboardMode = KBMODE_NONE;
					m_Hostname.SetText(m_kbBuffer);
					string hostname = wstrtostr(m_kbBuffer);
					SETTINGS::getInstance().setHostname(hostname);
					SambaServer::setHostname(hostname);
				} else {
					// Unhandled Keyboard Mode
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

HRESULT CScnOptionsGeneralSettings::OnMsgReturn(XUIMessageMessageBoxReturn *pXUIMessageMessageBoxReturn, BOOL &bHandled)
{
	switch( pXUIMessageMessageBoxReturn->nButton )
    {
	case 0:
		if(m_dwMsgBoxMode == MSGMODE_CLEARDATA) {
			m_dwMsgBoxMode = MSGMODE_NONE;
			FSDSql::getInstance().closeDatabase();
			_unlink("game:\\Data\\Databases\\fsd2data.db");
			if(FileExistsA("game:\\Data\\GameData")) {
				FileOperationManager::getInstance().AddFolderOperation("game:\\Data\\GameData", "", true);

				m_WaitInfo.title = L"Deleting Game Data";
				m_WaitInfo.type = 1;
				m_WaitInfo.reboot = true;

				SkinManager::getInstance().setScene("wait.xur", *this, true, "ScnWait", &m_WaitInfo);
			}
			break;
		} else if(m_dwMsgBoxMode == MSGMODE_DEFAULTSETTINGS) {
			m_dwMsgBoxMode = MSGMODE_NONE;
			FSDSql::getInstance().closeDatabase();
			_unlink("game:\\Data\\Databases\\fsd2settings.db");
			Restart();
			break;
		} else if(m_dwMsgBoxMode == MSGMODE_RESETSKIN) {

			if(FileExistsA(SkinManager::getInstance().getCurrentxmlPath())) {
				FileOperationManager::getInstance().AddFolderOperation(SkinManager::getInstance().getCurrentxmlPath(), "", true);

				m_WaitInfo.title = L"Deleting Skin Data";
				m_WaitInfo.type = 1;
				m_WaitInfo.reboot = true;

				SkinManager::getInstance().setScene("wait.xur", *this, true, "ScnWait", &m_WaitInfo);
			}
			break;
		}
	default:
		break;
	};

	// Return Successfully
	bHandled = TRUE;
	return S_OK;
}

HRESULT CScnOptionsGeneralSettings::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	// We've recieved a button press notification
	if( hObjPressed == m_EnableAvatars ) {
		SETTINGS::getInstance().setShowAvatar(1 - SETTINGS::getInstance().getShowAvatar());
		m_EnableAvatars.SetCheck(SETTINGS::getInstance().getShowAvatar());
		bHandled = TRUE;

	} else if( hObjPressed == m_EnableSmbClient ) {
		SETTINGS::getInstance().setSambaClientOn(!SETTINGS::getInstance().getSambaClientOn());
		m_EnableSmbClient.SetCheck(SETTINGS::getInstance().getSambaClientOn());

		if (m_EnableSmbClient.IsChecked() || m_EnableSmbServer.IsChecked()) 
			m_SmbClientWorkgroup.SetEnable(true);
		else
			m_SmbClientWorkgroup.SetEnable(false);

		bHandled = TRUE;
	} else if( hObjPressed == m_EnableAdvancedMode ) {
		SETTINGS::getInstance().setFMAdvancedMode( !SETTINGS::getInstance().getFMAdvancedMode());
		m_EnableAdvancedMode.SetCheck(SETTINGS::getInstance().getFMAdvancedMode());
		bHandled = TRUE;
	} else if( hObjPressed == m_EnableSmbServer ) {
		SETTINGS::getInstance().setSambaServerOn(!SETTINGS::getInstance().getSambaServerOn());
		m_EnableSmbServer.SetCheck(SETTINGS::getInstance().getSambaServerOn());

		if (m_EnableSmbServer.IsChecked() || m_EnableSmbClient.IsChecked()) 
			m_SmbClientWorkgroup.SetEnable(true);
		else
			m_SmbClientWorkgroup.SetEnable(false);

		if (m_EnableSmbServer.IsChecked()) 
			m_Hostname.SetEnable(true);
		else
			m_Hostname.SetEnable(false);

		bHandled = TRUE;

	} else if( hObjPressed == m_SmbClientWorkgroup ) {
		memset(&m_kbOverlapped, 0, sizeof(m_kbOverlapped));
		memset(m_kbBuffer, 0, sizeof(m_kbBuffer));

		// Convert our string to a wstring
		m_szTemp = strtowstr(m_szSambaClientWorkgroup);

		// Set Variables to display our Keyboard
		m_dwKeyboardMode = KBMODE_SMBWORKGROUP;
		XShowKeyboardUI(0, VKBD_DEFAULT, m_szTemp.c_str(), m_szWorkgroupCaption.c_str(), m_szWorkgroupPrompt.c_str(),
			m_kbBuffer, 32, &m_kbOverlapped);
		SetTimer(TM_KEYBOARD, 50);
		bHandled = TRUE;

	} else if( hObjPressed == m_Hostname ) {
		memset(&m_kbOverlapped, 0, sizeof(m_kbOverlapped));
		memset(m_kbBuffer, 0, sizeof(m_kbBuffer));

		// Convert our string to a wstring
		m_szTemp = strtowstr(m_szHostname);

		// Set Variables to display our Keyboard
		m_dwKeyboardMode = KBMODE_HOSTNAME;
		XShowKeyboardUI(0, VKBD_DEFAULT, m_szTemp.c_str(), m_szHostnameCaption.c_str(), m_szHostnamePrompt.c_str(),
			m_kbBuffer, 32, &m_kbOverlapped);
		SetTimer(TM_KEYBOARD, 50);
		bHandled = TRUE;

	} else if( hObjPressed == m_DisplayCelsius ) {
		SETTINGS::getInstance().setCels(1 - SETTINGS::getInstance().getCels());
		m_DisplayCelsius.SetCheck(SETTINGS::getInstance().getCels());
		bHandled = TRUE;
	
	} else if( hObjPressed == m_EnableNTPAtStart ) {
		SETTINGS::getInstance().setNTPonStartup(1 - SETTINGS::getInstance().getNTPonStartup());
		m_EnableNTPAtStart.SetCheck(SETTINGS::getInstance().getNTPonStartup());
		bHandled = TRUE;
	
	} else if( hObjPressed == m_EnableUpdateNotices ) {
		SETTINGS::getInstance().setUpdateXexNotice(1 - SETTINGS::getInstance().getUpdateXexNotice());		
		SETTINGS::getInstance().setUpdateSkinNotice(1 - SETTINGS::getInstance().getUpdateSkinNotice());
		SETTINGS::getInstance().setUpdatePluginNotice(1 - SETTINGS::getInstance().getUpdatePluginNotice());
		m_EnableUpdateNotices.SetCheck(SETTINGS::getInstance().getUpdateXexNotice());
		bHandled = TRUE;
	
	} else if( hObjPressed == m_EnableBetaNotices ) {
		SETTINGS::getInstance().setUpdateOnBeta(1 - SETTINGS::getInstance().getUpdateOnBeta());
		m_EnableBetaNotices.SetCheck(SETTINGS::getInstance().getUpdateOnBeta());
		bHandled = TRUE;
	
	} else if( hObjPressed == m_WebUsername ) {
		memset(&m_kbOverlapped, 0, sizeof(m_kbOverlapped));
		memset(m_kbBuffer, 0, sizeof(m_kbBuffer));

		// Convert our string to a wstring
		m_szTemp = strtowstr(m_szWebUsername);

		// Set Variables to display our Keyboard
		m_dwKeyboardMode = KBMODE_USERNAME;
		XShowKeyboardUI(0, VKBD_DEFAULT, m_szTemp.c_str(), m_szKeyboardUsernameCaption.c_str(), m_szKeyboardUsernamePrompt.c_str(),
			m_kbBuffer, 32, &m_kbOverlapped);
		SetTimer(TM_KEYBOARD, 50);
		bHandled = TRUE;

	} else if( hObjPressed == m_WebPassword ) {
		memset(&m_kbOverlapped, 0, sizeof(m_kbOverlapped));
		memset(m_kbBuffer, 0, sizeof(m_kbBuffer));

		// Convert our string to a wstring
		m_szTemp = strtowstr(m_szWebPassword);

		// Set Variables to display our Keyboard
		m_dwKeyboardMode = KBMODE_PASSWORD;
		XShowKeyboardUI(0, VKBD_DEFAULT, m_szTemp.c_str(), m_szKeyboardPasswordCaption.c_str(), m_szKeyboardPasswordPrompt.c_str(),
			m_kbBuffer, 32, &m_kbOverlapped);
		SetTimer(TM_KEYBOARD, 50);
		bHandled = TRUE;

	} else if( hObjPressed == m_LinkConsole ) {
		SETTINGS::getInstance().setFsdUseLogin(1 - SETTINGS::getInstance().getFsdUseLogin());
		m_LinkConsole.SetCheck(SETTINGS::getInstance().getFsdUseLogin());
		bHandled = TRUE;

	} else if( hObjPressed == m_CheckForUpdates ) {
		// Handled by XUITool (ignored)

	} else if( hObjPressed == m_ClearData ) {
		m_dwMsgBoxMode = MSGMODE_CLEARDATA;
		ShowMessageBoxEx(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(), m_szMsgboxWarningCaption.c_str(), m_szMsgboxClearDataPrompt.c_str(),
			2, m_msgButtons, 1, NULL, NULL);

	} else if( hObjPressed == m_DefaultSettings ) {
		m_dwMsgBoxMode = MSGMODE_DEFAULTSETTINGS;
		ShowMessageBoxEx(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(), m_szMsgboxWarningCaption.c_str(), m_szMsgboxDefaultSettingsPrompt.c_str(),
			2, m_msgButtons, 1, NULL, NULL);

	} else if( hObjPressed == m_ShowStatus ) {
		SETTINGS::getInstance().setShowStatusAtBoot(1 - SETTINGS::getInstance().getShowStatusAtBoot());
		m_ShowStatus.SetCheck(SETTINGS::getInstance().getShowStatusAtBoot());
		bHandled = TRUE;

	} else if(hObjPressed == m_ResetSkin) {
		if(SkinManager::getInstance().isCurrentCompressed())
		{
			m_dwMsgBoxMode = MSGMODE_RESETSKIN;
			ShowMessageBoxEx(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(), m_szMsgboxWarningCaption.c_str(), m_szMsgboxResetSkin.c_str(),
				2, m_msgButtons, 1, NULL, NULL);
		}
	} else {
		// Undefined control was pressed
	}

	// Return successfully
	return S_OK;
}

HRESULT CScnOptionsGeneralSettings::ReadUserSettings( void )
{
	// Set initial state of General Setting controls
	m_EnableAdvancedMode.SetCheck(SETTINGS::getInstance().getFMAdvancedMode());
	m_EnableAvatars.SetCheck(SETTINGS::getInstance().getShowAvatar());
	m_DisplayCelsius.SetCheck(SETTINGS::getInstance().getCels());
	m_EnableNTPAtStart.SetCheck(SETTINGS::getInstance().getNTPonStartup());
	m_EnableUpdateNotices.SetCheck(SETTINGS::getInstance().getUpdateXexNotice() || 
		SETTINGS::getInstance().getUpdateSkinNotice() || SETTINGS::getInstance().getUpdatePluginNotice());
	m_EnableBetaNotices.SetCheck(SETTINGS::getInstance().getUpdateOnBeta());
	m_ShowStatus.SetCheck(SETTINGS::getInstance().getShowStatusAtBoot());
	m_EnableSmbClient.SetCheck(SETTINGS::getInstance().getSambaClientOn());
	m_szSambaClientWorkgroup = SETTINGS::getInstance().getSambaClientWorkgroup();
	m_SmbClientWorkgroup.SetText(strtowstr(m_szSambaClientWorkgroup).c_str());

	m_EnableSmbServer.SetCheck(SETTINGS::getInstance().getSambaServerOn());
	m_szHostname = SETTINGS::getInstance().getHostname();
	m_Hostname.SetText(strtowstr(m_szHostname).c_str());

	if (!m_EnableSmbClient.IsChecked() && !m_EnableSmbServer.IsChecked()) {
		m_SmbClientWorkgroup.SetEnable(false);
	}

	if (!m_EnableSmbServer.IsChecked()) {
		m_Hostname.SetEnable(false);
	}

	// Set initial state of TeamFSD.com credential controls
	m_szWebUsername = SETTINGS::getInstance().getFsdUsername();
	m_szWebUsername = m_szWebUsername == "" ? "Username Here" : m_szWebUsername;
	m_WebUsername.SetText(strtowstr(m_szWebUsername).c_str());
	
	m_szWebPassword = SETTINGS::getInstance().getFsdPassword();
	m_szWebPassword = m_szWebPassword == "" ? "Password Here" : GetPasswordMask(m_szWebPassword.length(), "*");
	m_WebPassword.SetText(strtowstr(m_szWebPassword).c_str());

	m_LinkConsole.SetCheck(SETTINGS::getInstance().getFsdUseLogin());

	// Set initial state for other stuff here
	// *** Future

	return S_OK;
}

HRESULT CScnOptionsGeneralSettings::InitializeChildren( void )
{	
	// Initialize General Settings Controls
	GetChildById(L"EnableAvatars", &m_EnableAvatars);
	GetChildById(L"DisplayCelsius", &m_DisplayCelsius);
	GetChildById(L"NTPAtStart", &m_EnableNTPAtStart);
	GetChildById(L"EnableUpdateNotices", &m_EnableUpdateNotices);
	GetChildById(L"EnableBetaUpdates", &m_EnableBetaNotices);
	GetChildById(L"ShowStatusBar", &m_ShowStatus );
	GetChildById(L"EnableSmbClient", &m_EnableSmbClient);
	GetChildById(L"Workgroup", &m_SmbClientWorkgroup);
	GetChildById(L"EnableAdvancedMode", &m_EnableAdvancedMode);

	GetChildById(L"EnableSmbServer", &m_EnableSmbServer);
	GetChildById(L"Hostname", &m_Hostname);

	// Initialize TeamFSD.com Crediential Controls
	GetChildById(L"Username", &m_WebUsername);
	GetChildById(L"Password", &m_WebPassword);
	GetChildById(L"LinkConsole", &m_LinkConsole);

	// Initilaize General Button Controls
	GetChildById(L"CheckForUpdates", &m_CheckForUpdates);
	GetChildById(L"ResetData", &m_ClearData);
	GetChildById(L"DefaultSettings", &m_DefaultSettings);
	GetChildById(L"ResetSkin", &m_ResetSkin);

	return S_OK;
}