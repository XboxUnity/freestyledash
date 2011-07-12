#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/ContentList/ContentManager.h"
#include "../../Tools/GameContent/GameContentManager.h"

using namespace std;

class CScnGameControls : public CXuiTabSceneImpl
{

protected:

	// Xui Scene
	CXuiScene m_GameControls;

	// Xui Buttons
	CXuiControl m_MoveXbox360, m_MoveXBLA, m_MoveHomebrew, m_MoveXbox1, m_MoveEmulators, m_MoveNoSection;
	CXuiControl m_GameScreenshots, m_GameSaves, m_RenameGame, m_DeleteGame, m_MoveGame, m_DownloadGame;
	CXuiControl m_Back, m_Loading, m_ManageTUs, m_Achievements, m_ManageTrainers;

	// Xui Text Elements
	CXuiTextElement m_OverallPercent, m_GameTitle, m_FileName;
	
	// Xui Progress Bar
	CXuiProgressBar m_TotalProgressBar;

	// Animation Variables
	int nFrameStart, nFrameEnd;
	bool bIsNested, bBackToMain;

	// Game Specific Variables
	int nDiskNumber;
	CONTENT_ITEM_TAB nGameType;
	wstring szGameId;

	// Variables for Keyboard Control
	WCHAR * kbBuffer;
	XOVERLAPPED xoKeyboard;
	LPCWSTR* szBtnOk;

	int nMsgType;  // 0 = default, 1 = delete, 2 = move no section

	// Pointers
	GameListItem * pGameContent;
	ContentItemNew * pContentItem;

	// GameContentManager Variables
	GAMECONTENT_LIST_STATE listPack;
	CONTENT_ITEM_TAB m_sTabId;

    // Message map.
	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_MSG_RETURN( OnMsgReturn )
	XUI_END_MSG_MAP()

    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
    HRESULT InitializeChildren( void );
	HRESULT DisableCurrentGameType(CONTENT_ITEM_TAB sCurrentTabId);
	HRESULT ReloadContent(CONTENT_ITEM_TAB sTabId);
	HRESULT ReloadContent();
	void ShowFileProgress(bool show);
	void UpdateFileProgress();
	CScnGameControls();
	~CScnGameControls();
	
	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnMsgReturn(XUIMessageMessageBoxReturn *pXUIMessageMessageBoxReturn, BOOL &bHandled);
private:
	wstring szTempString;
	wstring szTempStringPath;
public:

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnGameControls, L"ScnGameControls", XUI_CLASS_TABSCENE )
};
