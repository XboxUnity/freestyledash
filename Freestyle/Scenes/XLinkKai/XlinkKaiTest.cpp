#pragma once
#include "stdafx.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Debug/Debug.h"
#include "XlinkKaiTest.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/KaiClient.h"

HRESULT CScnXlinkKaiTest::OnNotifyChat( KAIMessageOnChat * pNotifyChat, BOOL& bHandled )
{
	DebugMsg("CScnXlinkKaiTest", "Chat Received By XUI~  Current Room:  %s", pNotifyChat->szVector.c_str());
	DebugMsg("CScnXlinkKaiTest", "%s%s:    %s", pNotifyChat->bPrivate == true ? "[PRIV]" : "", pNotifyChat->szOpponent.c_str(), pNotifyChat->szMessage.c_str());

	//Not a private message
	if(pNotifyChat->bPrivate == false)
	{
		szChatBuffer.append(pNotifyChat->szOpponent);
		szChatBuffer.append(" : ");
		szChatBuffer.append(pNotifyChat->szMessage);
		szChatBuffer.append("\n");
	}

	//Add a 2 line buffer to the end to keep text from getting cut off
	chatLines = m_Chat.GetLineCount() + 3;

	//Move view down to keep current chat visible
    (chatLines > maxChatLines)? m_Chat.SetTopLine(chatLines - maxChatLines, TRUE): m_Chat.SetTopLine(0, TRUE);

	m_Chat.SetText(strtowchar(szChatBuffer));

	bHandled = true;
	return S_OK;
}

HRESULT CScnXlinkKaiTest::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{	
	DebugMsg("ScnXlinkKaiTest","CScnXlinkKaiTest");

	GetChildById( L"ArenaList", &m_ArenaList );
	GetChildById( L"PlayerList", &m_PlayerList );
	GetChildById( L"Back", &m_Back );
	GetChildById( L"Chat", &m_Chat);
	GetChildById( L"CurrentArena", &m_CurrentArena );
	GetChildById( L"ModeToggle", &m_ModeToggle );
	GetChildById( L"ToggleChat", &m_ToggleChat );

	ArenaManager::getInstance().GetArenaInfo(&m_pArenaInfo);

	chatEnabled = false;
	inArenaMode = true;

	// Init Chat Buffer
	szChatBuffer = "";
	chatLines = 0;
	maxChatLines = m_Chat.GetMaxVisibleLineCount();

	DebugMsg("XlinkKai","Max visible line count %d", maxChatLines);

    return S_OK;
}

HRESULT CScnXlinkKaiTest::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if(hObjPressed == m_ArenaList)
	{
		int index = m_ArenaList.GetCurSel();

		ArenaManager::getInstance().GetArenaInfo(&m_pArenaInfo);

		string szArena = m_pArenaInfo.m_subArenas.at(index).m_arenaVector;

		DebugMsg("XlinkKaiTest","New Arena Path %s", szArena.c_str());

		string pass = "";
		CKaiClient::getInstance().JoinArena(szArena, pass, true);

		bHandled = TRUE;
	}
	else if(hObjPressed == m_PlayerList)
	{
		ArenaManager::getInstance().GetArenaInfo(&m_pArenaInfo);

		int index = m_PlayerList.GetCurSel();

		string player = wstrtostr(m_pArenaInfo.m_player.at(index)->GetPlayerName());

		DebugMsg("XlinkKaiScn","Player Pressed %s", player.c_str());
		bHandled = TRUE;
	}
	else if(hObjPressed == m_ModeToggle)
	{
		if(!inArenaMode)
		{
			CKaiClient::getInstance().ArenaMode();
			m_ModeToggle.SetText(L"Chat Mode");

			inArenaMode = true;
		}
		else
		{
			CKaiClient::getInstance().MessengerMode();
			m_ModeToggle.SetText(L"Arena Mode");

			inArenaMode = false;
		}
		bHandled = TRUE;
	}
	else if(hObjPressed == m_ToggleChat)
	{
		if(!chatEnabled)
		{
			CKaiClient::getInstance().EnableChatMode();
			m_ToggleChat.SetText(L"Disable Chat");

			chatEnabled = true;
		}
		else
		{
			CKaiClient::getInstance().EndChatMode();
			m_ToggleChat.SetText(L"Enable Chat");

			chatEnabled = false;
		}
		bHandled = TRUE;
	}
	else if(hObjPressed == m_Back)
	{
		ArenaManager::getInstance().GetArenaInfo(&m_pArenaInfo);

		if(strcmp(m_pArenaInfo.m_arenaName.c_str(),"Arena") == 0)
		{
			NavigateBack(XUSER_INDEX_ANY);
		}
		else
		{
			CKaiClient::getInstance().ExitArena();
		}

		bHandled = TRUE;
	}

	return S_OK;
}

HRESULT CScnXlinkKaiTest::OnEnterArena(KAIMessageOnEnterArena * pData, BOOL& bHandled)
{
	m_CurrentArena.SetText(strtowstr(pData->szVector).c_str());

	//Reset Buffer for new Channel
	szChatBuffer = "";
	m_Chat.SetTopLine(0, TRUE);
	m_Chat.SetText(strtowchar(szChatBuffer));

	bHandled = TRUE;

	return S_OK;
}