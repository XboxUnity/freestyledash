#include "stdafx.h"

#include "ScnXlinkKaiPlayerInfo.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/Buddy/KaiBuddyManager.h"
#include "../../Tools/XlinkKai/General/KaiManager.h"

HRESULT CScnXlinkKaiPlayerInfo::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{	
	DebugMsg("CScnXlinkKaiPlayerInfo","CScnXlinkKaiPlayerInfo::OnInit");

	//Get Data from ScnXlinkKaiAren
	PLAYER_INFO * m_pPlayer = (PLAYER_INFO *)pInitData->pvInitData;

	isBuddy = m_pPlayer->isBuddy;

	ArenaManager::getInstance().getArenaInfo(&m_pArenaInfo);

	HRESULT hr = GetChildById( L"KaiControls", &m_KaiControls );
	isNested = hr == S_OK;

	DebugMsg("CScnXlinkKaiHost","IsNested %d", isNested);

	if(isNested)
	{
		m_KaiControls.GetChildById( L"PlayerName", &m_PlayerName );
		m_KaiControls.GetChildById( L"PlayerAge", &m_PlayerAge );
		m_KaiControls.GetChildById( L"PlayerLocation", &m_PlayerLocation );
		m_KaiControls.GetChildById( L"PlayerConnection", &m_PlayerConnection );
		m_KaiControls.GetChildById( L"PlayerBio", &m_PlayerBio );
		m_KaiControls.GetChildById( L"AddRemoveFriend", &m_AddRemoveFriend );
		m_KaiControls.GetChildById( L"PlayerPM", &m_PlayerPM );
		m_KaiControls.GetChildById( L"PlayerInvite", &m_PlayerInvite );
	}
	else
	{
		GetChildById( L"PlayerName", &m_PlayerName );
		GetChildById( L"PlayerAge", &m_PlayerAge );
		GetChildById( L"PlayerLocation", &m_PlayerLocation );
		GetChildById( L"PlayerConnection", &m_PlayerConnection );
		GetChildById( L"PlayerBio", &m_PlayerBio );
		GetChildById( L"AddRemoveFriend", &m_AddRemoveFriend );
		GetChildById( L"PlayerPM", &m_PlayerPM );
		GetChildById( L"PlayerInvite", &m_PlayerInvite );
	}

	GetChildById( L"Back", &m_back );

	if(isBuddy)
	{
		CKaiBuddyManager::getInstance().getBuddyInfo(&m_pBuddyList);
		CKaiBuddyManager::getInstance().getBuddyLookupInfo(&m_BuddyNames);

		player = wstrtostr(m_pBuddyList[m_BuddyNames.at(m_pPlayer->playerIndex)].GetPlayerName());

		m_AddRemoveFriend.SetText(L"Remove Friend");
		m_PlayerInvite.SetShow(true);
	}
	else
	{
		player = wstrtostr(m_pArenaInfo.vArenaPlayers.at(m_pPlayer->playerIndex).GetPlayerName());

		//Clear Old Oponent Avatar
		string dataPath = SETTINGS::getInstance().getDataPath();
		dataPath = sprintfaA("file://%s\\%s\\%s\\%s.dds",dataPath.c_str(),"XlinkKaiData", "Player", "Opponent");
		dataPath = str_replaceallA(dataPath,"\\\\","\\");

		unlink(dataPath.c_str());

		m_PlayerInvite.SetShow(false);
	}

	KaiManager::getInstance().KaiQueryUserProfile(player);
	KaiManager::getInstance().KaiQueryAvatar(player);

    return S_OK;
}

HRESULT CScnXlinkKaiPlayerInfo::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (hObjPressed == m_AddRemoveFriend)
	{
		if(isBuddy)
		{
			KaiManager::getInstance().KaiBuddyRemove(player);

			m_AddRemoveFriend.SetText(L"Add Friend");
			m_PlayerInvite.SetShow(false);
			isBuddy = false;
		}
		else
		{
			KaiManager::getInstance().KaiBuddyAdd(player);

			m_AddRemoveFriend.SetText(L"Remove Friend");
			m_PlayerInvite.SetShow(true);
			isBuddy = true;
		}

		bHandled = TRUE;
	}
	else if (hObjPressed == m_PlayerPM)
	{
		buffer = new WCHAR[512];

		memset(&keyboard, 0, sizeof(keyboard));
		memset(buffer, 0, sizeof(buffer));

		XShowKeyboardUI(0,VKBD_DEFAULT,L"",L"Private Message",L"Enter Message", buffer, 150, &keyboard);
		SetTimer(TM_KEYBOARD, 50);
		inputType = 1;

		bHandled = TRUE;
	}
	else if (hObjPressed == m_PlayerInvite)
	{
		buffer = new WCHAR[512];

		memset(&keyboard, 0, sizeof(keyboard));
		memset(buffer, 0, sizeof(buffer));

		XShowKeyboardUI(0,VKBD_DEFAULT,L"",L"Player Invite",L"Enter Message", buffer, 150, &keyboard);
		SetTimer(TM_KEYBOARD, 50);
		inputType = 2;

		bHandled = TRUE;
	}
	else if (hObjPressed == m_back)
	{
		NavigateBack(XUSER_INDEX_ANY);

		bHandled = TRUE;
	}

    return S_OK;
}

HRESULT CScnXlinkKaiPlayerInfo::OnUpdateOpponentInfo(KAIMessageOnUpdateOpponentInfo * pData, BOOL& bHandled)
{
	m_PlayerName.SetText(strtowstr(pData->szOpponent).c_str());
	m_PlayerAge.SetText(strtowstr(pData->szAge).c_str());
	m_PlayerLocation.SetText(strtowstr(pData->szLocation).c_str());
	m_PlayerConnection.SetText(strtowstr(pData->szBandwidth).c_str());
	m_PlayerBio.SetText(strtowstr(pData->szBio).c_str());

	return S_OK;
}

HRESULT CScnXlinkKaiPlayerInfo::OnUpdateOpponentAvatar(KAIMessageOnUpdateOpponentAvatar * pData, BOOL& bHandled)
{
	DebugMsg("ScnXlinkKaiPLayerInfo","Player Avatar %s", pData->szAvatarURL.c_str());

	ArenaManager::getInstance().DownloadPlayerAvatar(pData->szAvatarURL, pData->szOpponent, isBuddy);

	return S_OK;
}

HRESULT CScnXlinkKaiPlayerInfo::OnGetSourceDataImage(XUIMessageGetSourceImage * pGetSourceImageData, BOOL& bHandled)
{
	if( ( 100 == pGetSourceImageData->iData ) && ( !pGetSourceImageData->bItemData ) ) 
	{
		string dataPath;
		if(isBuddy)
		{
			dataPath = SETTINGS::getInstance().getDataPath();
			dataPath = sprintfaA("file://%s\\%s\\%s\\%s.dds",dataPath.c_str(),"XlinkKaiData", "Player", player.c_str());
			dataPath = str_replaceallA(dataPath,"\\\\","\\");
		}
		else
		{
			dataPath = SETTINGS::getInstance().getDataPath();
			dataPath = sprintfaA("file://%s\\%s\\%s\\%s.dds",dataPath.c_str(),"XlinkKaiData", "Player", "Opponent");
			dataPath = str_replaceallA(dataPath,"\\\\","\\");
		}

		temp = strtowstr(dataPath);

		pGetSourceImageData->szPath = temp.c_str();
	}

	bHandled = TRUE;
	return S_OK;
}

HRESULT CScnXlinkKaiPlayerInfo::OnTimer(XUIMessageTimer *pTimer, BOOL& bHandled)
{
    switch(pTimer->nId)
    {
		case TM_KEYBOARD:
		{
			if(XHasOverlappedIoCompleted(&keyboard))
            {
				KillTimer(TM_KEYBOARD);
				bHandled = TRUE;
				if(keyboard.dwExtendedError == ERROR_SUCCESS)
                {
					if(inputType == 1) // PM
					{
						KaiManager::getInstance().KaiSendPM(player, wstrtostr(buffer));
					}
					else if(inputType == 2) // Arena Invite
					{
						KaiManager::getInstance().KaiArenaInvite(player, m_pArenaInfo.szArenaVector, wstrtostr(buffer));
					}
				}				
			}
			break;
		}
    }
    return S_OK;
}