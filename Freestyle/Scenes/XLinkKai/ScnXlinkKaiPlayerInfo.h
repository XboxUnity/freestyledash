#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/Arena/ArenaManager.h"

using namespace std;

typedef struct
{
	int playerIndex;
	bool isBuddy;
} PLAYER_INFO, *pPLAYER_INFO;

class CScnXlinkKaiPlayerInfo : public CXuiSceneImpl
{
private:
	XOVERLAPPED keyboard;
	WCHAR * buffer;

	wstring temp;

protected:
	CXuiScene m_KaiControls;
    CXuiControl m_back;

	CXuiControl m_AddRemoveFriend,
				m_PlayerPM,
				m_PlayerInvite;

	CXuiTextElement m_PlayerName,
					m_PlayerAge,
					m_PlayerLocation,
					m_PlayerConnection,
					m_PlayerBio;

	bool isNested, isBuddy;
	string player;
	int inputType, playerIndex;
	ARENAITEM m_pArenaInfo;
	std::map<string, CKaiPlayer> m_pBuddyList;
	vector<string> m_BuddyNames;

public:

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_TIMER( OnTimer )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		KAI_ON_NOTIFY_UPDATE_OPPONENT_INFO( OnUpdateOpponentInfo )
		KAI_ON_NOTIFY_UPDATE_OPPONENT_AVATAR( OnUpdateOpponentAvatar )
	XUI_END_MSG_MAP()

    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnTimer(XUIMessageTimer *pTimer, BOOL& bHandled);
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);

	HRESULT OnUpdateOpponentInfo(KAIMessageOnUpdateOpponentInfo * pData, BOOL& bHandled);
	HRESULT OnUpdateOpponentAvatar(KAIMessageOnUpdateOpponentAvatar * pData, BOOL& bHandled);

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnXlinkKaiPlayerInfo, L"ScnXlinkKaiPlayerInfo", XUI_CLASS_TABSCENE )
};
