#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/General/KaiManager.h"

using namespace std;

//Time to search for kai server
#define KAI_TIMEOUT		10000

class CScnXlinkSignIn : public CXuiSceneImpl
{
protected:

	struct {
		CXuiScene m_XlinkSubScene;
		CXuiControl m_KaiStatus;
		CXuiControl m_ButtonBack;
		CXuiControl m_ButtonConnect;
		CXuiControl m_TextUsername;
		CXuiControl m_TextPassword;
		CXuiCheckbox m_CheckRemember;
		CXuiCheckbox m_CheckUseStored;
	} SceneXuiControls;

	struct {
		bool hasXlinkSubScene; bool hasKaiStatus; bool hasButtonBack; bool hasButtonConnect; 
		bool hasTextUsername; bool hasTextPassword; bool hasCheckRemember; bool hasCheckUseStored;
	} SceneXuiControlState;
	
	struct {
		bool bTimedOut;
		bool bSceneNested;
		bool bUsingStored;
		bool bRememberInfo;
		ClientState nConnectionState;
		XOVERLAPPED keyboard;
		WCHAR * buffer;
		int nInputType;
		string szPassword;
		string szUsername;
	} SceneXuiState;

	wstring temp;
	bool bDestroying;

	string PasswordMask(int size);
	HRESULT InitializeChildren( void );
	HRESULT SetKaiStatus(const WCHAR * szStatusText);
	HRESULT SetUsernamePasswordText( BOOL bSetting );

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_TIMER( OnTimer )
		// Handle Kai Specific Messages / Notifications
		KAI_ON_NOTIFY_ENGINE_ATTACHED( OnNotifyEngineAttached )
		KAI_ON_NOTIFY_ENGINE_INUSE( OnNotifyEngineInUse )
		KAI_ON_NOTIFY_ACCOUNT_LOGGED_IN( OnNotifyAccountLoggedIn )
		KAI_ON_NOTIFY_ACCOUNT_NOT_LOGGED_IN( OnNotifyAccountNotLoggedIn )
		KAI_ON_NOTIFY_AUTHENTICATION_FAILED( OnNotifyAuthenticationFailed )
	XUI_END_MSG_MAP()

    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	// Kai Specific Messages and Notifications
	HRESULT OnNotifyEngineAttached( KAIMessageOnEngineAttached * pOnEngineAttached, BOOL& bHandled );
	HRESULT OnNotifyEngineInUse( KAIMessageOnEngineInUse * pOnEngineInUse, BOOL& bHandled );
	HRESULT OnNotifyAccountLoggedIn( KAIMessageOnAccountLoggedIn * pOnAccountLoggedIn, BOOL& bHandled );
	HRESULT OnNotifyAccountNotLoggedIn( KAIMessageOnAccountNotLoggedIn * pOnAccountNotLoggedIn, BOOL& bHandled );
	HRESULT OnNotifyAuthenticationFailed( KAIMessageOnAuthenticationFailed * pOnAuthenticationFailed, BOOL& bHandled );


	//CRITICAL_SECTION lock;

public:

	// Constructor / Descontructor
	CScnXlinkSignIn();
	~CScnXlinkSignIn();

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnXlinkSignIn, L"ScnXlinkSignIn", XUI_CLASS_TABSCENE )
};
