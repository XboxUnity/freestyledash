#include "stdafx.h"

#include "ScnXlinkSignIn.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/GameContent/GameContentMessages.h"
#include "../../Tools/XlinkKai/General/KaiManager.h"
#include "../../Tools/XlinkKai/KaiNotifyManager.h"

// Strings used for the status messages
const WCHAR * szEngineSearch = L"Searching for Xlink Engine...";
const WCHAR * szEngineSearchTimeout = L"Xlink Engine not found on network...";
const WCHAR * szEngineTakeover = L"Negotiating control for Xlink Engine...";
const WCHAR * szEngineAttached = L"System attached to Xlink Engine...";
const WCHAR * szEngineLoggingOn = L"Connecting to Xlink Orbital Servers...";
const WCHAR * szEngineLoggedOn = L"Logged into Xlink Kai..";
const WCHAR * szEngineWrongPassword = L"Invalid credentials, please correct.";
const WCHAR * szEngineNotLoggedIn = L"Xlink Engine not connected to Server.";
const WCHAR * szEngineRetrieveInfo = L"Username/Password resolved from Engine.";


CScnXlinkSignIn::CScnXlinkSignIn()
{
	//InitializeCriticalSection(&lock);
	SceneXuiState.bRememberInfo = false;
	SceneXuiState.bTimedOut = false;
	SceneXuiState.bSceneNested = false;
	SceneXuiState.bUsingStored = false;
	SceneXuiState.nConnectionState = (ClientState)0;
	bDestroying = false;
}

CScnXlinkSignIn::~CScnXlinkSignIn()
{
	if(SceneXuiState.buffer != NULL )
		delete [] SceneXuiState.buffer;
	//DeleteCriticalSection(&lock);
	DebugMsg("CScnXlinkSignIn","Destroyed");
}

HRESULT CScnXlinkSignIn::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	DebugMsg("CScnXlinkSignIn", "OnInit");

	// Create instances of the Xui Controls
	InitializeChildren();

	SceneXuiState.buffer = new WCHAR[512];
	SceneXuiState.bUsingStored = true;

	// Initialize state of local variables and xui elements
	if(SceneXuiControlState.hasButtonConnect)
		SceneXuiControls.m_ButtonConnect.SetEnable(FALSE);

	// Initialize status message
	SetKaiStatus(szEngineSearch);

	// Initialize Kai Client / UDP Threads - start timeout timer
	SceneXuiState.nConnectionState = Discovering;
	KaiManager::getInstance().KaiAttachToClient();
	SetTimer(TM_TIMEOUT, KAI_TIMEOUT);
	
	SceneXuiState.bRememberInfo = SETTINGS::getInstance().getKaiSettingsRemember() == 0 ? false : true;
	if(SceneXuiControlState.hasCheckRemember) {
		SceneXuiControls.m_CheckRemember.SetCheck(SETTINGS::getInstance().getKaiSettingsRemember());	
	}
	
	SceneXuiState.bUsingStored = SETTINGS::getInstance().getKaiSettingsAutoLogin() == 0 ? false : true;
	if(SceneXuiControlState.hasCheckUseStored) {
		SceneXuiControls.m_CheckUseStored.SetCheck(SETTINGS::getInstance().getKaiSettingsAutoLogin());
	}

	SceneXuiState.szUsername = SETTINGS::getInstance().getKaiSettingsUsername();
	SceneXuiState.szPassword = SETTINGS::getInstance().getKaiSettingsPassword();
	
	SetUsernamePasswordText(SceneXuiState.bUsingStored);

	if(SceneXuiState.bUsingStored) {
		SceneXuiControls.m_TextUsername.SetEnable(FALSE);
		SceneXuiControls.m_TextPassword.SetEnable(FALSE);
	} else {
		SceneXuiControls.m_TextUsername.SetEnable(TRUE);
		SceneXuiControls.m_TextPassword.SetEnable(TRUE);
	}
	
	return S_OK;
}

HRESULT CScnXlinkSignIn::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if(hObjPressed == SceneXuiControls.m_ButtonConnect)
	{
		DebugMsg("CScnXlinkSignIn", "Connect Button pressed...");
		SetKaiStatus(szEngineLoggingOn);
		SceneXuiControls.m_ButtonConnect.SetEnable(FALSE);

		KaiManager::getInstance().QueryClientState();
		SceneXuiState.nConnectionState = Querying;
		bHandled = true;

	
	}
	if(hObjPressed == SceneXuiControls.m_CheckRemember)
	{
		SETTINGS::getInstance().setKaiSettingsRemember(!SETTINGS::getInstance().getKaiSettingsRemember());
		SceneXuiControls.m_CheckUseStored.SetCheck(SETTINGS::getInstance().getKaiSettingsRemember());

		bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_CheckUseStored)
	{
		SETTINGS::getInstance().setKaiSettingsAutoLogin(!SETTINGS::getInstance().getKaiSettingsAutoLogin());
		BOOL bSetting = SETTINGS::getInstance().getKaiSettingsAutoLogin();
		SceneXuiControls.m_CheckUseStored.SetCheck(bSetting);
		SetUsernamePasswordText(bSetting);

		bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_TextUsername)
	{
		memset(&SceneXuiState.keyboard, 0, sizeof(SceneXuiState.keyboard));
		memset(SceneXuiState.buffer, 0, sizeof(SceneXuiState.buffer));

		temp = strtowstr(SceneXuiState.szUsername);

		XShowKeyboardUI(0,VKBD_DEFAULT,temp.c_str(),L"Xlink Kai Username",L"Enter username", SceneXuiState.buffer, 32, &SceneXuiState.keyboard);
		SetTimer(TM_KEYBOARD, 50);

		SceneXuiState.nInputType = 1;
		bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_TextPassword)
	{
		memset(&SceneXuiState.keyboard, 0, sizeof(SceneXuiState.keyboard));
		memset(SceneXuiState.buffer, 0, sizeof(SceneXuiState.buffer));

		temp = strtowstr(SceneXuiState.szPassword);

		XShowKeyboardUI(0,VKBD_DEFAULT,temp.c_str(),L"Xlink Kai Username",L"Enter username", SceneXuiState.buffer, 32, &SceneXuiState.keyboard);
		SetTimer(TM_KEYBOARD, 50);

		SceneXuiState.nInputType = 2;
		bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_ButtonBack)
	{
		if(SceneXuiState.nConnectionState != Authenticated) {
			KaiManager::getInstance().ShutdownKai();
		}

		NavigateBack(XUSER_INDEX_ANY);
		bHandled = true;
		return S_OK;
	}
	if(hObjPressed == SceneXuiControls.m_CheckUseStored)
	{
		if(SceneXuiControls.m_CheckUseStored.IsChecked())
		{
			SceneXuiState.bUsingStored = TRUE;
		}
		else
		{
			SceneXuiState.bUsingStored = FALSE;
		}

		bHandled = true;
	}

    return S_OK;
}

HRESULT CScnXlinkSignIn::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
    // which timer is it?
    switch( pTimer->nId )
    {
		case TM_TIMEOUT:
		{
			KillTimer(TM_TIMEOUT);
			DebugMsg("CScnXlinkSignIn", "Xlink Engine could not be found on the network, shutting down");
			SceneXuiState.bTimedOut = true;
			SetKaiStatus(szEngineSearchTimeout);
			KaiManager::getInstance().ShutdownKai();
			SceneXuiState.nConnectionState = Disconnected;
			break;
		}
		case TM_KEYBOARD:
		{
			if(XHasOverlappedIoCompleted(&SceneXuiState.keyboard))
            {
				KillTimer(TM_KEYBOARD);
				bHandled = true;
				if(SceneXuiState.keyboard.dwExtendedError == ERROR_SUCCESS)
                {
					if(SceneXuiState.nInputType == 1)
					{
						SceneXuiState.szUsername = wstrtostr(SceneXuiState.buffer);
						SetUsernamePasswordText( SceneXuiState.bUsingStored );
						if(SceneXuiState.bRememberInfo) 
						{
							SETTINGS::getInstance().setKaiSettingsUsername(SceneXuiState.szUsername);
						}
					}
					else if(SceneXuiState.nInputType == 2)
					{
						SceneXuiState.szPassword = wstrtostr(SceneXuiState.buffer);
						SetUsernamePasswordText( SceneXuiState.bUsingStored );
						if(SceneXuiState.bRememberInfo) 
						{
							SETTINGS::getInstance().setKaiSettingsPassword(SceneXuiState.szPassword);
						}
					}
				}				
			}
			break;
		}
		case TM_GOBACKTRIGGER:
		{
			KillTimer(TM_GOBACKTRIGGER);
			//Join Arena Mode
			bHandled = TRUE;
			return NavigateBack(XUSER_INDEX_ANY);
			//DebugMsg("ScnXlinkSignIn", "Command to leave Scene complete");
			//break;
		}
    }
    
    bHandled = TRUE;
	return S_OK;
}

HRESULT CScnXlinkSignIn::OnNotifyEngineAttached( KAIMessageOnEngineAttached * pOnEngineAttached, BOOL& bHandled )
{
	if(bDestroying == true) return S_FALSE;
	//EnterCriticalSection(&lock);
	// Client received attached message from xlink engine
	KillTimer(TM_TIMEOUT);
	SceneXuiState.bTimedOut = false;

	// Update status
	DebugMsg("CScnXlinkSignIn", "Attached successfully to the xlink engine...");
	SetKaiStatus(szEngineAttached);

	// Enable the appropriate controls
	SceneXuiControls.m_ButtonConnect.SetEnable(TRUE);
	SceneXuiState.nConnectionState = Attaching;
	bHandled = true;
	//LeaveCriticalSection(&lock);
	return S_OK;
}

HRESULT CScnXlinkSignIn::OnNotifyEngineInUse( KAIMessageOnEngineInUse * pOnEngineInUse, BOOL& bHandled )
{
	if(bDestroying == true) return S_FALSE;
	//EnterCriticalSection(&lock);
	// Reset timeout timer- 
	KillTimer(TM_TIMEOUT);
	SceneXuiState.bTimedOut = false;
	SetTimer(TM_TIMEOUT, KAI_TIMEOUT);

	// Update status
	DebugMsg("CScnXlinkSignIn", "Xlink Engine in use, attmepting to takeover...");
	SetKaiStatus(szEngineTakeover);
	bHandled = TRUE;
	//LeaveCriticalSection(&lock);
	return S_OK;
}

HRESULT CScnXlinkSignIn::OnNotifyAccountLoggedIn( KAIMessageOnAccountLoggedIn * pOnAccountLoggedIn, BOOL& bHandled )
{
	if(bDestroying == true) return S_FALSE;
	//EnterCriticalSection(&lock);
	DebugMsg("CScnXlinkSignIn", "Xlink Engine Logged in and accepting messages...");
	SetKaiStatus(szEngineLoggedOn);
	SceneXuiState.nConnectionState = Authenticated;

	SceneXuiControls.m_ButtonConnect.SetEnable(FALSE);

	//Sleep(100);
	CKaiNotifyManager::getInstance().NotifyXlinkKaiOnline("Username Here");
	DebugMsg("ScnXlinkSignIn", "Time To Enter Arena Mode");
	KaiManager::getInstance().KaiLeaveChatmode();
	Sleep(150);
	KaiManager::getInstance().KaiArenaMode();
	DebugMsg("ScnXlinkSignIn", "Messages Sent for Enter Arena Mode");
	
	//NavigateBack(XUSER_INDEX_ANY);
	bDestroying = true;
	Sleep(50);
	SetTimer(TM_GOBACKTRIGGER, 100);
	//LeaveCriticalSection(&lock);
	bHandled = TRUE;

	return S_OK;
}

HRESULT CScnXlinkSignIn::OnNotifyAuthenticationFailed( KAIMessageOnAuthenticationFailed * pOnAuthenticationFailed, BOOL& bHandled )
{
	if(bDestroying == true) return S_FALSE;
	//EnterCriticalSection(&lock);
	DebugMsg("CScnXlinkSignIn", "Authentication Failed");
	KaiManager::getInstance().ShutdownKai();
	
	// Initialize status message
	SetKaiStatus(szEngineSearch);

	// Initialize Kai Client / UDP Threads - start timeout timer
	SceneXuiState.nConnectionState = Discovering;
	KaiManager::getInstance().KaiAttachToClient();
	SetTimer(TM_TIMEOUT, KAI_TIMEOUT);
	//LeaveCriticalSection(&lock);
	return S_OK;
}

HRESULT CScnXlinkSignIn::OnNotifyAccountNotLoggedIn( KAIMessageOnAccountNotLoggedIn * pOnAccountNotLoggedIn, BOOL& bHandled )
{
	if(bDestroying == true) return S_FALSE;
	//EnterCriticalSection(&lock);
	bool bUseStored = SceneXuiState.bUsingStored;
	string szUsername = "";
	string szPassword = "";

	if(bUseStored) {
		szUsername = pOnAccountNotLoggedIn->szUsername;
		szPassword = pOnAccountNotLoggedIn->szPassword;

		KaiManager::getInstance().LoginToKai(szUsername, szPassword);
	}
	else {
		if(SceneXuiControlState.hasTextUsername)
			szUsername = SceneXuiState.szUsername;

		if(SceneXuiControlState.hasTextPassword)
			szPassword = SceneXuiState.szPassword;

		if(szUsername != "" && szPassword != "")
			KaiManager::getInstance().LoginToKai(szUsername, szPassword);
	}
	//LeaveCriticalSection(&lock);
	return S_OK;
}

HRESULT CScnXlinkSignIn::SetKaiStatus(const WCHAR * szStatusText)
{
	if(SceneXuiControlState.hasKaiStatus){
		SceneXuiControls.m_KaiStatus.SetText(szStatusText);
	}
	return S_OK;
}

HRESULT CScnXlinkSignIn::SetUsernamePasswordText( BOOL bSetting )
{
	if(SceneXuiControlState.hasTextUsername) {
		if(bSetting == TRUE)
			SceneXuiControls.m_TextUsername.SetText(L"Using Xlink Engine Username");
		else 
		{
			if( strcmp(SceneXuiState.szUsername.c_str(), "") == 0 )
				SceneXuiControls.m_TextUsername.SetText(L"Enter Username..");
			else
				SceneXuiControls.m_TextUsername.SetText(strtowstr(SceneXuiState.szUsername).c_str());
		}

		SceneXuiControls.m_TextUsername.SetEnable(!bSetting);
	}
	if(SceneXuiControlState.hasTextPassword) {
		if(bSetting == TRUE)
			SceneXuiControls.m_TextPassword.SetText(L"Using Xlink Engine Password");
		else
		{
			if( strcmp( SceneXuiState.szPassword.c_str(), "") == 0 )
				SceneXuiControls.m_TextPassword.SetText(L"Enter Password..");
			else
				SceneXuiControls.m_TextPassword.SetText(strtowstr(GetPasswordMask(SceneXuiState.szPassword.length(), "*")).c_str());
		}

		SceneXuiControls.m_TextPassword.SetEnable(!bSetting);
	}

	return S_OK;
}

HRESULT CScnXlinkSignIn::InitializeChildren( void )
{
	HRESULT hr = NULL;

	hr = GetChildById(L"XlinkSubScene", &SceneXuiControls.m_XlinkSubScene);
	SceneXuiControlState.hasXlinkSubScene = SceneXuiState.bSceneNested = hr == S_OK;

	hr = GetChildById(L"ButtonBack", &SceneXuiControls.m_ButtonBack);
	SceneXuiControlState.hasButtonBack = hr == S_OK;
	
	if(SceneXuiState.bSceneNested) {
		hr = SceneXuiControls.m_XlinkSubScene.GetChildById(L"KaiStatusText", &SceneXuiControls.m_KaiStatus);
		SceneXuiControlState.hasKaiStatus = hr == S_OK;

		hr = SceneXuiControls.m_XlinkSubScene.GetChildById(L"ButtonConnect", &SceneXuiControls.m_ButtonConnect);
		SceneXuiControlState.hasButtonConnect = hr == S_OK;

		hr = SceneXuiControls.m_XlinkSubScene.GetChildById(L"ButtonChangeUsername", &SceneXuiControls.m_TextUsername);
		SceneXuiControlState.hasTextUsername = hr == S_OK;

		hr = SceneXuiControls.m_XlinkSubScene.GetChildById(L"ButtonChangePassword", &SceneXuiControls.m_TextPassword);
		SceneXuiControlState.hasTextPassword = hr == S_OK;

		hr = SceneXuiControls.m_XlinkSubScene.GetChildById(L"UseStoredLoginInfo", &SceneXuiControls.m_CheckUseStored);
		SceneXuiControlState.hasCheckUseStored = hr == S_OK;

		hr = SceneXuiControls.m_XlinkSubScene.GetChildById(L"RememberLoginInfo", &SceneXuiControls.m_CheckRemember);
		SceneXuiControlState.hasCheckRemember = hr == S_OK;
	}
	else
	{
		hr = GetChildById(L"KaiStatusText", &SceneXuiControls.m_KaiStatus);
		SceneXuiControlState.hasKaiStatus = hr == S_OK;

		hr = GetChildById(L"ButtonConnect", &SceneXuiControls.m_ButtonConnect);
		SceneXuiControlState.hasButtonConnect = hr == S_OK;

		hr = GetChildById(L"ButtonChangeUsername", &SceneXuiControls.m_TextUsername);
		SceneXuiControlState.hasTextUsername = hr == S_OK;

		hr = GetChildById(L"ButtonChangePassword", &SceneXuiControls.m_TextPassword);
		SceneXuiControlState.hasTextPassword = hr == S_OK;

		hr = GetChildById(L"UseStoredLoginInfo", &SceneXuiControls.m_CheckUseStored);
		SceneXuiControlState.hasCheckUseStored = hr == S_OK;

		hr = GetChildById(L"RememberLoginInfo", &SceneXuiControls.m_CheckRemember);
		SceneXuiControlState.hasCheckRemember = hr == S_OK;
	}

	return S_OK;
}	