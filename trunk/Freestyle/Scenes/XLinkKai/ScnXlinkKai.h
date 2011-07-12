#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/XlinkKai/KaiMessage.h"

using namespace std;

class CScnXlinkKai : public CXuiSceneImpl
{
protected:
    CXuiControl m_btnSignIn, m_btnPlayOnline, m_btnContacts, m_btnChat, m_btnSettings, m_btnArena;

	CRITICAL_SECTION lock;
	bool m_bIsLoggedIn;

	CScnXlinkKai() { InitializeCriticalSection(&lock); }
	~CScnXlinkKai() { DeleteCriticalSection(&lock); }

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_TIMER( OnTimer )
		// Handle Kai Specific Messages / Notifications
		KAI_ON_NOTIFY_ACCOUNT_LOGGED_IN( OnNotifyAccountLoggedIn )
		KAI_ON_NOTIFY_DEINITIALISE( OnNotifyDeinitialise )
		
	XUI_END_MSG_MAP()

	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnNotifyAccountLoggedIn( KAIMessageOnAccountLoggedIn * pOnAccountLoggedIn, BOOL& bHandled );
	HRESULT OnNotifyDeinitialise( KAIMessageOnDeinitialise * pOnDeinitialise, BOOL& bHandled );

public:


    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnXlinkKai, L"ScnXlinkKai", XUI_CLASS_TABSCENE )
};
