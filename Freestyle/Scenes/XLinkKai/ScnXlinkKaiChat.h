#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/Chat/ChatManager.h"

using namespace std;

class CScnXlinkKaiChat : public CXuiSceneImpl
{
private:
	XOVERLAPPED keyboard;
	

protected:
	CXuiList m_PlayerList;
	CXuiControl m_Back, m_SendMessage;
	CXuiEdit m_ChatLog, m_TextInput;

	ChatPlayer m_pChatPlayers;

	bool bChatChanged;
	WCHAR * szKbBuffer;
	int nTotalChatLines, nMaxVisibleChatLines;

	wstring szChatMessage;

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		KAI_ON_NOTIFY_CHAT( OnNotifyChat )
	XUI_END_MSG_MAP()

    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnTimer(XUIMessageTimer *pTimer, BOOL& bHandled);
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );

	// KaiMessage Callback Methods
	HRESULT OnNotifyChat( KAIMessageOnChat * pNotifyChat, BOOL& bHandled );

public:
	CScnXlinkKaiChat();
	~CScnXlinkKaiChat();

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnXlinkKaiChat, L"ScnXlinkKaiChat", XUI_CLASS_TABSCENE )
};
