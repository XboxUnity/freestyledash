#pragma once
#include "stdafx.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Debug/Debug.h"
#include "ScnXlinkKaiChat.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/General/KaiManager.h"

// Macros and definitions to handle the chat room update
#define CHAT_UPDATE_INTERVAL	100
#define TM_CHATUPDATE			71
#define CHAT_LOG_OFFSET			4
#define MAX_CHAT_LINES			200
#define CHAT_MAX_MESSAGELEN		512

CScnXlinkKaiChat::~CScnXlinkKaiChat()
{
	//Go Back to arena mode
	KaiManager::getInstance().KaiArenaMode();

	// Clear and stop the chat channel
	ChatManager::getInstance().StopChatChannel();
}

CScnXlinkKaiChat::CScnXlinkKaiChat()
{
	// Initialize variables
	bChatChanged = false;
}


HRESULT CScnXlinkKaiChat::OnNotifyChat( KAIMessageOnChat * pNotifyChat, BOOL& bHandled )
{
	// Trigger the flag to notify our timer that the chat has updated
	bChatChanged = true;
	bHandled=  TRUE;

	return S_OK;
}

HRESULT CScnXlinkKaiChat::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{	
	DebugMsg("CScnXlinkKaiChat", "CScnXlinkKaiChat::OnInit");

	// Initailize our children
	GetChildById( L"PlayerList", &m_PlayerList );
	GetChildById( L"Back", &m_Back );
	GetChildById( L"Chat", &m_ChatLog );
	GetChildById( L"SendMessage", &m_SendMessage );
	GetChildById( L"TextInput", &m_TextInput );

	ChatManager::getInstance().getChatPlayerInfo(&m_pChatPlayers);

	// Init Chat Buffer
	szChatMessage = L"";
	nTotalChatLines = 0;
	szKbBuffer = new WCHAR[CHAT_MAX_MESSAGELEN];
	nMaxVisibleChatLines = m_ChatLog.GetMaxVisibleLineCount();

	ChatManager::getInstance().StartChatChannel();
	SetTimer(TM_CHATUPDATE, CHAT_UPDATE_INTERVAL);

    return S_OK;
}

HRESULT CScnXlinkKaiChat::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if(hObjPressed == m_PlayerList)
	{
		// This function doesn't do anything
		ChatManager::getInstance().getChatPlayerInfo(&m_pChatPlayers);
		int index = m_PlayerList.GetCurSel();
		string player = wstrtostr(m_pChatPlayers.at(index).GetPlayerName());
		DebugMsg("XlinkKaiScn","Player Pressed %s", player.c_str());
		bHandled = TRUE;
	}
	else if(hObjPressed == m_Back)
	{
		// Exit our scene
		NavigateBack(XUSER_INDEX_ANY);
		bHandled = TRUE;
	}
	else if(hObjPressed == m_TextInput)
	{
		// Initailize the keyboard for text input
		memset(&keyboard, 0, sizeof(keyboard));
		memset(szKbBuffer, 0, sizeof(szKbBuffer));

		XShowKeyboardUI(0, VKBD_DEFAULT, szChatMessage.c_str(), L"Send Message", L"Enter a new message",
			szKbBuffer,200,&keyboard);

		SetTimer(TM_KEYBOARD, 50);
		bHandled = TRUE;
	}
	else if(hObjPressed == m_SendMessage)
	{
		ChatManager::getInstance().SendChatMessage( szChatMessage );

		szChatMessage = L"";
		m_TextInput.SetText(L"");

		bHandled = TRUE;
	}

	return S_OK;
}

HRESULT CScnXlinkKaiChat::OnTimer(XUIMessageTimer *pTimer, BOOL& bHandled)
{
    switch(pTimer->nId)
    {
		case TM_CHATUPDATE:
		{
			// Update has triggered- lets do some work
			if( bChatChanged == false ) break;
			// Reset our flag, so it can be triggered again by another thread
			bChatChanged = false;

			// Adds an offset to the ChatLog to keep text from being cut off
			nTotalChatLines = m_ChatLog.GetLineCount() + CHAT_LOG_OFFSET;

			// Move the view downward to stay on current chat
			if( nTotalChatLines > nMaxVisibleChatLines ) {
				m_ChatLog.SetTopLine( nTotalChatLines - nMaxVisibleChatLines, TRUE );
			} else {
				m_ChatLog.SetTopLine( 0, TRUE );
			}

			// Update our chat log's buffer
			m_ChatLog.SetText( ChatManager::getInstance().getChatBuffer().c_str() );
			break;
		}
		case TM_KEYBOARD:
		{
			if(XHasOverlappedIoCompleted(&keyboard))
            {
				KillTimer(TM_KEYBOARD);
				bHandled = TRUE;
				if(keyboard.dwExtendedError == ERROR_SUCCESS)
                {
					szChatMessage = szKbBuffer;
					m_TextInput.SetText(szKbBuffer);
				}				
			}
			break;
		}
    }
    return S_OK;
}