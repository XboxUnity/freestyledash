#include "stdafx.h"

#include "ScnGameView.h"

#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/GameContent/GameContentManager.h"
#include "../../Tools/GameContent/GameContentMessages.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/XlinkKai/General/KaiManager.h"
#include "../../Tools/SQLite/FSDSql.h"

using namespace std;

CScnGameView::CScnGameView()
{
	// Initialize GameListItem pointer 
	pCurrentGame = NULL;

	// Initialize Private State Variables
	GameViewSettings.nCurrentIndex = 0;
	GameViewSettings.nGameListSize = 0;
	GameViewSettings.m_bShowingFavorites = false;
	GameViewSettings.m_bBackgroundVisible = false;
	GameViewSettings.nSortStyle = (GAMELIST_SORTSTYLE)0;
	GameViewSettings.nSortDirection = (GAMELIST_SORTDIRECTION)0;
	GameViewSettings.m_bLeftShiftHeld = false;
	GameViewSettings.m_bRightShiftHeld = false;
	GameViewSettings.m_bBackgroundVisible = false;

}

CScnGameView::~CScnGameView()
{
	// ScnGameView Clean up
	DebugMsg("CScnGameView", "CScnGameView Deconstructor [%08X]", this->m_hObj);
}

HRESULT CScnGameView::OnPlayTransition(FSDMessagePlayTransition *pData, BOOL& bHandled)
{
	if(bHasFrames) {
		this->PlayTimeline(nStartFrame, nStartFrame, nEndFrame, false, false);
	}
	bHandled = true;
	return S_OK;
}

HRESULT CScnGameView::OnInit( XUIMessageInit *pInitData, BOOL &bHandled )
{
	DebugMsg("ScnGameView", "OnInit- Initializing Scene [%X]", this->m_hObj);

	// Load the configurable scene settings map
	LoadSettings("ScnGameView", "ScnGameView"); 
	ReadSettings();

	// TransToBkg = 0; BlackToBkg = 1; SlideInBkg = 2;
	int nTransSetting = SETTINGS::getInstance().getGameListTransition();  
	LPCWSTR szStartFrameName;
	LPCWSTR szEndFrameName;

	switch (nTransSetting)
	{
	case FSDTRANSITION_BLACKTOIMAGE:
		szStartFrameName = FSDTRANSNAME_STARTOPT2;
		szEndFrameName = FSDTRANSNAME_ENDOPT2;
		break;
	case FSDTRANSITION_SLIDEINIMAGE:
		szStartFrameName = FSDTRANSNAME_STARTOPT3;
		szEndFrameName = FSDTRANSNAME_ENDOPT3;
		break;
	case FSDTRANSITION_TRANSTOIMAGE:		// Default Setting
	default:
		szStartFrameName = FSDTRANSNAME_STARTOPT1;
		szEndFrameName = FSDTRANSNAME_ENDOPT1;	
		break;
	};

	this->FindNamedFrame(szStartFrameName, &nStartFrame);
	this->FindNamedFrame(szEndFrameName, &nEndFrame);
	if(nStartFrame != -1 && nEndFrame != -1) bHasFrames = true; else bHasFrames = false;

	// Initialize XUI Children
	InitializeGameXuiControls();
	InitializeSceneXuiControls();

	// Grab Current GameListShowType from VariablesCache
	GameViewSettings.szContentType = VariablesCache::getInstance().getVariable("GameListShowType", "");
	CONTENT_ITEM_TAB sTabId = ContentManager::getInstance().GetTabIdFromString(GameViewSettings.szContentType);
	GameViewSettings.nContentType = (int)sTabId;
	GameContentManager::getInstance().setGameListTabId(sTabId);

	// Hide the Release Date and the ReleaseDate Caption if desired when not in XBLA List
	if(GameViewSettings.m_bAutoHideReleaseDate) {
		if(GameViewSettings.nContentType != (int)CONTENT_XBLA) {
			if(GameXuiControlState.hasReleaseDate) {
				GameXuiControls.m_GameReleaseDate.SetShow(false);
			}
			if(GameXuiControlState.hasReleaseDateCaption) {
				GameXuiControls.m_GameReleaseDateCaption.SetShow(false);
			}
		}
	}

	// Set the display title to the scene
	SetSceneTitle();

	int sortType = 0;
	int sortDirection = 0;

	//FSDSql::getInstance().getGameListSort(sTabId, 0, sortType, sortDirection);

	// Set the initial sort order sort style and sort direction text
	SetSortStyleCaptions((GAMELIST_SORTSTYLE)sortType);
	SetSortDirectionCaptions((GAMELIST_SORTDIRECTION)sortDirection);
	SetGameListCounter(0, 0);
	SetFavoritesButtonCaption(false);
	return S_OK;
}

HRESULT CScnGameView::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if(hObjPressed == SceneXuiControls.m_ButtonBack)
	{
		HXUIOBJ thisObj = this->m_hObj;
		HXUIOBJ backScene = this->GetBackScene();
		HXUIOBJ ScnRoot = SkinManager::getInstance().getXurObjectHandle["GameListScene"];
		HXUIOBJ hBack = XuiSceneGetBackScene(ScnRoot);
		
		HRESULT hr = XuiSceneNavigateBack(ScnRoot, hBack, XUSER_INDEX_ANY);
		bHandled = true;
		return S_OK;
	}
	if(hObjPressed == SceneXuiControls.m_ButtonAdjustRatingDown)
	{
		DebugMsg("ScnGameView", "Adjust Rating Down Button Pressed");
		GameContentManager::getInstance().AdjustUserRating(pCurrentGame, 0, 500, -25);
		bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_ButtonAdjustRatingUp)
	{
		DebugMsg("ScnGameView", "Adjust Rating Up Button Pressed");
		GameContentManager::getInstance().AdjustUserRating(pCurrentGame, 0, 500, 25);
		bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_ButtonToggleBackground)
	{
		DebugMsg("ScnGameView", "Toggle Background Button Pressed");
		ToggleBackground();
		bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_ButtonToggleFavorite)
	{
		DebugMsg("ScnGameView", "Toggle Favorite Button Pressed");
		GameContentManager::getInstance().ToggleFavorite(pCurrentGame);
		bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_ButtonToggleSubtitle)
	{
		DebugMsg("ScnGameView", "Toggle Subtitle Button Pressed");
		GameContentManager::getInstance().ToggleSubTitleView();
		bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_ButtonToggleSortDirection)
	{
		DebugMsg("ScnGameView", "Toggle Sort Direction Button Pressed");
		GAMELIST_SORTDIRECTION nSortDir = GameContentManager::getInstance().ToggleSortDirection();
		DebugMsg("ScnGameView", "Sort Direction:  %d", (int)nSortDir);		
		bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_ButtonToggleSortStyle)
	{
		DebugMsg("ScnGameView", "Toggle Sort Style Button Pressed");
		GAMELIST_SORTSTYLE nSortStyle = GameContentManager::getInstance().ToggleSortStyle();
		DebugMsg("ScnGameView", "Sort Style:  %d", (int)nSortStyle);
		bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_ButtonShowFavorites)
	{
		DebugMsg("ScnGameView", "Show Favorites Button Pressed");
		BOOL bShowFavs = GameContentManager::getInstance().ToggleDisplayFavorites();
		SetFavoritesButtonCaption(bShowFavs);
		SetSceneTitle(bShowFavs);
		bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_ButtonShowOptions)
	{
		DebugMsg("ScnGameView", "Show Options Button Pressed");
		if(GameViewSettings.m_bBackgroundVisible){
			bHandled = true;
			return S_OK;
		}

		// Set the a list snapshot for the currently selected game
		if(pCurrentGame != NULL && pCurrentGame->ContentRef != NULL) {
			GameContentManager::getInstance().setGameListSnapshot();
		} else {
			bHandled = true;
			return S_OK;
		}
	}
	if(hObjPressed == SceneXuiControls.m_ButtonShiftLB)
	{
		DebugMsg("ScnGameView", "LB Shift Button Pressed");
		//bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_ButtonShiftRB)
	{
		DebugMsg("ScnGameView", "RB Shift Button Pressed");
		//bHandled = true;
	}
	if(hObjPressed == SceneXuiControls.m_ButtonLaunchGame)
	{
		// Launches the currently focused game
		DebugMsg("ScnGameView", "Launch Game Button Pressed");
		if(GameViewSettings.nContentType == 7)
		{
			// Set the a list snapshot for the currently selected game
			if(pCurrentGame != NULL && pCurrentGame->ContentRef != NULL) {
				GameContentManager::getInstance().setGameListSnapshot();
			}
			else {
				bHandled = TRUE;
				return S_OK;
			}

			VECTOR_INFO * m_pVector = (VECTOR_INFO*)malloc(sizeof(VECTOR_INFO));
			m_pVector->launchGame = true;

			SkinManager::getInstance().setScene("XlinkKaiArena.xur", *this, false, "XlinkKaiArena", m_pVector);
		}
		else if(pCurrentGame != NULL && pCurrentGame->ContentRef != NULL) {
			pCurrentGame->ContentRef->LaunchGame();
		}
		
		bHandled = true;
	}
	return S_OK;
}

HRESULT CScnGameView::OnShiftKeyUp(FSDMessageShiftKeyUp * pShiftKeyUp, BOOL& bHandled)
{
	if(pShiftKeyUp->nShiftKey == GameViewSettings.m_wRightShiftKey && GameViewSettings.m_bRightShiftHeld){
		int FrameStart, FrameEnd;
		this->FindNamedFrame(strtowstr("RBRelease").c_str(), &FrameStart);
		this->FindNamedFrame(strtowstr("RBReleaseEnd").c_str(), &FrameEnd);
		if(FrameStart != -1 && FrameEnd != -1)
		{
			this->PlayTimeline(FrameStart, FrameStart, FrameEnd, false, false);
			GameViewSettings.m_bRightShiftHeld = false;
		}
		bHandled = true;
	}
	else if(pShiftKeyUp->nShiftKey == GameViewSettings.m_wLeftShiftKey && GameViewSettings.m_bLeftShiftHeld){
		int FrameStart, FrameEnd;
		this->FindNamedFrame(strtowstr("LBRelease").c_str(), &FrameStart);
		this->FindNamedFrame(strtowstr("LBReleaseEnd").c_str(), &FrameEnd);
		if(FrameStart != -1 && FrameEnd != -1)
		{
			this->PlayTimeline(FrameStart, FrameStart, FrameEnd, false, false);
			GameViewSettings.m_bLeftShiftHeld = false;
		}
		bHandled = true;
	}
	return S_OK;
}

HRESULT CScnGameView::OnShiftKeyDown(FSDMessageShiftKeyDown * pShiftKeyDown, BOOL& bHandled)
{
	if(pShiftKeyDown->nShiftKey == GameViewSettings.m_wRightShiftKey && !GameViewSettings.m_bLeftShiftHeld){
		if(GameViewSettings.m_bRightShiftHeld == false)
		{
			int FrameStart, FrameEnd;
			this->FindNamedFrame(strtowstr("RBOn").c_str(), &FrameStart);
			this->FindNamedFrame(strtowstr("RBOnEnd").c_str(), &FrameEnd);
			if(FrameStart != -1 && FrameEnd != -1)
			{
				this->PlayTimeline(FrameStart, FrameStart, FrameEnd, false, false);
				GameViewSettings.m_bRightShiftHeld = true;
			}
			
			if(SceneXuiControlState.hasButtonShiftRB)
				SceneXuiControls.m_ButtonShiftRB.Press(XUSER_INDEX_ANY);
		}
		bHandled = true;
	}
	else if(pShiftKeyDown->nShiftKey == GameViewSettings.m_wLeftShiftKey && !GameViewSettings.m_bRightShiftHeld ){
		if(GameViewSettings.m_bLeftShiftHeld == false)
		{
			int FrameStart, FrameEnd;
			this->FindNamedFrame(strtowstr("LBOn").c_str(), &FrameStart);
			this->FindNamedFrame(strtowstr("LBOnEnd").c_str(), &FrameEnd);
			if(FrameStart != -1 && FrameEnd != -1)
			{
				this->PlayTimeline(FrameStart, FrameStart, FrameEnd, false, false);
				GameViewSettings.m_bLeftShiftHeld = true;
			}

			if(SceneXuiControlState.hasButtonShiftLB)
				SceneXuiControls.m_ButtonShiftLB.Press(XUSER_INDEX_ANY);
		}
		bHandled = true;

	}

	return S_OK;
}

HRESULT CScnGameView::OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled)
{
	if(pInputData->dwKeyCode == GameViewSettings.m_wRightShiftKey){
		FsdShiftKeyDown(SkinManager::getInstance().getXurObjectHandle["GameListScene"], GameViewSettings.m_wRightShiftKey);
		bHandled = true;
	}
	if(pInputData->dwKeyCode == GameViewSettings.m_wLeftShiftKey){
		FsdShiftKeyDown(SkinManager::getInstance().getXurObjectHandle["GameListScene"], GameViewSettings.m_wLeftShiftKey);
		bHandled = true;
	}
	return S_OK;
}

HRESULT CScnGameView::OnKeyUp(XUIMessageInput *pInputData, BOOL& bHandled)
{
	if(pInputData->dwKeyCode == GameViewSettings.m_wRightShiftKey){
		FsdShiftKeyUp(SkinManager::getInstance().getXurObjectHandle["GameListScene"], GameViewSettings.m_wRightShiftKey);
		bHandled = true;
	}
	if(pInputData->dwKeyCode == GameViewSettings.m_wLeftShiftKey){
		FsdShiftKeyUp(SkinManager::getInstance().getXurObjectHandle["GameListScene"], GameViewSettings.m_wLeftShiftKey);
		bHandled = true;
	}

	return S_OK;
}

HRESULT CScnGameView::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled)
{ 

	// Request current list from the GameViewManager
	GameListContentPtr vListContent = GameContentManager::getInstance().getCurrentListContent();
	
	// Display Icon Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEICON && !pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEICON, NULL, vListContent);
		bHandled = true;
	}

	// Display Boxart Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEBOXART && !pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEBOXART, NULL, vListContent);
		bHandled = true;
	}

	// Display Banner Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEBANNER && !pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEBANNER, NULL, vListContent);
		bHandled = true;
	}

	// Display Background Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEBACKGROUND && !pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEBACKGROUND, NULL, vListContent);
		bHandled = true;
	}

	// Display Screenshot1 Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMESCREENSHOT1 && !pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMESCREENSHOT1, NULL, vListContent);
		bHandled = true;
	}

	// Display Screenshot2 Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMESCREENSHOT2 && !pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMESCREENSHOT2, NULL, vListContent);
		bHandled = true;
	} 

	// Display Favorites Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEFAVORITE && !pGetSourceImageData->bItemData) {
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEFAVORITE, NULL, vListContent);
		bHandled = true;
	}

	// Display Game Rating Star 1
	if(pGetSourceImageData->iData == FSDIMAGE_GAMERATINGSTAR1 && !pGetSourceImageData->bItemData) {
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMERATINGSTAR1, NULL, vListContent);
		bHandled = true;
	}

	// Display Game Rating Star 2
	if(pGetSourceImageData->iData == FSDIMAGE_GAMERATINGSTAR2 && !pGetSourceImageData->bItemData) {
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMERATINGSTAR2, NULL, vListContent);
		bHandled = true;
	}

	// Display Game Rating Star 3
	if(pGetSourceImageData->iData == FSDIMAGE_GAMERATINGSTAR3 && !pGetSourceImageData->bItemData) {
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMERATINGSTAR3, NULL, vListContent);
		bHandled = true;
	}

	// Display Game Rating Star 4
	if(pGetSourceImageData->iData == FSDIMAGE_GAMERATINGSTAR4 && !pGetSourceImageData->bItemData) {
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMERATINGSTAR4, NULL, vListContent);
		bHandled = true;
	}

	// Display Game Rating Star 5
	if(pGetSourceImageData->iData == FSDIMAGE_GAMERATINGSTAR5 && !pGetSourceImageData->bItemData) {
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMERATINGSTAR5, NULL, vListContent);
		bHandled = true;
	}

	return S_OK;
}

HRESULT CScnGameView::OnRefreshContent(FSDMessageRefreshContent *pRefreshContent, BOOL& bHandled)
{
	// Update local variable with current GameListItem Pointer
	pCurrentGame = pRefreshContent->pGameItem;

	// If the pointer is NULL, or contains a NULL ContentItem Refrence, dont fill
	if(pRefreshContent->pGameItem != NULL && pRefreshContent->pGameItem->ContentRef != NULL)
	{
		// Update all of our assets
		SetTitle(*pRefreshContent->pGameItem);
		SetDescription(*pRefreshContent->pGameItem);
		SetGenre(*pRefreshContent->pGameItem);
		SetReleaseDate(*pRefreshContent->pGameItem);
		SetRaters(*pRefreshContent->pGameItem);
		SetRating(*pRefreshContent->pGameItem);
		SetDeveloper(*pRefreshContent->pGameItem);
	}
	else
	{
		GameListItem pContent(NULL);
		// Update all of our assets
		SetTitle(pContent);
		SetDescription(pContent);
		SetGenre(pContent);
		SetReleaseDate(pContent);
		SetRaters(pContent);
		SetRating(pContent);
		SetDeveloper(pContent);
	}
	//SetSceneTitle();
	SetSceneSubTitle(GameContentManager::getInstance().isDisplayFavorites());
	SetGameListCounter(pRefreshContent->nCurrentIndex, pRefreshContent->nGameListSize);
	bHandled = true;
	return S_OK;
}

HRESULT CScnGameView::OnNotifySortChange(FSDMessageNotifySortChange *pNotifySortChange, BOOL& bHandled)
{
	// Message received stating that the sort type has changed
	SetSortStyleCaptions((GAMELIST_SORTSTYLE)pNotifySortChange->nSortStyle);
	SetSortDirectionCaptions((GAMELIST_SORTDIRECTION)pNotifySortChange->nSortDirection);
	SetFavoritesButtonCaption((GAMELIST_SORTDIRECTION)pNotifySortChange->bFavoritesOnly);

	bHandled = true;
	return S_OK;
}

void CScnGameView::ReadSettings()
{
	// Extract the settings from the Skin.xml Settings Map

	// Setting Tag: <LEFTSHIFT></LEFTSHIFT>
	// Values:  (Any valid VK_KEY)
	// Description:  Setting this to a VK_KEY will cause that press to run an animation
	//				 used in shifting the menus to a second set of buttons.
	//				 Note:  Whatever key you set here will be ignored throughout scene
	GameViewSettings.m_wLeftShiftKey = StringToVKey(GetSetting("LEFTSHIFT", "VK_PAD_LSHOULDER"));

	// Setting Tag: <RIGHTSHIFT></RIGHTSHIFT>
	// Values:  (Any valid VK_KEY)
	// Description:  Setting this to a VK_KEY will cause that press to run an animation
	//				 used in shifting the menus to a second set of buttons
	//				 Note:  Whatever key you set here will be ignored throughout scene
	GameViewSettings.m_wRightShiftKey = StringToVKey(GetSetting("RIGHTSHIFT", "VK_PAD_RSHOULDER"));

	// Setting Tag: <RELEASEDATE></RELEASEDATE>
	// Values:	AUTOHIDE, ON
	// Description:   Setting "AUTOHIDE" will hide the XuiControl GameReleaseDate
	//				  when not on the XBLA Content List, "ON" will cause the release
	//				  date to always be visible to the end-user
	strcmp(GetSetting("RELEASEDATE", "AUTOHIDE").c_str(), "ON") == 0 ? 
		GameViewSettings.m_bAutoHideReleaseDate = false : GameViewSettings.m_bAutoHideReleaseDate = true;

	// Setting Tag: <SORTTYPEDISPLAY></SORTTYPEDISPLAY>
	// Values:	CURRENT, NEXT
	// Description:   Setting "CURRENT" will cause the sorting buttons to display the
	//				  the currently selected sort type when pressed.  Setting "NEXT" 
	//				  will cause the sorting buttons to display the next sort type when pressed.
	//strcmp(GetSetting("SORTTYPEDISPLAY", "CURRENT").c_str(), "NEXT") == 0 ?
	//	GameViewSettings.m_bUseCurrentSortType = false : GameViewSettings.m_bUseCurrentSortType = true;

	// Setting Tag: <LISTCOUNTERFORMAT></LISTCOUNTERFORMAT>
	// Values:	(Custom String)  %current, %total
	// Description:   This xml setting allows a custom string.  Use %current to represent
	//				  the currently selected game index and use %total to represent
	//				  the total number of games in the list
	GameViewSettings.m_szListCounterFormat = strtowstr(GetSetting("LISTCOUNTERFORMAT", "%current of %total"));
}

void CScnGameView::InitializeGameXuiControls( void )
{
	HRESULT retVal = NULL;

	retVal = GetChildById(L"GameDescription", &GameXuiControls.m_GameDescription);
	GameXuiControlState.hasDescription = retVal == S_OK;

	retVal = GetChildById(L"GameTitle", &GameXuiControls.m_GameTitle);
	GameXuiControlState.hasTitle = retVal == S_OK;

	retVal = GetChildById(L"GameGenre", &GameXuiControls.m_GameGenre);
	GameXuiControlState.hasGenre = retVal == S_OK;

	retVal = GetChildById(L"GameReleaseDate", &GameXuiControls.m_GameReleaseDate);
	GameXuiControlState.hasReleaseDate = retVal == S_OK;

	retVal = GetChildById(L"ReleaseDateCaption", &GameXuiControls.m_GameReleaseDateCaption);
	GameXuiControlState.hasReleaseDateCaption = retVal == S_OK;

	retVal = GetChildById(L"GameRating", &GameXuiControls.m_GameRating);
	GameXuiControlState.hasRating = retVal == S_OK;

	retVal = GetChildById(L"GameRaters", &GameXuiControls.m_GameRaters);
	GameXuiControlState.hasRaters = retVal == S_OK;

	retVal = GetChildById(L"GameDeveloper", &GameXuiControls.m_GameDeveloper);
	GameXuiControlState.hasDeveloper = retVal == S_OK;

	retVal = GetChildById(L"GameFullscreenVideo", &GameXuiControls.m_GameFullscreenVideo);
	GameXuiControlState.hasFullScreenVideo = retVal == S_OK;

	retVal = GetChildById(L"GamePreviewVideo", &GameXuiControls.m_GamePreviewVideo);
	GameXuiControlState.hasPreviewVideo = retVal == S_OK;

}

void CScnGameView::InitializeSceneXuiControls( void )
{
	HRESULT retVal = NULL;

	retVal = GetChildById(L"ButtonLaunchGame", &SceneXuiControls.m_ButtonLaunchGame);
	SceneXuiControlState.hasButtonLaunchGame = retVal == S_OK;

	retVal = GetChildById(L"ButtonBack", &SceneXuiControls.m_ButtonBack);
	SceneXuiControlState.hasButtonBack = retVal == S_OK;

	retVal = GetChildById(L"ButtonAdjustRatingUp", &SceneXuiControls.m_ButtonAdjustRatingUp);
	SceneXuiControlState.hasButtonAdjustRatingUp = retVal == S_OK;

	retVal = GetChildById(L"ButtonAdjustRatingDown", &SceneXuiControls.m_ButtonAdjustRatingDown);
	SceneXuiControlState.hasButtonAdjustRatingDown = retVal == S_OK;

	retVal = GetChildById(L"ButtonToggleFavorite", &SceneXuiControls.m_ButtonToggleFavorite);
	SceneXuiControlState.hasButtonToggleFavorite = retVal == S_OK;

	retVal = GetChildById(L"ButtonToggleSubtitle", &SceneXuiControls.m_ButtonToggleSubtitle);
	SceneXuiControlState.hasButtonToggleSubtitle = retVal == S_OK;

	retVal = GetChildById(L"ButtonToggleSortStyle", &SceneXuiControls.m_ButtonToggleSortStyle);
	SceneXuiControlState.hasButtonToggleSortStyle = retVal == S_OK;

	retVal = GetChildById(L"ButtonToggleSortDirection", &SceneXuiControls.m_ButtonToggleSortDirection);
	SceneXuiControlState.hasButtonToggleSortDirection = retVal == S_OK;

	retVal = GetChildById(L"ButtonShowFavorites", &SceneXuiControls.m_ButtonShowFavorites);
	SceneXuiControlState.hasButtonShowFavorites = retVal == S_OK;

	retVal = GetChildById(L"ButtonShowOptions", &SceneXuiControls.m_ButtonShowOptions);
	SceneXuiControlState.hasButtonShowOptions = retVal == S_OK;

	retVal = GetChildById(L"ButtonShiftLB", &SceneXuiControls.m_ButtonShiftLB);
	SceneXuiControlState.hasButtonShiftLB = retVal == S_OK;

	retVal = GetChildById(L"ButtonShiftRB", &SceneXuiControls.m_ButtonShiftRB);
	SceneXuiControlState.hasButtonShiftRB = retVal == S_OK;

	retVal = GetChildById(L"TextSceneCaption", &SceneXuiControls.m_TextSceneCaption);
	SceneXuiControlState.hasTextSceneCaption = retVal == S_OK;

	retVal = GetChildById(L"TextSceneSubCaption", &SceneXuiControls.m_TextSceneSubCaption);
	SceneXuiControlState.hasTextSceneSubCaption = retVal == S_OK;

	retVal = GetChildById(L"TextGameListCounter", &SceneXuiControls.m_TextGameListCounter);
	SceneXuiControlState.hasTextGameListCounter = retVal == S_OK;

	retVal = GetChildById(L"TextCurrentSortStyle", &SceneXuiControls.m_TextCurrentSortStyle);
	SceneXuiControlState.hasTextCurrentSortStyle = retVal == S_OK;

	retVal = GetChildById(L"TextPrevSortStyle", &SceneXuiControls.m_TextPrevSortStyle);
	SceneXuiControlState.hasTextPrevSortStyle = retVal == S_OK;

	retVal = GetChildById(L"TextNextSortStyle", &SceneXuiControls.m_TextNextSortStyle);
	SceneXuiControlState.hasTextNextSortStyle = retVal == S_OK;

	retVal = GetChildById(L"TextCurrentSortDirection", &SceneXuiControls.m_TextCurrentSortDirection);
	SceneXuiControlState.hasTextCurrentSortDirection = retVal == S_OK;

	retVal = GetChildById(L"TextPrevSortDirection", &SceneXuiControls.m_TextPrevSortDirection);
	SceneXuiControlState.hasTextPrevSortDirection = retVal == S_OK;

	retVal = GetChildById(L"TextNextSortDirection", &SceneXuiControls.m_TextNextSortDirection);
	SceneXuiControlState.hasTextNextSortDirection = retVal == S_OK;

}

LPCWSTR CScnGameView::GetSortDirectionText( GAMELIST_SORTDIRECTION nSortDirection )
{
	switch (nSortDirection)
	{
	case GAMELIST_SORTDIRECTION_FORWARD:
		return L"Forward";
		break;
	case GAMELIST_SORTDIRECTION_BACKWARD:
		return L"Backward";
		break;
	}

	return L"Unknown";
}

LPCWSTR CScnGameView::GetSortStyleText( GAMELIST_SORTSTYLE nSortStyle)
{
	switch (nSortStyle) 
	{
	case GAMELIST_SORTSTYLE_ALPHA:
		return L"Title";
		break;
	case GAMELIST_SORTSTYLE_RATING:
		return L"Rating"; 
		break;
	case GAMELIST_SORTSTYLE_RATERS:
		return L"# of Ratings";
		break;
	case GAMELIST_SORTSTYLE_RELEASEDATE:
		return L"Release Date";
		break;
	case GAMELIST_SORTSTYLE_LASTPLAYEDDATE:
		return L"Last Played";
		break;
	case GAMELIST_SORTSTYLE_USERRATING:
		return L"User Rating";
		break;
	}
	return L"Title";
}

wstring CScnGameView::GetSceneTitle( BOOL showingFavs )
{
	wstring szTitleName;

	SkinXMLReader xmlReader;
	xmlReader.LoadSettings("ScnGameView", "Captions");

	szTitleName = strtowstr(xmlReader.GetSetting("DEFAULT", "Freestyle Dash"));

	string szFavoritesTitle = "";
	if(showingFavs == TRUE)
		szFavoritesTitle = " - Favorites";
	else
		szFavoritesTitle = "";

	switch (GameViewSettings.nContentType)
	{
	case 1:
		szTitleName = strtowstr(xmlReader.GetSetting("XBLA", "Freestyle Dash").append(szFavoritesTitle));
		break;
	case 2 :
		szTitleName = strtowstr(xmlReader.GetSetting("XBOX360", "Xbox 360 Games").append(szFavoritesTitle));
		break;
	case 3 :
		szTitleName = strtowstr(xmlReader.GetSetting("XBOXCLASSIC", "Xbox Classic Games").append(szFavoritesTitle));
		break;
	case 4 :
		szTitleName = strtowstr(xmlReader.GetSetting("HOMEBREW", "Homebrew").append(szFavoritesTitle));
		break;
	case 5 :
		szTitleName = strtowstr(xmlReader.GetSetting("EMULATORS", "Emulators").append(szFavoritesTitle));
		break;
	case 7:
		szTitleName = strtowstr(xmlReader.GetSetting("XLINKKAI", "Xlink Kai Games").append(szFavoritesTitle));
		break;
	};

	return szTitleName;
}

void CScnGameView::SetFavoritesButtonCaption(BOOL showingFavorites)
{
	if(showingFavorites)
	{
		if(SceneXuiControlState.hasButtonShowFavorites)
			SceneXuiControls.m_ButtonShowFavorites.SetText(L"Show All");
	}
	else
	{		
		if(SceneXuiControlState.hasButtonShowFavorites)
			SceneXuiControls.m_ButtonShowFavorites.SetText(L"Show Favs");
	}
}

void CScnGameView::SetSceneSubTitle( BOOL showingFavs )
{
	if(SceneXuiControlState.hasTextSceneSubCaption )
		SceneXuiControls.m_TextSceneSubCaption.SetText(showingFavs == TRUE ? L"Favorites" : L"" );
}

void CScnGameView::SetSceneTitle( BOOL showingFavs )
{
	if(SceneXuiControlState.hasTextSceneCaption)
		SceneXuiControls.m_TextSceneCaption.SetText(GetSceneTitle(FALSE).c_str());		
}

void CScnGameView::SetTitle(GameListItem& pContent)
{
	if(GameXuiControlState.hasTitle){}
		GameXuiControls.m_GameTitle.SetText(pContent.Content.m_szTitle);
}

void CScnGameView::SetDeveloper(GameListItem& pContent)
{
	if(GameXuiControlState.hasDeveloper){}
	GameXuiControls.m_GameDeveloper.SetText(pContent.Content.m_szDeveloper);
}

void CScnGameView::SetDescription(GameListItem& pContent)
{
	if(GameXuiControlState.hasDescription) {
		GameXuiControls.m_GameDescription.SetText(pContent.Content.m_szDescription);
	}
}

void CScnGameView::SetGenre(GameListItem& pContent)
{
	if(GameXuiControlState.hasGenre)
		GameXuiControls.m_GameGenre.SetText(pContent.Content.m_szGenre);
}

void CScnGameView::SetReleaseDate(GameListItem& pContent)
{
	if(GameXuiControlState.hasReleaseDate)
		GameXuiControls.m_GameReleaseDate.SetText(pContent.Content.m_szReleaseDate);
}

void CScnGameView::SetRating(GameListItem& pContent)
{
	if(GameXuiControlState.hasRating)
		GameXuiControls.m_GameRating.SetText(pContent.Content.m_szRating);
}

void CScnGameView::SetRaters(GameListItem& pContent)
{
	if(GameXuiControlState.hasRaters)
		GameXuiControls.m_GameRaters.SetText(pContent.Content.m_szRaters);
}

void CScnGameView::SetGameListCounter(int nCurrentIndex, int nListSize)
{
	if(SceneXuiControlState.hasTextGameListCounter){
		wstring szBuffer = GameViewSettings.m_szListCounterFormat;
		wstring current = sprintfaW(L"%d", nCurrentIndex + 1);
		wstring total = sprintfaW(L"%d", nListSize);
		szBuffer = str_replaceallW(szBuffer, L"%current", current);
		szBuffer = str_replaceallW(szBuffer, L"%total", total);
		SceneXuiControls.m_TextGameListCounter.SetText(szBuffer.c_str());
	}
}

void CScnGameView::SetSortStyleCaptions(GAMELIST_SORTSTYLE nCurrentSortStyle)
{
	GAMELIST_SORTSTYLE nPrevSortStyle;
	GAMELIST_SORTSTYLE nNextSortStyle;

	// Determine NextSortStyle from CurrentSortStyle
	if(nCurrentSortStyle == GAMELIST_SORTSTYLE_LAST) 
		nNextSortStyle = GAMELIST_SORTSTYLE_FIRST;
	else 
		nNextSortStyle = (GAMELIST_SORTSTYLE)((int)nCurrentSortStyle + 1);

	// Determine PrevSortStyle from CurrentSortStyle
	if(nCurrentSortStyle == GAMELIST_SORTSTYLE_FIRST)
		nPrevSortStyle = GAMELIST_SORTSTYLE_LAST;
	else
		nPrevSortStyle = (GAMELIST_SORTSTYLE)((int)nCurrentSortStyle - 1);

	// Update XUI Controls to show proper text
	if(SceneXuiControlState.hasTextCurrentSortStyle)
		SceneXuiControls.m_TextCurrentSortStyle.SetText(GetSortStyleText(nCurrentSortStyle));

	if(SceneXuiControlState.hasTextPrevSortStyle)
		SceneXuiControls.m_TextPrevSortStyle.SetText(GetSortStyleText(nPrevSortStyle));

	if(SceneXuiControlState.hasTextNextSortStyle)
		SceneXuiControls.m_TextNextSortStyle.SetText(GetSortStyleText(nNextSortStyle));

	if(SceneXuiControlState.hasButtonToggleSortStyle)
		SceneXuiControls.m_ButtonToggleSortStyle.SetText(GetSortStyleText(nCurrentSortStyle));
}

void CScnGameView::SetSortDirectionCaptions(GAMELIST_SORTDIRECTION nCurrentSortDirection)
{
	GAMELIST_SORTDIRECTION nPrevSortDirection;
	GAMELIST_SORTDIRECTION nNextSortDirection;

	// Determine NextSortDirection from CurrentSortDirection
	if(nCurrentSortDirection == GAMELIST_SORTDIRECTION_LAST) 
		nNextSortDirection = GAMELIST_SORTDIRECTION_FIRST;
	else 
		nNextSortDirection = (GAMELIST_SORTDIRECTION)((int)nCurrentSortDirection + 1);

	
	// Determine PrevSortDirection from CurrentSortDirection
	if(nCurrentSortDirection == GAMELIST_SORTDIRECTION_FIRST)
		nPrevSortDirection = GAMELIST_SORTDIRECTION_LAST;
	else
		nPrevSortDirection = (GAMELIST_SORTDIRECTION)((int)nCurrentSortDirection - 1);

	// Update XUI Controls to show proper text
	if(SceneXuiControlState.hasTextCurrentSortDirection)
		SceneXuiControls.m_TextCurrentSortDirection.SetText(GetSortDirectionText(nCurrentSortDirection));

	if(SceneXuiControlState.hasTextPrevSortDirection)
		SceneXuiControls.m_TextPrevSortDirection.SetText(GetSortDirectionText(nPrevSortDirection));

	if(SceneXuiControlState.hasTextNextSortDirection)
		SceneXuiControls.m_TextNextSortDirection.SetText(GetSortDirectionText(nNextSortDirection));

	if(SceneXuiControlState.hasButtonToggleSortDirection)
		SceneXuiControls.m_ButtonToggleSortDirection.SetText(GetSortDirectionText(nCurrentSortDirection));
}

void CScnGameView::ToggleBackground( void )
{
	if (!GameContentManager::getInstance().GetControlsAreHidden()) 
	{
		GameViewSettings.m_vControlsVector.clear();

		CXuiElement parent;
		if (GetParent(&parent) == S_OK) {
			CXuiElement child;
			if (parent.GetFirstChild(&child) == S_OK) {
				XUIClass cl;
				XuiGetClass(child.GetClass(),&cl);

				if (wcscmp(cl.szClassName, L"ScnCanvasPresenter") != 0 && child.IsShown()) {
					GameViewSettings.m_vControlsVector.push_back(child);
					child.SetShow(false);
				}
				else
					parent.SetShow(true);

				while(child.GetNext(&child) == S_OK) 
				{
					XuiGetClass(child.GetClass(),&cl);

					if (wcscmp(cl.szClassName, L"ScnCanvasPresenter") != 0 && child.IsShown()) 
					{
						GameViewSettings.m_vControlsVector.push_back(child);
						child.SetShow(false);
					}
					else
						parent.SetShow(true);
				}
			}
			GameContentManager::getInstance().SetControlsAreHidden(true);
			DebugMsg("ScnGameView", "invisible [%X]", this);
		}
		else 
		{
			for(unsigned int i =0; i < GameViewSettings.m_vControlsVector.size(); i++) 
			{
				GameViewSettings.m_vControlsVector[i].SetShow(true);
			}
			GameContentManager::getInstance().SetControlsAreHidden(false);
		}
	}
}
