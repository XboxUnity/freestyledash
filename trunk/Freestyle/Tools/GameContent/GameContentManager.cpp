#pragma once
#include "stdafx.h"
#include "GameContentManager.h"

#include "../Managers/Skin/SkinManager.h"

#include "./QueueThreads/FileQueue.h"
#include "./QueueThreads/FileTexQueue.h"
#include "./QueueThreads/TextureQueue.h"
#include "./QueueThreads/ReleaseQueue.h"
#include "./WorkerThreads/FileWorker.h"
#include "./WorkerThreads/FileTexWorker.h"
#include "./WorkerThreads/TextureWorker.h"
#include "./WorkerThreads/ReleaseWorker.h"

#include "../Managers/GameList/GameListItem.h"
#include "../Managers/GameList/GameListManager.h"
#include "../../Tools/SQLite/FSDSql.h"

GameContentManager::GameContentManager() 
{
	// Initialize all variables and pointers
	m_bListReady = false;
	m_bAttachedList = false;
	m_cGameList = NULL;
	m_sTabId = CONTENT_UNKNOWN;
	m_bDisplayBackground = false;
	m_bDisplayScreenshots = true;
	m_bDisplayVideos = false;
	
	m_bDisplayFavorites = false;
	
	m_sListState.SortDirection= GAMELIST_SORTDIRECTION_FIRST;
	m_sListState.SortStyle = GAMELIST_SORTSTYLE_FIRST;
	m_sListState.SubTitleView = GAMELIST_SUBTITLEVIEW_FIRST;

	// Initialize all settings
	m_sListSettings.LoadingBannerPath = "";
	m_sListSettings.LoadingBoxartPath = "";
	m_sListSettings.LoadingIconPath = "";
	m_sListSettings.LoadingScreenshotPath = "";
	m_sListSettings.MissingBannerPath = "";
	m_sListSettings.MissingBoxartPath = "";
	m_sListSettings.MissingIconPath = "";
	m_sListSettings.MissingScreenshotPath = "";
	m_sListSettings.StarFavoritePath = "";
	for(int nCount = 0; nCount < FSDRATING_MAXSTARCOUNT; nCount++)
		m_sListSettings.StarRatingPath[nCount] = "";

	// Define the null list state
	m_sNullState.CurrentGame = NULL;
	m_sNullState.CurrentIndex = -1;
	m_sNullState.ListSize = 0;
	m_sNullState.PreviousGame = NULL;
	m_sNullState.PreviousIndex = -1;
	m_sNullState.SortDirection = GAMELIST_SORTDIRECTION_FIRST;
	m_sNullState.SortStyle = GAMELIST_SORTSTYLE_FIRST;
	m_sNullState.SubTitleView = GAMELIST_SUBTITLEVIEW_FIRST;

	// Initialize the list state/snapshot
	m_sListState = m_sNullState;
	m_sListSnapshot = m_sNullState;

	// Load skin settings.
	ReadSkinSettings();

}

void GameContentManager::DeleteGameFromList(GameListItem* pGameContent)
{
	int idx = GameListManager::getInstance().FindGameIndex(m_sTabId, pGameContent);
	DebugMsg("GameContentManager", "Deleting %s(%d) From GameList", wstrtostr(pGameContent->ContentRef->getTitle()).c_str(), idx);
	GameListManager::getInstance().DeleteGameAt(m_sTabId, idx);
	
//	m_cGameList->SetListSize(m_sListState.ListSize - 1);
//	m_cGameList->DeleteItems(idx,1);

	m_sListState.ListSize--;
//	if(m_sListState.ListSize <= 0) {
//		m_sListState.ListSize = 0;
//		return;
//	}

	// Send a message to the list box to update the list vector
	setListContent(m_sTabId, m_bDisplayFavorites, true);

//	m_sListState.PreviousGame = NULL;
//	m_cGameList->SetCurSel(m_sListState.CurrentIndex - 1);
//	m_sPreviousSortGame = NULL;
//	m_cGameList->RefreshCurrentItem();
}

HRESULT GameContentManager::InitializeGameList( CONTENT_ITEM_TAB sTabId )
{
	if(m_bAttachedList == true) {
		// Do what is necessary to initialize the game list
		StartLoaderThread();
		StartContentThread();
	}
	else
	{
		DebugMsg("GameContentManager", "Failed to populate games list because no CGamesList was attached");
	}
	return S_OK;
}

HRESULT GameContentManager::HandleCompleteAttach( )
{

	//  Start up processing threads and queues
	FileQueue::getInstance().Startup();
	FileTexQueue::getInstance().Startup();
	TextureQueue::getInstance().Startup();
	ReleaseQueue::getInstance().Startup();

	// Load all the static textures to be used in the scene
	LoadStaticTextures();

		getInstance().m_sLoadingIcon.AddRef();
		FileTexQueue::getInstance().AddToQueue(&getInstance().m_sLoadingIcon);
		getInstance().m_sLoadingBoxArt.AddRef();
		FileTexQueue::getInstance().AddToQueue(&getInstance().m_sLoadingBoxArt);
		getInstance().m_sLoadingBanner.AddRef();
		FileTexQueue::getInstance().AddToQueue(&getInstance().m_sLoadingBanner);
		getInstance().m_sLoadingScreenshot.AddRef();
		FileTexQueue::getInstance().AddToQueue(&getInstance().m_sLoadingScreenshot);
		// Load 'Missing' Images Into Memory for use
		getInstance().m_sMissingIcon.AddRef();
		FileTexQueue::getInstance().AddToQueue(&getInstance().m_sMissingIcon);
		getInstance().m_sMissingBoxArt.AddRef();
		FileTexQueue::getInstance().AddToQueue(&getInstance().m_sMissingBoxArt);
		getInstance().m_sMissingBanner.AddRef();
		FileTexQueue::getInstance().AddToQueue(&getInstance().m_sMissingBanner);
		getInstance().m_sMissingScreenshot.AddRef();
		FileTexQueue::getInstance().AddToQueue(&getInstance().m_sMissingScreenshot);

		getInstance().m_sFavoritesStar.AddRef();
		FileTexQueue::getInstance().AddToQueue(&getInstance().m_sFavoritesStar);

		for(int i = 0; i < FSDRATING_MAXSTARCOUNT; i++)
		{
			getInstance().m_vUserRatingTextures.at(i)->AddRef();
			FileTexQueue::getInstance().AddToQueue(getInstance().m_vUserRatingTextures.at(i));
		}

		return S_OK;
}

HRESULT GameContentManager::AttachGameList( CGamesList& GameList, CONTENT_ITEM_TAB sTabId )
{
	//  Only attach a list if a list is not already attached
	if(m_bAttachedList == true || m_cGameList != NULL) {
		DebugMsg("GameContentManager", "AttachGameList:  GameList has already been attached... returning.");
		return S_FALSE;
	}
	
	//  Clear any previously saved list snapshots
	clearGameListSnapshot();

	//  Clear any stored GameListItems for the specified TabID
	GameListManager::getInstance().ClearList(sTabId);

//	#ifdef DEBUG_VERBOSE
//		//  Obtain GameListID to ensure proper pointer is obtained
//		LPCWSTR szListID;
//		if(GameList->GetId(&szListID) != S_OK)
//			szListID = L"Unavailable";
//
//		DebugMsg(L"GameContentManager", L"Attached List- '%s' at address:  %08X", szListID, GameList);
//	#endif

	ReadUserSettings();

	//  Update private variables and pointers for later use
	m_cGameList = &GameList;
	m_bAttachedList = true;

	// Set internal content type
	setGameListTabId(sTabId);
	return S_OK;
}

HRESULT GameContentManager::DetachGameList( void )
{
	// Only detach a list if a current list is attached
	if(m_bAttachedList == false || m_cGameList == NULL) {
		DebugMsg("GameContentManager", "AttachGameList:  GameList has already been attached... returning.");
		return S_FALSE;
	}

	clearGameListSnapshot();
	clearGameListAssets();
	
	// Reset some global variables
	m_bListReady = false;

	FileQueue::getInstance().Pause();
	FileQueue::getInstance().ClearQueue();
	FileQueue::getInstance().TerminateAll();

	TextureQueue::getInstance().Pause();
	TextureQueue::getInstance().ClearQueue();
	TextureQueue::getInstance().TerminateAll();

	FileTexQueue::getInstance().Pause();
	FileTexQueue::getInstance().ClearQueue();
	FileTexQueue::getInstance().TerminateAll();

	int nListSize = GameListManager::getInstance().GetListSize(m_sTabId);

	// Store List Size to loop through each content item::
	for(int nIndex = 0; nIndex < nListSize; nIndex++)
	{
		// Obtain GameList pointer so we can add it to the Release Queue
		GameListItem * pContent = &GameListManager::getInstance().GetGameAt(m_sTabId, nIndex);
		
		// Add m_GameIconTexture to Release Queue
		pContent->Textures.m_IconTexture->Release();
		pContent->Textures.m_BoxartTexture->Release();
		pContent->Textures.m_BannerTexture->Release();
	}

	m_sMissingIcon.ReleaseAll();
	m_sMissingBoxArt.ReleaseAll();
	m_sMissingBanner.ReleaseAll();
	m_sMissingScreenshot.ReleaseAll();
	m_sLoadingIcon.ReleaseAll();
	m_sLoadingBoxArt.ReleaseAll();
	m_sLoadingBanner.ReleaseAll();
	m_sLoadingScreenshot.ReleaseAll();

	m_sFavoritesStar.ReleaseAll();
	for(int i = 0; i < (int) m_vUserRatingTextures.size(); i++)
		delete m_vUserRatingTextures.at(i);

	m_vUserRatingTextures.clear();

	ReleaseQueue::getInstance().Pause();
	ReleaseQueue::getInstance().ClearQueue();
	ReleaseQueue::getInstance().TerminateAll();

	// Clear Game List for specified content list
	GameListManager::getInstance().ClearList(m_sTabId);

	// Extract GameListID from control to make sure we have proper pointer
	#ifdef DEBUG_VERBOSE
		LPCWSTR szListID;
		if(m_cGameList->GetId(&szListID) != S_OK)
			szListID = L"Unavailable";one v

		DebugMsg(L"GameContentManager", L"Detached List- '%s' at address:  %08X", szListID, m_cGameList);
	#endif

	TerminateContentThread();
	TerminateLoaderThread();
	
	// Reset all the internal variables
	m_sListState = m_sNullState;
	m_sPreviousSortGame = NULL;
	m_bAttachedList = false;
	m_bDisplayFavorites = false;
	
	m_cGameList = NULL;
	m_sTabId = CONTENT_UNKNOWN;
	return S_OK;
}

void GameContentManager::OnContentAdded(ContentItemNew * pContent)
{
	bool kaiMode = false;
	if(m_sTabId == CONTENT_XLINKKAI) kaiMode = true;
	
	if((pContent->GetItemTab() != m_sTabId && kaiMode == false) || pContent == NULL)
		return;

	if(kaiMode == true && pContent->isKaiCompatible() == FALSE)
		return;

	while(!m_bListReady) {
		Sleep(0);
	};

	// Create a new GameList item from ContentItem
	GameListItem * pGameContent = new GameListItem(pContent);
	
	// Send the pGameContent to another thread to be processed for image loading
	ContentItemLoadAssets(pGameContent);
	
	// Add add GameListItem to GamesList
	GameListManager::getInstance().AddToList(m_sTabId, *pGameContent);

	// Store the active list for use in updating the game list
	m_vActiveListPointer = GameListManager::getInstance().GetGameList(m_sTabId);

	// Send a message to the list box to update the list vector
	setListContent(m_sTabId, m_bDisplayFavorites, true);

	// Let the game list know that its time to sort again
	FsdNotifySortChange(SkinManager::getInstance().getXurObjectHandle["GameListScene"], m_sListState.SortStyle, m_sListState.SortStyle, m_bDisplayFavorites, true);
}

void GameContentManager::OnContentMassAdd(vector<ContentItemNew *> items)
{
	bool kaiMode = false;
	if(m_sTabId == CONTENT_XLINKKAI) kaiMode = true;
	
	while(!m_bListReady) {
		Sleep(0);
	};

	vector<ContentItemNew*>::iterator iter;
	for(iter = items.begin(); iter != items.end(); ++iter) {
		ContentItemNew* pContent = *iter;
		if((pContent->GetItemTab() != m_sTabId && kaiMode == false) || pContent == NULL)
			continue;

		if(kaiMode == true && pContent->isKaiCompatible() == FALSE)
			continue;

		// Create a new GameList item from ContentItem
		GameListItem * pGameContent = new GameListItem(*iter);

		// since we need some of these things before ContentItemLoadAssets does it's job, set them here
		pGameContent->Flags.m_bFavorite = pContent->getIsFavorite();
		pGameContent->Content.m_nLastPlayedOrder = pContent->getLastPlayedOrder();

		// Send the pGameContent to another thread to be processed for image loading
		ContentItemLoadAssets(pGameContent);

		// Add add GameListItem to GamesList
		GameListManager::getInstance().AddToList(m_sTabId, *pGameContent);
	}

	// Store the active list for use in updating the game list
	m_vActiveListPointer = GameListManager::getInstance().GetGameList(m_sTabId);

	// Send a message to the list box to update the list vector
	setListContent(m_sTabId, m_bDisplayFavorites, true);

	// Let the game list know that its time to sort again
	// FsdNotifySortChange(SkinManager::getInstance().getXurObjectHandle["GameListScene"], m_sListState.SortStyle, m_sListState.SortStyle, m_bDisplayFavorites, true);
	// don't have to do the sort since the list is about to get a "CacheLoaded" msg which will do the sort
}

HRESULT GameContentManager::RefreshCurrentContent( GameListItem * pGameContent, int nCurrentIndex, int nGameListSize )
{
	// Obtain the HXUIOBJ from teh Object Handle Map
	HXUIOBJ hObj = SkinManager::getInstance().getXurObjectHandle["GameListScene"];

	// If the handle is valid, Broadcast the RefreshContent Message to the handle
	if(XuiHandleIsValid(hObj) == 1){
		FsdGameRefreshContent(hObj, pGameContent, nCurrentIndex, nGameListSize);
		return S_OK;
	}

	return S_FALSE;
}

void GameContentManager::OnContentUpdated(ContentItemNew * pContent, DWORD dwContentFlags)
{
	// If the updated ContentItem coming in, is NULL, return
	if(pContent == NULL || m_vActiveListPointer == NULL)
		return;

	GameListIterator iter;
	// Loop through all of the GameListItems within the active list
	for(iter = m_vActiveListPointer->begin(); iter != m_vActiveListPointer->end(); ++iter)
	{
		// Only proceed for the items in the list that matches the updated content
		if((*iter)->ContentRef == pContent) 
		{
			HXUIOBJ hObj;
			GameListItem * pGameContent = *iter;
			switch (dwContentFlags){
				case ITEM_UPDATE_TYPE_DISPLAY_INFO:
					pGameContent->RefreshContentText();
					hObj = SkinManager::getInstance().getXurObjectHandle["GameListScene"];
					FsdGameRefreshContent(hObj, *iter, m_sListState.CurrentIndex, m_sListState.ListSize, true);
					break;
				case ITEM_UPDATE_TYPE_INFO:
					pGameContent->RefreshContentText();
					hObj = SkinManager::getInstance().getXurObjectHandle["GameListScene"];
					FsdGameRefreshContent(hObj, *iter, m_sListState.CurrentIndex, m_sListState.ListSize, true);
					break;
				case ITEM_UPDATE_TYPE_RELEASEDATE:
					pGameContent->RefreshContentText();
					hObj = SkinManager::getInstance().getXurObjectHandle["GameListScene"];
					FsdGameRefreshContent(hObj, *iter, m_sListState.CurrentIndex, m_sListState.ListSize, true);
					break;
				case ITEM_UPDATE_TYPE_BANNER:
					pGameContent->Textures.m_BannerTexture->SetTextureInfo(pGameContent->ContentRef->getBoxArtPath(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2);
					pGameContent->Textures.m_BannerTexture->Release();
					pGameContent->Textures.m_BannerTexture->ReleaseAll();
					pGameContent->Textures.m_BannerTexture->AddRef();
					FileTexQueue::getInstance().AddToQueue(pGameContent->Textures.m_BannerTexture);
					break;
				case ITEM_UPDATE_TYPE_BACKGROUND:
					break;
				case ITEM_UPDATE_TYPE_BOXART:
					pGameContent->Textures.m_BoxartTexture->SetTextureInfo(pGameContent->ContentRef->getBoxArtPath(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2);
					pGameContent->Textures.m_BoxartTexture->Release();
					pGameContent->Textures.m_BoxartTexture->ReleaseAll();
					pGameContent->Textures.m_BoxartTexture->AddRef();
					FileTexQueue::getInstance().AddToQueue(pGameContent->Textures.m_BoxartTexture);
					break;
				case ITEM_UPDATE_TYPE_PREVIEW:
					break;
				case ITEM_UPDATE_TYPE_SCREENSHOT:
					break;
				case ITEM_UPDATE_TYPE_THUMBNAIL:
					pGameContent->Textures.m_IconTexture->SetTextureInfoMemory(D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, TRUE);
					BYTE * iconData;
					DWORD iconSize = pGameContent->ContentRef->getIconData(&iconData);
					pGameContent->Textures.m_IconTexture->Release();
					pGameContent->Textures.m_IconTexture->ReleaseAll();
					pGameContent->Textures.m_IconTexture->SetFilePointer(iconData, iconSize);
					pGameContent->Textures.m_IconTexture->SetCurrentBrushState(FSDTEX_LOADING);
					pGameContent->Textures.m_IconTexture->AddRef();
					TextureQueue::getInstance().AddToQueue(pGameContent->Textures.m_IconTexture);
					break;
			};
		}
	}
}
void GameContentManager::OnContentCacheLoaded( DWORD dwItemCount )
{
	// Cache has completed being sent
	if(dwItemCount == 0)
		return;

	// Obtain the current sort settings
	int nSortStyle = 0, nSortDirection = 0, nSubTitleView;
	//SETTINGS::getInstance().getGameListSort((int)m_sTabId, &nSortStyle, &nSortDirection, &nSubTitleView);
	bool bOnlyFavorites;

	// get the sort settings from the database
	FSDSql::getInstance().getGameListSort((int)m_sTabId, 0, nSortStyle, nSortDirection, nSubTitleView, bOnlyFavorites);

	m_bDisplayFavorites = bOnlyFavorites;

	// Sort the cached items before loading any further items found in scan
	SortGameList((GAMELIST_SORTSTYLE)nSortStyle, (GAMELIST_SORTDIRECTION)nSortDirection, m_vActiveListPointer);

	m_sListState.SortStyle = (GAMELIST_SORTSTYLE)nSortStyle;
	m_sListState.SortDirection = (GAMELIST_SORTDIRECTION)nSortDirection;
	m_sListState.SubTitleView = (GAMELIST_SUBTITLEVIEW)nSubTitleView;

	// except that on 11/27/10 No one is actually listening to the FsdListSetSort message...
	FsdNotifySortChange(SkinManager::getInstance().getXurObjectHandle["GameListScene"], (int)m_sListState.SortStyle, (int)m_sListState.SortDirection, m_bDisplayFavorites);
	//	FsdListSetSort(m_cGameList->m_hObj,  m_sListState.SortStyle, m_sListState.SortDirection, false);
	///	RefreshSortCaptions((int)m_sListState.SortStyle, (int)m_sListState.SortDirection, m_bDisplayFavorites);
	//	m_cGameList->RefreshCurrentItem();
}

void GameContentManager::OnContentDatabaseLoaded( DWORD dwItemCount )
{
	// Database has completed loading
	DebugMsg("GameContentManager", "Database has Loaded");
	
}
void GameContentManager::OnContentScanCompleted( DWORD dwItemCount )
{
	// Scanning has compled
	DebugMsg("GameContentManager", "File Scanning is complete");
}

void GameContentManager::OnContentLoadAssets( GameListItem * pGameContent )
{
	// if the GameListItem is NULL or it's ContentItem reference is NULL, we return
	if(pGameContent == NULL || pGameContent->ContentRef == NULL) 
		return;

	// Update and refresh all the text items
	pGameContent->RefreshContentText();
	
	// Load Icon Images
	if(pGameContent->ContentRef->GetAssetFlags() & CONTENT_ASSET_THUMBNAIL) {
		BYTE * iconData;
		DWORD iconSize = pGameContent->ContentRef->getIconData(&iconData);
		pGameContent->Textures.m_IconTexture->SetTextureInfoMemory(D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, TRUE);
		pGameContent->Textures.m_IconTexture->SetFilePointer(iconData, iconSize);
		pGameContent->Textures.m_IconTexture->SetCurrentBrushState(FSDTEX_BRUSHLOADING);
		pGameContent->Textures.m_IconTexture->AddRef();
		TextureQueue::getInstance().AddToQueue(pGameContent->Textures.m_IconTexture);
	} else {
		pGameContent->Textures.m_IconTexture->SetCurrentTextureState(FSDTEX_TEXERROR);
		pGameContent->Textures.m_IconTexture->SetCurrentBrushState(FSDTEX_BRUSHERROR);
	}

	// Load Boxart Images
	if(pGameContent->ContentRef->GetAssetFlags() & CONTENT_ASSET_BOXART) {
		pGameContent->Textures.m_BoxartTexture->SetTextureInfo(pGameContent->ContentRef->getBoxArtPath(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2);
		pGameContent->Textures.m_BoxartTexture->AddRef();
		FileQueue::getInstance().AddToQueue(pGameContent->Textures.m_BoxartTexture);			
		TextureQueue::getInstance().AddToQueue(pGameContent->Textures.m_BoxartTexture);
	} else {
		pGameContent->Textures.m_BoxartTexture->SetCurrentTextureState(FSDTEX_TEXERROR);
		pGameContent->Textures.m_BoxartTexture->SetCurrentBrushState(FSDTEX_BRUSHERROR);
	}

	// Load Banner Images
	if(pGameContent->ContentRef->GetAssetFlags() & CONTENT_ASSET_BANNER) {
		pGameContent->Textures.m_BannerTexture->SetTextureInfo(pGameContent->ContentRef->getBannerPath(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2);				
		pGameContent->Textures.m_BannerTexture->AddRef();
		FileQueue::getInstance().AddToQueue(pGameContent->Textures.m_BannerTexture);
		TextureQueue::getInstance().AddToQueue(pGameContent->Textures.m_BannerTexture);
	} else {
		pGameContent->Textures.m_BannerTexture->SetCurrentTextureState(FSDTEX_TEXERROR);
		pGameContent->Textures.m_BannerTexture->SetCurrentBrushState(FSDTEX_BRUSHERROR);
	}
}

HRESULT GameContentManager::setListContent(CONTENT_ITEM_TAB sTabId, bool onlyFavorites, bool bRetainSelection)
{
	// If the list is not attached- return false
	if(!m_bAttachedList)
		return S_FALSE;

	// Create a temporary variable to hold the new list
	GameListContentPtr vListContent;

	// Create the appropriate list (standard, or favorites)
	if(onlyFavorites)
		vListContent = GameListManager::getInstance().GetFavoritesList(sTabId);
	else
		vListContent = GameListManager::getInstance().GetGameList(sTabId);

	// Store the pointer globally
	m_vDisplayListPointer = vListContent;

	// Send the message to the list to update it's content
	FsdListSetContent(m_cGameList->m_hObj, vListContent, bRetainSelection);

	// Return Successfully
	return S_OK;
}

HRESULT GameContentManager::getUserRatingVector(int nRatingIn, vector<int> * vRatingOut)
{
	// Return if the vRatingOut vector is invalid
	if(vRatingOut == NULL)
		return S_FALSE;

	// Clear vector
	vRatingOut->clear();

	int nFullStarCount = (int)floor((double)(nRatingIn / 100));	// integer representing number of full stars
	int nPartialStarIndex = (int)floor((double)((nRatingIn % 100) / 25)); // integer representing index for partial star

	// Fill the rating vector with the correct conversions
	for(int nIndex = 0; nIndex < FSDRATING_MAXSTARCOUNT; nIndex++)
	{
		if(nIndex < nFullStarCount)
			vRatingOut->push_back(FSDIMAGE_100STAR);
		else if(nPartialStarIndex > 0 && nIndex == nFullStarCount)
			vRatingOut->push_back(nPartialStarIndex);
		else
			vRatingOut->push_back(FSDIMAGE_000STAR);
	}
	return S_OK;
}

GAMELIST_SORTSTYLE GameContentManager::ToggleSortStyle( void )
{
	// Grab the current Sort Style from the global list state
	GAMELIST_SORTSTYLE nCurSortStyle = m_sListState.SortStyle;

	// If the list is equal to the last item in the list...
	if( nCurSortStyle >= GAMELIST_SORTSTYLE_LAST) {
		// Set the sort style to the beginning
		nCurSortStyle = GAMELIST_SORTSTYLE_FIRST;
	} else {
		// Convert the current sort style to an integer
		int nIndex = (int)nCurSortStyle;
		nIndex++;
		// If the content type is not equal to XBLA
		if(m_sTabId != CONTENT_XBLA) {
			// Then don't include Release date in the sort order
			if((GAMELIST_SORTSTYLE)nIndex == GAMELIST_SORTSTYLE_RELEASEDATE) {
				nIndex++;
			}
		}
		nCurSortStyle = (GAMELIST_SORTSTYLE)nIndex;
	}
	
	// Set the new sort style 
	m_sListState.SortStyle = nCurSortStyle;

	FSDSql::getInstance().setGameListSort(m_sTabId, 0, m_sListState.SortStyle, m_sListState.SortDirection, m_sListState.SubTitleView, m_bDisplayFavorites);

	// Send messages to both the gamelist and the gameview scene to update their components
	FsdNotifySortChange(SkinManager::getInstance().getXurObjectHandle["GameListScene"],  m_sListState.SortStyle, m_sListState.SortDirection, m_bDisplayFavorites, true);

	return nCurSortStyle;
}

GAMELIST_SORTDIRECTION GameContentManager::ToggleSortDirection( void )
{
	// Toggle between Forward and Backward
	GAMELIST_SORTDIRECTION nCurSortDirection = m_sListState.SortDirection;

	if(nCurSortDirection == GAMELIST_SORTDIRECTION_FORWARD)
		nCurSortDirection = GAMELIST_SORTDIRECTION_BACKWARD;
	else
		nCurSortDirection = GAMELIST_SORTDIRECTION_FORWARD;

	 m_sListState.SortDirection = nCurSortDirection;
	
	 FSDSql::getInstance().setGameListSort(m_sTabId, 0, m_sListState.SortStyle, m_sListState.SortDirection, m_sListState.SubTitleView, m_bDisplayFavorites);

	// Send messages to both the gamelist and the gameview scene to update their components
	// except that on 11/27/10 No one is actually listening to the FsdListSetSort message...
	FsdNotifySortChange(SkinManager::getInstance().getXurObjectHandle["GameListScene"],(int) m_sListState.SortStyle, m_sListState.SortDirection, m_bDisplayFavorites, true);

	return nCurSortDirection;

}

GAMELIST_SUBTITLEVIEW GameContentManager::ToggleSubTitleView( void )
{
	// Grab the current SubTitleView from the global list state
	GAMELIST_SUBTITLEVIEW nCurSubTitleView = m_sListState.SubTitleView;
	
	// If the list is equal to the last item in the list...
	if(nCurSubTitleView == GAMELIST_SUBTITLEVIEW_LAST) {
		// Then set it to our first
		nCurSubTitleView = GAMELIST_SUBTITLEVIEW_FIRST;
	} else {
		// Otherwise, increase the index by 1
		int nIndex = (int)nCurSubTitleView;
		nIndex++;

		// Set the subtitleview to the new view type
		nCurSubTitleView = (GAMELIST_SUBTITLEVIEW)nIndex;
	}

	// Update the global list state
	m_sListState.SubTitleView = nCurSubTitleView;

 	FSDSql::getInstance().setGameListSort(m_sTabId, 0, m_sListState.SortStyle, m_sListState.SortDirection, m_sListState.SubTitleView, m_bDisplayFavorites);

	return nCurSubTitleView;
}

BOOL GameContentManager::ToggleDisplayFavorites()
{
	if(m_bAttachedList == false)
		return m_bDisplayFavorites ? TRUE : FALSE;
	bool bRefresh = true;
	
	// Store the currently selected game for later release
	m_sPreviousSortGame = m_cGameList->GetCurrentGame();
	// Toggle the local variable for favorites display
	m_bDisplayFavorites = !m_bDisplayFavorites;

	FSDSql::getInstance().setGameListSort(m_sTabId, 0, m_sListState.SortStyle, m_sListState.SortDirection, m_sListState.SubTitleView, m_bDisplayFavorites);

	if(m_bDisplayFavorites == false && m_cGameList->GetItemCount() == 0)
		bRefresh = true;

	// Update the list DisplayListPointer to show favorites 
	setListContent(m_sTabId, m_bDisplayFavorites);
	// Refresh the currently displayed item
	if(bRefresh)
		m_cGameList->RefreshCurrentItem();

	// Return the current favorites state
	return m_bDisplayFavorites ? TRUE : FALSE;
}

BOOL GameContentManager::ToggleFavorite( GameListItem * pGameContent )
{
	// if the gamelistitem is valid, then toggle the favorite status
	if(pGameContent != NULL && pGameContent->ContentRef != NULL) {
		pGameContent->Flags.m_bFavorite = !pGameContent->Flags.m_bFavorite;
		pGameContent->ContentRef->setFavorite(pGameContent->Flags.m_bFavorite);
		FSDSql::getInstance().updateFavorite(0, pGameContent->ContentRef->GetItemId(), pGameContent->Flags.m_bFavorite);

		return pGameContent->Flags.m_bFavorite ? TRUE : FALSE;
	}

	return -1;
}


HRESULT GameContentManager::UpdateListContent(GAMECONTENT_LIST_STATE listPacket)
{
	if(m_bAttachedList == false)
		return S_FALSE;

	// Send the Play Transition Message to all the scenes if Backgrounds are enabled
	if(m_bDisplayBackground) 
	{
		HXUIOBJ hObj = SkinManager::getInstance().getXurObjectHandle["GameListScene"];
		// If the handle is valid, Broadcast the PlayTransition to the handle
		if(XuiHandleIsValid(hObj) == 1){
			FsdMsgPlayTransition(hObj, true, true);
		}
	}

	// Update internal listState from GamesList
	m_sListState = listPacket;

	// Execute ManageTexture method to load and release textures for newly selected game
	ManageContentTextures();

	// Notify registered classes that the GameList changed index
	RefreshCurrentContent(listPacket.CurrentGame, listPacket.CurrentIndex, listPacket.ListSize);

	return S_OK;
}

HRESULT GameContentManager::SortGameList( GAMELIST_SORTSTYLE nSortStyle, GAMELIST_SORTDIRECTION nSortDirection, GameListContentPtr pContentList)
{
	if(m_bAttachedList == false || pContentList == NULL)
		return S_FALSE;

	// Store the currently selected game for comparison and release later
	m_sPreviousSortGame = m_sListState.CurrentGame;

	switch (nSortStyle) 
	{
	case GAMELIST_SORTSTYLE_ALPHA:
		nSortDirection == GAMELIST_SORTDIRECTION_FORWARD ? GameListManager::getInstance().SortListAlpha(pContentList) :
			GameListManager::getInstance().SortListAlphaReverse(pContentList);
		break;
	case GAMELIST_SORTSTYLE_RATING:
		nSortDirection == GAMELIST_SORTDIRECTION_FORWARD ? GameListManager::getInstance().SortListRating(pContentList) :
			GameListManager::getInstance().SortListRatingReverse(pContentList);
		break;
	case GAMELIST_SORTSTYLE_RATERS:
		nSortDirection == GAMELIST_SORTDIRECTION_FORWARD ? GameListManager::getInstance().SortListRaters(pContentList) :
			GameListManager::getInstance().SortListRatersReverse(pContentList);
		break;
	case GAMELIST_SORTSTYLE_RELEASEDATE:
		nSortDirection == GAMELIST_SORTDIRECTION_FORWARD ? GameListManager::getInstance().SortListReleaseDate(pContentList) :
			GameListManager::getInstance().SortListReleaseDateReverse(pContentList);
		break;
	case GAMELIST_SORTSTYLE_LASTPLAYEDDATE:
		nSortDirection == GAMELIST_SORTDIRECTION_FORWARD ? GameListManager::getInstance().SortListLastPlayedDate(pContentList) :
			GameListManager::getInstance().SortListLastPlayedDateReverse(pContentList);
		break;
	case GAMELIST_SORTSTYLE_USERRATING:
		nSortDirection == GAMELIST_SORTDIRECTION_FORWARD ? GameListManager::getInstance().SortListUserRating(pContentList) :
			GameListManager::getInstance().SortListUserRatingReverse(pContentList);
		break;
	default:
		nSortDirection == GAMELIST_SORTDIRECTION_FORWARD ? GameListManager::getInstance().SortListAlpha(pContentList) :
			GameListManager::getInstance().SortListAlphaReverse(pContentList);
		break;
	};

	// Return successful
	return S_OK;
}

HRESULT GameContentManager::RefreshSortCaptions( int nSortStyle, int nSortDirection, bool bFavoritesOnly )
{
	HXUIOBJ hObj = SkinManager::getInstance().getXurObjectHandle["GameListScene"];
	// If the handle is valid, Broadcast the NotifySortChange to the handle
	if(XuiHandleIsValid(hObj) == 1){
		FsdNotifySortChange(hObj, nSortStyle, nSortDirection, bFavoritesOnly);
		return S_OK;
	}

	return S_FALSE;
}

HRESULT GameContentManager::setUserRating(GameListItem * pGameItem, int nNewRating)
{
	// If the GameListItem is not Null, and it references a valid ContentItem
	if(pGameItem != NULL && pGameItem->ContentRef != NULL)
	{
		// Update the GameItem's Internal Rating and Rating Vector
		pGameItem->Content.m_nUserRating = nNewRating;
		getUserRatingVector(nNewRating, &(pGameItem->Content.m_vUserRating));
		// Submit changes to the database
		pGameItem->ContentRef->setUserRating(nNewRating, true);

		// Return successfully
		return S_OK;
	}
	
	// Return negative
	return S_FALSE;
}

HRESULT GameContentManager::AdjustUserRating( GameListItem * pGameContent, int nMinRating, int nMaxRating, int nAmount )
{
	if(pGameContent != NULL && pGameContent->ContentRef != NULL) {
		int nCurrentRating = (int)pGameContent->ContentRef->getUserRating();
		
		nCurrentRating = nCurrentRating + nAmount;
		bool bIncrease = nAmount > 0;

		if(nCurrentRating >= nMaxRating || nCurrentRating <= nMinRating) {
			if(bIncrease)
				nCurrentRating = nMaxRating;
			else
				nCurrentRating = nMinRating;
		}
		setUserRating(pGameContent, nCurrentRating);

		if (m_sListState.SortStyle == GAMELIST_SORTSTYLE_USERRATING) { 
			int idx = GameListManager::getInstance().FindGameIndex(m_sTabId, pGameContent);
			SortGameList(m_sListState.SortStyle, m_sListState.SortDirection, m_vActiveListPointer);
			int idx2 = GameListManager::getInstance().FindGameIndex(m_sTabId, pGameContent);
			if (idx != idx2) {
				if (!m_cGameList->IsItemVisible(idx2)) {
					m_cGameList->SetTopItem(idx2);
				}
				m_cGameList->SetCurSel(idx2);
			}
		}

		return S_OK;
	}	
	
	return S_FALSE;
}