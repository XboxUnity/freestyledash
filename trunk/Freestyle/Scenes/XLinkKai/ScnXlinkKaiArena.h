#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/Arena/ArenaManager.h"
#include "../../Tools/ContentList/ContentManager.h"
#include "../../Tools/GameContent/GameContentManager.h"

using namespace std;

#define MAX_CHAT_LINES 200

// Action Type enum
typedef enum ACTION_TYPE { ACTION_TYPE_LAUNCH, ACTION_TYPE_HOST, ACTION_TYPE_HOSTING, ACTION_TYPE_UNKNOWN };

enum PlayerLists 
{
	OpponentList,
	BuddyList,
	ListSize,
};

class CScnXlinkKaiArena : public CXuiSceneImpl
{
private:
	XOVERLAPPED keyboard;
	WCHAR * buffer;
	WCHAR * szKbBuffer;
	LPCWSTR* btnOk;
	LPCWSTR* btnJoin;
	LPCWSTR* btnReply;

	wstring temp;
	bool inMsgBox;

	string m_szGameArenaName;
	string m_szSelectedArena;
	string m_szArenaPassword;
	DWORD m_dwKeyboardType;
	bool m_bChatEnabled;
	bool m_bInMsgBox;
	bool m_bJoinGame;
	bool m_bInsideGame;
	bool m_bInUserRoom;
	bool m_bEnterArenaMsg;
	bool m_bEnterArenaFailedMsg;

	// Private scene variables
	bool m_bFromGamelist;
	ACTION_TYPE m_nActionFunc;
	// GameContentManager Variables
	GAMECONTENT_LIST_STATE listPack;
	GAMECONTENT_LIST_STATE snapShot;

	// Message box variables
	LPCWSTR* szBtnOk;
	LPCWSTR* szBtnJoin;
	LPCWSTR* szBtnReply;


	wstring szDescription;
	wstring szTitle;

private:
	
	// Method to quickly change the action button
	HRESULT SetActionInterface( ACTION_TYPE nActionFunc );

	// Timer Functions to handle Kai Events
	HRESULT handleEnterArena( void );
	HRESULT handleEnterArenaFailed( void );
	HRESULT handleNotifyChat( void );
	HRESULT handleNotifyPrivateMsg( void );
	HRESULT handleNotifyInvite( void );

	// Messages that we pass to our timer
	KAIMessageOnChatPrivateMessage * pNotifyPrivateMsg;
	KAIMessageOnChat * pNotifyChat;
	KAIMessageOnContactInvite * pNotifyInvite;
	KAIMessageOnEnterArena * pEnterArena;
	KAIMessageOnEnterArenaFailed * pEnterArenaFailed;

protected:
	CXuiList m_ArenaList, m_PlayerList, m_BuddyList;

	CXuiControl m_ListFocus;
	CXuiControl m_Back, m_SendMessage, m_ToggleChat, m_Action, m_PrevList, m_NextList;
	CXuiEdit m_Chat, m_TextInput;
	CXuiTextElement m_CurrentArena, m_CurrentPlayerList;

	bool chatEnabled, fromGameList, joinGame;
	bool bChatChanged;
	int chatLines, maxChatLines, keyboardType, listType, messageboxType;
	ARENAITEM m_pArenaInfo;

	string gameArenaName;

	string szChatBuffer1, chatMessage, arenaPassword, szSelectedArena, player, arena;
	wstring szChatMessage;
	wstring szChatBuffer;
	int nTotalChatLines;
	int nMaxVisibleChatLines;

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_NOTIFY_SET_FOCUS( OnNotifySetFocus )
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_MSG_RETURN( OnMsgReturn )

		KAI_ON_NOTIFY_CONTACT_INVITE( OnNotifyContactInvite )
		KAI_ON_NOTIFY_CHAT_PRIVATE_MESSAGE( OnNotifyChatPrivateMessage )
		KAI_ON_NOTIFY_CHAT( OnNotifyChat )
		KAI_ON_NOTIFY_ENTER_ARENA( OnEnterArena )
		KAI_ON_NOTIFY_ENTER_ARENA_FAILED( OnEnterArenaFailed )
	XUI_END_MSG_MAP()

    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnNotifySetFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled );
	HRESULT OnMsgReturn(XUIMessageMessageBoxReturn *pMsgBox, BOOL &bHandled);

	// KaiMessage Callback Methods
	HRESULT OnNotifyContactInvite(KAIMessageOnContactInvite * pData, BOOL& bHandled);
	HRESULT OnNotifyChat( KAIMessageOnChat * pNotifyChat, BOOL& bHandled );
	HRESULT OnEnterArena(KAIMessageOnEnterArena * pData, BOOL& bHandled);
	HRESULT OnEnterArenaFailed(KAIMessageOnEnterArenaFailed * pData, BOOL& bHandled);
	HRESULT OnNotifyChatPrivateMessage(KAIMessageOnChatPrivateMessage * pData, BOOL& bHandled);

	HRESULT SendMessage(string buffer);
	void LoadList(int listType);

public:

	CScnXlinkKaiArena();
	~CScnXlinkKaiArena();

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnXlinkKaiArena, L"ScnXlinkKaiArena", XUI_CLASS_TABSCENE )
};
