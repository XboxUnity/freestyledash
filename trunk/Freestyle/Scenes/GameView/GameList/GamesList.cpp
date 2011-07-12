#pragma once
#include "stdafx.h"
#include "GamesList.h"

#include "../../../Tools/Settings/Settings.h"
#include "../../../Application/FreestyleApp.h"
#include "../../../Tools/Managers/VariablesCache/VariablesCache.h"
#include "../../../Tools/Managers/GameList/GameListItem.h"
#include "../../../Tools/Managers/GameList/GameListManager.h"
#include "../../../Tools/Managers/Skin/SkinManager.h"
#include "../../../Tools/GameContent/GameContentManager.h"
#include "../../Abstracts/ConfigurableList/ConfigurableList.h"
#include "../../../Tools/GameContent/GameContentMessages.h"

CGamesList::CGamesList() : m_vListContent(NULL)
{
	// CGamesList Constructor
	m_nListSize = 0;
	m_vListContent = NULL;
}

CGamesList::~CGamesList()
{
	// CGamesList Deconstructor
	// Remove List from GameContentManager
	GameContentManager::getInstance().DetachGameList();
}

HRESULT CGamesList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	flag = false;
	
	// Grab ContentType from VariablesCache for processing
	string szContentType = m_szContentType = VariablesCache::getInstance().getVariable("GameListShowType", "");
	DebugMsg("CGamesList", "Initializing CGamesList (%X), ContentType:  %s", this, szContentType.c_str());

	CONTENT_ITEM_TAB sTabId = ContentManager::getInstance().GetTabIdFromString(szContentType);

	// Attach GamesList to GameViewManager
	GameContentManager::getInstance().AttachGameList(*this, sTabId);

	// Fill private variables with content types for later use
	//GameContentManager::getInstance().GetContentType(&m_szContentType, &m_nContentType);

	// Initalize ListInformation Variables
	m_nCurrentIndex = 0;
	m_nPreviousIndex = 0;
	m_nListSize = 0;
	m_nSortStyle = (GAMELIST_SORTSTYLE)0;
	m_nSortDirection = (GAMELIST_SORTDIRECTION)0;
	m_nSubtitleView = (GAMELIST_SUBTITLEVIEW)0;
	m_CurrentGame = NULL;
	m_PreviousGame = NULL; 

	// Tell GameViewManager to Initialize the GamesList for current specified ContentType
	GameContentManager::getInstance().InitializeGameList(sTabId);

	m_nSortDirection = GameContentManager::getInstance().getGameListState().SortDirection;
	m_nSortStyle = GameContentManager::getInstance().getGameListState().SortStyle;
	m_nSubtitleView = GameContentManager::getInstance().getGameListState().SubTitleView;
	return S_OK;
}


HRESULT CGamesList::OnSetListContent(FSDMessageSetListContent * pSetListContent, BOOL &bHandled )
{
	// Unpack the message and store into local vector
	m_vListContent = pSetListContent->pVec;
	
	if(m_vListContent != NULL)
		SetListSize((int)m_vListContent->size());

	// Local variables used 
	unsigned int nCurrentSel = 0;
	int nCurrentTop = 0;

	// If interested, save current list position for later use
	if(pSetListContent->bRetainSelection){
		
		if(GetListSize() > 0 && GetCurSel() == -1)
			nCurrentSel = 0;
		else
			nCurrentSel = GetCurSel();
		nCurrentTop = GetTopItem();
		if (nCurrentTop == -1)
			nCurrentTop = 0;
	}

	// Clear the current list and prepare the list to use a new data vector
	DeleteItems(0, GetItemCount());
	if(pSetListContent->pVec != NULL){
		InsertItems(0, pSetListContent->pVec->size());
		if( nCurrentTop > pSetListContent->pVec->size() )
			nCurrentTop = pSetListContent->pVec->size();
	}

	// Restore list position
	SetTopItem(nCurrentTop);
	SetCurSel(nCurrentSel);

	if(pSetListContent->pVec != NULL)
		GameContentManager::getInstance().RefreshCurrentContent(m_CurrentGame, nCurrentSel, pSetListContent->pVec->size());

	// Return Successfully
	bHandled = TRUE;
	return S_OK;
}

HRESULT CGamesList::OnSetCurSel(XUIMessageSetCurSel *pSetCurSelData, BOOL &bHandled )
{

	int nFocalPoint = 4;

	if(SETTINGS::getInstance().getGameListVisual() == 0)
		return S_OK;

	if(SETTINGS::getInstance().getGameListVisual() == 1)
		nFocalPoint = 4;
	else if(SETTINGS::getInstance().getGameListVisual() == 2)
		nFocalPoint = 3;
	else
		nFocalPoint = 4;

	if(flag == false)
	{
	
		int nFocalInverse = GetVisibleItemCount() - nFocalPoint - 1;
		
		int previousItem = GetCurSel();
		int currentTopItem = GetTopItem();
		int currentItem = pSetCurSelData->iItem;
		int direction = currentItem - previousItem;
		int listCount = GetItemCount();

		if(direction > 0) 
		{
			currentTopItem++;
			if(currentItem > nFocalPoint && currentItem <= listCount - nFocalInverse - 1)
			{
				SetTopItem(currentTopItem);
				pSetCurSelData->iItem = currentTopItem + nFocalPoint;
				flag = true;
			}
		}
		else
		{
			currentTopItem--;
			if(currentItem >= nFocalPoint && currentItem < listCount - nFocalInverse - 1)
			{
				SetTopItem(currentTopItem);
				pSetCurSelData->iItem = currentTopItem + nFocalPoint;
				flag = true;
			}
		}
	}
	else
	{
		flag = false;
		bHandled = true;
	}

	return S_OK;
}

HRESULT CGamesList::OnNotifySortChanged(FSDMessageNotifySortChange * pNotifySortChange, BOOL &bHandled )
{
	m_nSortDirection = (GAMELIST_SORTDIRECTION)pNotifySortChange->nSortDirection;
	m_nSortStyle = (GAMELIST_SORTSTYLE)pNotifySortChange->nSortStyle;

	if (m_vListContent) {
		GameContentManager::getInstance().SortGameList(m_nSortStyle, m_nSortDirection, m_vListContent);
		FsdGameRefreshContent(SkinManager::getInstance().getXurObjectHandle["GameListScene"], GetCurrentGame(), GetCurSel(), m_vListContent->size());
	}

	RefreshCurrentItem();

	return S_OK;
}

HRESULT CGamesList::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	// Define a update buffer to store listbox state
	GAMECONTENT_LIST_STATE updPack;

	// return FALSE if there is no content in the currently active list
	if(m_vListContent == NULL || m_vListContent->size() == 0)
		return S_FALSE;

	// Extract the Current and Previous Index
	m_nCurrentIndex = pNotifySelChangedData->iItem;
	m_nPreviousIndex = pNotifySelChangedData->iOldItem;

	// Store the list size temporarily
	int nListSize = (int)m_vListContent->size();

	// Current Index should never be less than 0 and larger than nListSize
	if(m_nCurrentIndex >= 0 && m_nCurrentIndex <= nListSize - 1) {
		m_CurrentGame = m_vListContent->at(m_nCurrentIndex);
	} else {
		// This condition *should not* ever happen
		m_CurrentGame = NULL;
		m_nCurrentIndex = -1;
	}

	// Ensure that the previous index is within the range of the vector before accessing
	if(m_nPreviousIndex >= 0 && m_nPreviousIndex <= nListSize - 1) {
		m_PreviousGame = m_vListContent->at(m_nPreviousIndex);
	} else {
		// This condition should only happen when the list is first loaded
		m_PreviousGame = NULL;
		m_nPreviousIndex = -1;
	}

	// Fill the update buffer with our data
	updPack.CurrentGame = m_CurrentGame;
	updPack.PreviousGame = m_PreviousGame;
	updPack.CurrentIndex = m_nCurrentIndex;
	updPack.PreviousIndex = m_nPreviousIndex;
	updPack.ListSize = nListSize;
	updPack.SortStyle = m_nSortStyle;
	updPack.SortDirection = m_nSortDirection;
	updPack.SubTitleView = GameContentManager::getInstance().getGameListSubtitleView();

	// Send UpdateGamesList to GameViewManager
	GameContentManager::getInstance().UpdateListContent(updPack);

	// Return Successfully
	bHandled = true;
	return S_OK;
}

GameListItem * CGamesList::GetCurrentGame( void )
{
	// Null pointer to store return value
	GameListItem * pGameItem = NULL;

	// If the Content List pointer is NULL, then return a NULL game
	if(m_vListContent == NULL)
		return NULL;

	// Determine the size of the currently displayed list content
	int nListSize = m_vListContent->size();
	
	// Make sure the list has elements within it, and also that the list has focus
	if(nListSize > 0 && GetCurSel() != -1)
	{
		// Extract the GameItem pointer from the currently selected location
		pGameItem = m_vListContent->at(GetCurSel());
	}

	// Return to caller
	if(pGameItem == NULL)
		DebugMsg("CGameList", "GetCurrentGame() Failed - Returning NULL");

	return pGameItem;
}

HRESULT CGamesList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
{
	if(m_vListContent == NULL)
		return S_OK;

	// Determine if the current TextPresenter is looking for the MainTitle block
	if( (FSDTEXT_MAINTITLE == pGetSourceTextData->iData) && (pGetSourceTextData->bItemData) )
	{
		// Ensure that the index being requested isnt larger than the size of the list
		if( pGetSourceTextData->iItem < (int)m_vListContent->size())
		{
			// Extract the game title from the content list
			GameListItem * pGameItem = m_vListContent->at(pGetSourceTextData->iItem);
			pGetSourceTextData->szText = pGameItem->Content.m_szTitle;
			bHandled = TRUE;
		}
	}

	// Determine if the current TextPresenter is looking for the SubTitle block
	if((FSDTEXT_SUBTITLE == pGetSourceTextData->iData) && ((pGetSourceTextData->bItemData))) 
	{
		// Ensure that the index being requested isnt larger than the size of the list
		if( pGetSourceTextData->iItem < (int)m_vListContent->size() )
		{
			subTitleText = L"";
			GameListItem * pGameItem = m_vListContent->at(pGetSourceTextData->iItem);
			
			// Switch/Case the subtitle view selection for easy implementation of future views
			switch (GameContentManager::getInstance().getGameListSubtitleView())
			{
			case GAMELIST_SUBTITLEVIEW_VIEW1:
				// Display Title ID if Developer cannot be found
				if( pGameItem->ContentRef->getDeveloper() == L"")
					if( pGameItem->ContentRef->getTitleId() == 0x00000000 )
						subTitleText = L"Homebrew Development";
					else
						subTitleText = pGameItem->Content.m_szId;
				else
					subTitleText = pGameItem->Content.m_szDeveloper;
				break;
			case GAMELIST_SUBTITLEVIEW_VIEW2:
				// Display game's device code DRIVE: TITLEID (ex. HDD1: 535107E4) and Users Rating
				subTitleText = pGameItem->Content.m_szDeviceCode;
				break;
			case GAMELIST_SUBTITLEVIEW_VIEW3:
				// Current subtitleview 3 is blank
				subTitleText = L"";
				break;
			default:
				subTitleText = L"";
			};
	
			// Paint text to main screen
			pGetSourceTextData->szText = subTitleText;
			bHandled = TRUE;
		}
	}
	
    return S_OK;
}

HRESULT CGamesList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData, BOOL& bHandled)
{
	int nListIndex = pGetSourceImageData->iItem;

	// Display Icon Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEICON && pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEICON, &nListIndex, m_vListContent);
		bHandled = true;
	}

	// Display Boxart Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEBOXART && pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEBOXART, &nListIndex, m_vListContent);
		bHandled = true;
	}

	// Display Banner Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEBANNER && pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEBANNER, &nListIndex, m_vListContent);
		bHandled = true;
	}

	// Display Favorites Star
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEFAVORITE && pGetSourceImageData->bItemData){
		if(GameContentManager::getInstance().getGameListSubtitleView() == GAMELIST_SUBTITLEVIEW_VIEW1)
			pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEFAVORITE, &nListIndex, m_vListContent);
		else
			pGetSourceImageData->hBrush = NULL;
		
		bHandled = true;
	}

	// Display User's Rating
	for(int nIndex = 0; nIndex < FSDRATING_MAXSTARCOUNT; nIndex++)
	{
		if(pGetSourceImageData->iData == (FSDIMAGE_GAMERATINGSTARBASE + nIndex) && pGetSourceImageData->bItemData){
			if(GameContentManager::getInstance().getGameListSubtitleView() == GAMELIST_SUBTITLEVIEW_VIEW2)
				pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture((FSDIMAGE_GAMERATINGSTARBASE + nIndex), &nListIndex, m_vListContent);
			else
				pGetSourceImageData->hBrush = NULL;
			
			bHandled = true;
		}
	}

	return S_OK;
}

HRESULT CGamesList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	if(m_vListContent == NULL)
		pGetItemCountData->cItems = 0;
	else
		pGetItemCountData->cItems = (int)m_vListContent->size();

	// Signifiy to the GameViewManager that the list is ready to be added to
	GameContentManager::getInstance().m_bListReady = true;
	bHandled = TRUE;
	return S_OK;
}

void CGamesList::RefreshCurrentItem() 
{
	GAMECONTENT_LIST_STATE updPack;
	GAMECONTENT_LIST_STATE listPack;
	listPack = GameContentManager::getInstance().getGameListState(false);
	
	// Track down the game 
	GameListItem * pGameContent = GetCurrentGame();
	GameListItem * pPreviousGame = GameContentManager::getInstance().m_sPreviousSortGame;

	if(pGameContent != pPreviousGame) {
		
		// Create an update buffer for changing the content
		updPack.CurrentGame = pGameContent;
		updPack.PreviousGame = pPreviousGame;
		updPack.CurrentIndex = this->GetCurSel();
		updPack.PreviousIndex = -1;
		updPack.ListSize = this->GetItemCount();
		updPack.SortStyle = listPack.SortStyle;
		updPack.SortDirection = listPack.SortDirection;
		updPack.SubTitleView = listPack.SubTitleView;

		GameContentManager::getInstance().UpdateListContent(updPack);
	}
}
