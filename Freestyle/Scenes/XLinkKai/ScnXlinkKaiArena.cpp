#pragma once
#include "stdafx.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "ScnXlinkKaiArena.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/General/KaiManager.h"
#include "ScnXlinkKaiPlayerInfo.h"

// Timers to handle the scene changes required
#define TM_ENTERARENA				80
#define TM_ENTERARENAFAILED			81
#define TM_NOTIFYCHAT				82
#define TM_NOTIFYPRIVATEMSG			83
#define TM_NOTIFYINVITE				84

// Update intervals for the various timers
#define KAIMSG_UPDATE_INTERVAL		10

#define CHAT_LOG_OFFSET			4
#define MAX_CHAT_LINES			200
#define CHAT_MAX_MESSAGELEN		512

CScnXlinkKaiArena::CScnXlinkKaiArena()
{
	// Initialize scene specific variables
	m_bFromGamelist = false;
	bChatChanged = false;
	m_nActionFunc = ACTION_TYPE_UNKNOWN;

	// Allocate memory for message box buttons
	szBtnOk = new LPCWSTR[1];
	szBtnJoin = new LPCWSTR[2];
	szBtnReply = new LPCWSTR[2];

	// Keyboard buffers
	szKbBuffer = new WCHAR[512];

	// Initlaize message box buttons
	szBtnOk[0] = L"Okay";
	szBtnJoin[0] = L"Join";  szBtnJoin[1] = L"Cancel";
	szBtnReply[0] = L"Reply";  szBtnReply[1] = L"Cancel";

}

CScnXlinkKaiArena::~CScnXlinkKaiArena()
{
	ArenaManager::getInstance().StopChatChannel();
	
	// Kill any timers we started
	KillTimer(TM_ENTERARENA);
	KillTimer(TM_NOTIFYCHAT);

	// Free memory associate with our message box buttons
	if( szBtnOk != NULL ) delete [] szBtnOk;
	if( szBtnJoin != NULL ) delete [] szBtnJoin;
	if( szBtnReply != NULL ) delete [] szBtnReply;

}

HRESULT CScnXlinkKaiArena::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{	
	DebugMsg("CScnXlinkKaiArena","CScnXlinkKaiArena::OnInit");

	// First thing we want to do is determine where this scene is being launched from
	if( pInitData->pvInitData != NULL ) {
		// This scene was launched from an alternate scene
		VECTOR_INFO * pVector = (VECTOR_INFO *)pInitData->pvInitData;
		m_bFromGamelist = pVector->launchGame;
	}

	// Now let's initialize our scene according to our init data
	if( m_bFromGamelist == true ) {
		// This launch came from the game list
		snapShot = GameContentManager::getInstance().getGameListSnapshot(false);

		// Grab the vector path for the current game
		string szVectorPath;
		ContentKaiVector::getInstance().GetVectorByTitleId(snapShot.CurrentGame->ContentRef->getTitleId(), szVectorPath);

		// Store current arena name for navigate back
		int nDelimiter = szVectorPath.rfind('/');

		// Extract the game's arena name
		m_szGameArenaName = szVectorPath.substr( nDelimiter + 1, szVectorPath.length() - 1 );

		//Navigate to Game Arena
		KaiManager::getInstance().KaiEnterArena(szVectorPath, "", true);
	}

	// Now let's intialize our XUI Controls
	// Register all of our lists
	GetChildById( L"ArenaList", &m_ArenaList );
	GetChildById( L"PlayerList", &m_PlayerList );
	GetChildById( L"BuddyList", &m_BuddyList );

	// Register all of our List Controls
	GetChildById( L"PrevList", &m_PrevList );
	GetChildById( L"NextList", &m_NextList );
	GetChildById( L"CurrentPlayerList", &m_CurrentPlayerList );

	// Register all of our buttons
	GetChildById( L"Back", &m_Back );
	GetChildById( L"Action", &m_Action );
	GetChildById( L"ChatToggle", &m_ToggleChat );

	// Register all chat log controls
	GetChildById( L"Chat", &m_Chat );
	GetChildById( L"TextInput", &m_TextInput );
	GetChildById( L"SendMessage", &m_SendMessage );
	
	// Register all Scene elements // title etc
	GetChildById( L"CurrentArena", &m_CurrentArena );
	GetChildById( L"ListFocus", &m_ListFocus );

	// Let's grab the current Arena Info from the ArenaManager
	ArenaManager::getInstance().getArenaInfo(&m_pArenaInfo);

	// Initialize our scene elements iwth the information we know
	m_CurrentArena.SetText(m_pArenaInfo.szArenaNameW.c_str());
	m_CurrentPlayerList.SetText(L"Player List");


	m_bChatEnabled = false;
	m_bInMsgBox = false;
	m_bJoinGame = false;


	chatEnabled = false;
	inMsgBox = false;
	joinGame = false;
//	m_Action.SetShow(false);

	m_bEnterArenaMsg = true;
	handleEnterArena();

	// Init Chat Buffer
	szChatBuffer = L"";
	szChatMessage = L"";
	ArenaManager::getInstance().StartChatChannel();
	nTotalChatLines = 0;
	nMaxVisibleChatLines = m_Chat.GetMaxVisibleLineCount();

	listType = 0;

	// Start our timers
	SetTimer(TM_ENTERARENA, 50);
	SetTimer(TM_NOTIFYCHAT, 100);
    return S_OK;
}

HRESULT CScnXlinkKaiArena::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	// Something in the Arena Scene was pressed
	if(hObjPressed == m_ArenaList) {
		// The Arena List was pressed- let's step into the next arena
		int nArenaIndex = m_ArenaList.GetCurSel();
		// Store the current Arena Information
		ArenaManager::getInstance().getArenaInfo(&m_pArenaInfo);

		// Store our selected arena
		m_szSelectedArena = m_pArenaInfo.vSubArenas.at(nArenaIndex).szArenaVector;

		// Before entering this new arena, we need to make sure it's not password protected
		if(m_pArenaInfo.vSubArenas.at(nArenaIndex).bHasPassword)
		{
			//Promp user for password
			memset(&keyboard, 0, sizeof(keyboard));
			memset(szKbBuffer, 0, sizeof(szKbBuffer));

			// Set up our keyboard scene
			szDescription = sprintfaW(L"Please enter the password for %s", m_pArenaInfo.vSubArenas.at(nArenaIndex).szArenaNameW.c_str());
			szTitle = L"Password Required";
			//m_dwKeyboardType = 0;
			keyboardType = 0;

			// Let's launch our keyboard!
			XShowKeyboardUI(0, VKBD_LATIN_PASSWORD, L"", szTitle.c_str(), szDescription.c_str(),
				szKbBuffer, 200, &keyboard);
			
			// Start our keyboard timer
			SetTimer(TM_KEYBOARD, 50);
			bHandled = TRUE;
			return S_OK;
		} else {
			// No password was required, so let's enter our selected arena
			m_szArenaPassword = "";
			KaiManager::getInstance().KaiEnterArena(m_szSelectedArena, m_szArenaPassword, true);
		}

		// Now that we are in our arena, let's set up our buttons
		if(m_pArenaInfo.vSubArenas.at(nArenaIndex).bIsPersonal == true  )
		{
			// We are inside of a game room (so we should be able to launch game)
			m_bInsideGame = true;
			
			// If we are entering a personal arena via the game list, let's show our start game button
			if( m_bFromGamelist == true ) {
				// Enable our button, set our text, and display button
				m_Action.SetEnable(TRUE);
				m_Action.SetText(L"Launch Game");
				m_Action.SetShow(TRUE);
			} 
			else
			{
				// We are not in the game list, but in a personal room- 
				// Here we need to enable arena launching - somehow
			}
		} else {	// We are in a public arena
			// We are not inside of a game room (so we can't launch a game until we host)
			m_bInsideGame = false;

			// Enable our button, set our text, and display button
			m_Action.SetEnable(TRUE);
			m_Action.SetText(L"Host Game");
			m_Action.SetShow(FALSE);
		}
		bHandled = TRUE;
	}
	else if(hObjPressed == m_PlayerList)
	{
		int index = m_PlayerList.GetCurSel();

		PLAYER_INFO * m_pPlayer = (PLAYER_INFO*)malloc(sizeof(PLAYER_INFO));

		m_pPlayer->playerIndex = index;

		m_pPlayer->isBuddy = false;

		SkinManager::getInstance().setScene("XlinkKaiPlayerInfo.xur", *this, true, "XlinkKaiPLayerInfo", m_pPlayer);

		bHandled = TRUE;
	}
	else if(hObjPressed == m_BuddyList)
	{
		int index = m_BuddyList.GetCurSel();

		PLAYER_INFO * m_pPlayer = (PLAYER_INFO*)malloc(sizeof(PLAYER_INFO));

		m_pPlayer->playerIndex = index;
		m_pPlayer->isBuddy = true;

		SkinManager::getInstance().setScene("XlinkKaiPlayerInfo.xur", *this, true, "XlinkKaiPLayerInfo", m_pPlayer);

		bHandled = TRUE;
	}
	else if(hObjPressed == m_ToggleChat)
	{
		if(!chatEnabled)
		{
			szChatBuffer = L"";
			szChatMessage = L"";
			nTotalChatLines = 0;
			nMaxVisibleChatLines = m_Chat.GetMaxVisibleLineCount();

			int FrameStart, FrameEnd;
			this->FindNamedFrame(strtowstr("EnableChat").c_str(), &FrameStart);
			this->FindNamedFrame(strtowstr("EnableChatEnd").c_str(), &FrameEnd);
			if(FrameStart != -1 && FrameEnd != -1)
			{
				DebugMsg("ScnXlinkKaiArena","%d start %d End", FrameStart, FrameEnd);
				this->PlayTimeline(FrameStart, FrameStart, FrameEnd, false, false);
			}

			KaiManager::getInstance().KaiEnableChatmode();
			m_ToggleChat.SetText(L"Disable Chat");

			chatEnabled = true;
		}
		else
		{
			int FrameStart, FrameEnd;
			this->FindNamedFrame(strtowstr("DisableChat").c_str(), &FrameStart);
			this->FindNamedFrame(strtowstr("DisableChatEnd").c_str(), &FrameEnd);
			if(FrameStart != -1 && FrameEnd != -1)
			{
				DebugMsg("ScnXlinkKaiArena","%d start %d End", FrameStart, FrameEnd);
				this->PlayTimeline(FrameStart, FrameStart, FrameEnd, false, false);
			}

			KaiManager::getInstance().KaiLeaveChatmode();
			m_ToggleChat.SetText(L"Enable Chat");

			chatEnabled = false;
		}
		bHandled = TRUE;
	}
	else if(hObjPressed == m_TextInput)
	{
		// Initailize the keyboard for text input
		memset(&keyboard, 0, sizeof(keyboard));
		memset(szKbBuffer, 0, sizeof(szKbBuffer));
		keyboardType = 1;

		XShowKeyboardUI(0, VKBD_DEFAULT, szChatMessage.c_str(), L"Send Message", L"Enter a new message",
			szKbBuffer,200,&keyboard);

		SetTimer(TM_KEYBOARD, 50);
		bHandled = TRUE;
	}
	else if(hObjPressed == m_SendMessage)
	{
		ArenaManager::getInstance().sendChatMessage( szChatMessage );

		szChatMessage = L"";
		m_TextInput.SetText(L"");

		bHandled = TRUE;
	}
	else if(hObjPressed == m_Action)
	{
		// Our action button was pressed and we need to decide what action we should take
		switch (m_nActionFunc) {
			case ACTION_TYPE_LAUNCH:
				// If we're in the gamelist and we have a launch game action- let's launch
				if( m_bFromGamelist == true ) {
					// Launch from game list
					if( snapShot.CurrentGame != NULL && snapShot.CurrentGame->ContentRef != NULL ) {
						snapShot.CurrentGame->ContentRef->LaunchGame();
					}
				} else {
					// Launch from arena - need a routine that can find a game in your list
					// Based on VectorPath in Kai
				}
				break;
			case ACTION_TYPE_HOST:
				// Invoke the skin manager to launch our next skin
				SkinManager::getInstance().setScene("XlinkKaiHost.xur", *this, true);
				break;
			case ACTION_TYPE_UNKNOWN:
				DebugMsg("ScnXlinkKaiArena", "ACTION_TYPE_UNKNOWN.....");
			default:
				break;
		};
		bHandled = TRUE;
	}
	else if(hObjPressed == m_PrevList)
	{
		if(listType == 0)
		{
			LoadList(ListSize - 1);

			listType = ListSize - 1;
		}
		else
		{
			listType--;

			LoadList(listType);
		}

		bHandled = TRUE;
	}
	else if(hObjPressed == m_NextList)
	{
		listType++;

		if(listType == ListSize)
		{
			LoadList(0);
			listType = 0;
		}
		else
		{
			LoadList(listType);
		}

		bHandled = TRUE;
	}
	else if(hObjPressed == m_Back)
	{
		// Grab our current state
		ArenaManager::getInstance().getArenaInfo(&m_pArenaInfo);
		if(m_bFromGamelist) {
			if(strcmp(m_pArenaInfo.szArenaName.c_str(), m_szGameArenaName.c_str()) == 0) {
				NavigateBack(XUSER_INDEX_ANY);
			} else {
				KaiManager::getInstance().KaiLeaveArena();
			}
		}
		else if(strcmp(m_pArenaInfo.szArenaName.c_str(),"Arena") == 0 || strcmp(m_pArenaInfo.szArenaName.c_str(),"") == 0 ) {
			NavigateBack(XUSER_INDEX_ANY);
		} else {
			KaiManager::getInstance().KaiLeaveArena();
		}

		bHandled = TRUE;
	}

	return S_OK;
}

HRESULT CScnXlinkKaiArena::OnNotifySetFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled )
{
	if(hObjSource == m_ListFocus)
	{
		switch(listType)
		{
			case OpponentList:
				m_PlayerList.SetFocus();
			break;
			case BuddyList:
				m_BuddyList.SetFocus();
			break;
		}
	}
	    
    bHandled = TRUE;
    
    return S_OK ;
}

HRESULT CScnXlinkKaiArena::SetActionInterface( ACTION_TYPE nActionFunc )
{
	switch (nActionFunc)
	{
	case ACTION_TYPE_HOST:
		m_Action.SetEnable(TRUE);
		m_Action.SetText(L"Host Game");
		m_Action.SetShow(TRUE);
		break;
	case ACTION_TYPE_LAUNCH:
		m_Action.SetEnable(TRUE);
		m_Action.SetText(L"Launch Game");
		m_Action.SetShow(TRUE);
		break;
	case ACTION_TYPE_UNKNOWN:
	default:
		m_Action.SetEnable(FALSE);
		m_Action.SetShow(FALSE);
		m_Action.SetText(L"Unknown");
		break;
	};

	return S_OK;
}

HRESULT CScnXlinkKaiArena::handleEnterArena( void ) {
	// First thing we want to do is reset our event flag
	if( m_bEnterArenaMsg == false ) return S_FALSE;
	m_bEnterArenaMsg = false;

	// Now we can update our User Interface
	m_nActionFunc = ACTION_TYPE_UNKNOWN;

	// We'll update our arena info
	ArenaManager::getInstance().getArenaInfo(&m_pArenaInfo);

	// First let's update our arena vector
	m_CurrentArena.SetText( m_pArenaInfo.szArenaVectorW.c_str() );
	//m_szSelectedArena = m_pArenaInfo.szArenaVector;

	// Using our global arena state- let's check to see if we can host
	if( m_pArenaInfo.bHostingAllowed == true ) {
		m_nActionFunc = ACTION_TYPE_HOST;
	} else {
		if( m_bInsideGame = false ) {
			// Check if we are inside of a private room and hosting
			if( KaiManager::getInstance().KaiIsInPrivate() == TRUE ) {
				// We are the host of this private room
				if(m_bFromGamelist == true )
					m_nActionFunc = ACTION_TYPE_LAUNCH;
				else
					m_nActionFunc = ACTION_TYPE_UNKNOWN;
			} else {
				if(m_bFromGamelist == true )
					m_nActionFunc = ACTION_TYPE_LAUNCH;
				else
					m_nActionFunc = ACTION_TYPE_UNKNOWN;
			}
		} else {
			// Check if we are inside of a private room and hosting
			if( KaiManager::getInstance().KaiIsInPrivate() == TRUE ) {
				// We are the host of this private room
				if(m_bFromGamelist == true )
					m_nActionFunc = ACTION_TYPE_LAUNCH;
				else
					m_nActionFunc = ACTION_TYPE_UNKNOWN;
			} else {
				if(m_bFromGamelist == true )
					m_nActionFunc = ACTION_TYPE_LAUNCH;
				else
					m_nActionFunc = ACTION_TYPE_UNKNOWN;
			}
		}
	}

	// Make the changes to the action button
	SetActionInterface(m_nActionFunc);

	// Reset our chat buffer
	ArenaManager::getInstance().clearChatBuffer();
	m_Chat.SetTopLine(0, TRUE);
	m_Chat.SetText(L"");

	return S_OK;
}

HRESULT CScnXlinkKaiArena::handleEnterArenaFailed( void ) {
	// First thing we want to do is is kill this timer
	KillTimer(TM_ENTERARENAFAILED);
	return S_OK;
}

HRESULT CScnXlinkKaiArena::handleNotifyChat( void ) {
	// Update has triggered- lets do some work
	if( bChatChanged == false ) return S_FALSE;
	// Reset our flag, so it can be triggered again by another thread
	bChatChanged = false;

	// Adds an offset to the ChatLog to keep text from being cut off
	nTotalChatLines = m_Chat.GetLineCount() + CHAT_LOG_OFFSET;

	// Move the view downward to stay on current chat
	if( nTotalChatLines > nMaxVisibleChatLines ) {
		m_Chat.SetTopLine( nTotalChatLines - nMaxVisibleChatLines, TRUE );
	} else {
		m_Chat.SetTopLine( 0, TRUE );
	}

	// Update our chat log's buffer
	m_Chat.SetText( ArenaManager::getInstance().getChatBuffer().c_str() );
	
	return S_OK;
}

HRESULT CScnXlinkKaiArena::handleNotifyPrivateMsg( void ) {
	// First thing we want to do is is kill this timer
	KillTimer(TM_NOTIFYPRIVATEMSG);
	return S_OK;
}

HRESULT CScnXlinkKaiArena::handleNotifyInvite( void ) {
	// First thing we want to do is is kill this timer
	KillTimer(TM_NOTIFYINVITE);
	return S_OK;
}

HRESULT CScnXlinkKaiArena::OnTimer(XUIMessageTimer *pTimer, BOOL& bHandled)
{
    switch(pTimer->nId)
    {
		case TM_ENTERARENA:
			handleEnterArena();
			break;
		case TM_ENTERARENAFAILED:
			handleEnterArenaFailed();
			break;
		case TM_NOTIFYCHAT:
			handleNotifyChat();
			break;
		case TM_NOTIFYPRIVATEMSG:
			handleNotifyPrivateMsg();
			break;
		case TM_NOTIFYINVITE:
			handleNotifyInvite();
			break;
		case TM_KEYBOARD:
		{
			if(XHasOverlappedIoCompleted(&keyboard))
            {
				KillTimer(TM_KEYBOARD);
				bHandled = TRUE;
				if(keyboard.dwExtendedError == ERROR_SUCCESS)
                {
					if(keyboardType == 0) //Join Private Arena
					{
						KaiManager::getInstance().KaiEnterArena(m_szSelectedArena, wstrtostr(szKbBuffer), true);
						inMsgBox = false;
					}
					else if(keyboardType == 1)//Send Chat Msg
					{
						szChatMessage = szKbBuffer;
						m_TextInput.SetText(szKbBuffer);	
					}
					else if(keyboardType == 2) // Reply to PM
					{
						KaiManager::getInstance().KaiSendPM(player, wstrtostr(szKbBuffer));
						inMsgBox = false;
					}
				}				
			}
			break;
		}
    }
    return S_OK;
}

HRESULT CScnXlinkKaiArena::OnMsgReturn(XUIMessageMessageBoxReturn *pMsgBox, BOOL &bHandled)
{
	switch( pMsgBox->nButton )
    {
        case 0:
			if(messageboxType == 1)// Join Arena Invite
			{
				KaiManager::getInstance().KaiEnterArena(arena, "", true);
				inMsgBox = false;
			}
			else if(messageboxType == 2)// Reply to PM
			{
//				buffer = new WCHAR[512];

				memset(&keyboard, 0, sizeof(keyboard));
				memset(szKbBuffer, 0, sizeof(szKbBuffer));

				XShowKeyboardUI(0,VKBD_DEFAULT,L"",L"Private Message",L"Enter Message", szKbBuffer, 150, &keyboard);
				SetTimer(TM_KEYBOARD, 50);
				keyboardType = 2;
			}
			else if(messageboxType == 3) // Join arena failed
			{
				//Promp user for password
				memset(&keyboard, 0, sizeof(keyboard));
				memset(szKbBuffer, 0, sizeof(szKbBuffer));

				// Set up our keyboard scene
				szDescription = sprintfaW(L"Please enter the password for %s", strtowstr(arena).c_str());
				szTitle = L"Password Required";
				//m_dwKeyboardType = 0;
				keyboardType = 0;

				m_szSelectedArena = arena;

				// Let's launch our keyboard!
				XShowKeyboardUI(0, VKBD_LATIN_PASSWORD, L"", szTitle.c_str(), szDescription.c_str(),
					szKbBuffer, 200, &keyboard);
				
				// Start our keyboard timer
				SetTimer(TM_KEYBOARD, 50);
			}
		break;
	}

	bHandled = TRUE;
	return S_OK;
}


//Send General Chat Message
HRESULT CScnXlinkKaiArena::SendMessage(string buffer)
{
	KaiManager::getInstance().KaiChat(buffer);
	return S_OK;
}

void CScnXlinkKaiArena::LoadList(int listType)
{
	switch(listType)
	{
		case OpponentList:
			m_PlayerList.SetShow(true);
			m_BuddyList.SetShow(false);

			m_PlayerList.SetFocus();

			m_CurrentPlayerList.SetText(L"Player List");

			break;
		case BuddyList:
			m_PlayerList.SetShow(false);
			m_BuddyList.SetShow(true);

			m_BuddyList.SetFocus();

			m_CurrentPlayerList.SetText(L"Buddy List");

			break;
	}
}

//Private Message Recieved
HRESULT CScnXlinkKaiArena::OnNotifyChatPrivateMessage(KAIMessageOnChatPrivateMessage * pData, BOOL& bHandled)
{
	if(!inMsgBox)
	{
		//Store player for pm send
		player = pData->szPlayer;
		szDescription = strtowstr(sprintfaA("From %s: \n\n%s", pData->szPlayer.c_str(), pData->szMessage.c_str()));
		inMsgBox = true;

		ShowMessageBoxEx(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(), L"Private Message", szDescription.c_str(), 2, szBtnReply, 0, NULL, NULL);
		messageboxType = 2;
	}
	bHandled = TRUE;
    return S_OK ; 
}

//Arena Invite Recieved
HRESULT CScnXlinkKaiArena::OnNotifyContactInvite(KAIMessageOnContactInvite * pData, BOOL& bHandled)
{

	if(!inMsgBox)
	{
		//Store arean for join
		arena = pData->szVector;
		szDescription = strtowstr(sprintfaA("%s has invited you to join \n%s \n\n%s", pData->szFriend.c_str(), pData->szVector.c_str(), pData->szMessage.c_str()));
		inMsgBox = true;

		ShowMessageBoxEx(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(), L"Arena Invite", szDescription.c_str(), 2, szBtnJoin, 0, NULL, NULL);
		messageboxType = 1;
	}

	bHandled = TRUE;
    return S_OK ; 
}

// Recieved Kai Message for entering a new arena
HRESULT CScnXlinkKaiArena::OnEnterArena(KAIMessageOnEnterArena * pData, BOOL& bHandled) {
	
	// First let's sync our local item with our ArenaManager state
	m_bEnterArenaMsg = true;
	bHandled = TRUE;

	// Return succussfully
	return S_OK;
}

//Failed to Join Private Arena
HRESULT CScnXlinkKaiArena::OnEnterArenaFailed(KAIMessageOnEnterArenaFailed * pData, BOOL& bHandled)
{
	if(!inMsgBox)
	{
		int vectorDelimiter = pData->szVector.rfind('/');
		string vectorName = pData->szVector.substr(vectorDelimiter + 1, pData->szVector.length() - 1);
		if(arena != "")
			vectorName = arena;
		inMsgBox = true;
		szDescription = strtowstr(sprintfaA("Failed to join %s:\n%s", vectorName.c_str(), pData->szReason.c_str()));

		ShowMessageBoxEx(L"XuiMessageBox1", CFreestyleUIApp::getInstance().GetRootObj(), L"Failed To Join Arena", szDescription.c_str(), 1, szBtnOk, 0, NULL, NULL);
		messageboxType = 3;
	}
	return S_OK;
}

HRESULT CScnXlinkKaiArena::OnNotifyChat( KAIMessageOnChat * pNotifyChat, BOOL& bHandled )
{
	// Trigger the flag to notify our timer that the chat has updated
	bChatChanged = true;
	bHandled=  TRUE;

	return S_OK;
}