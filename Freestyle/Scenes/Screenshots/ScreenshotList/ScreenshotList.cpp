#include "stdafx.h"
#include "../../../Tools/GameContent/GameContentManager.h"
#include "../../../Tools/Managers/Screenshot/ScreenshotManager.h"
#include "../../../Tools/Managers/GameList/GameListItem.h"
#include "ScreenshotList.h"

HRESULT CScreenshotList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	DebugMsg("ScreenshotList", "List Initialized");
	flag = false;

	HRESULT hr = NULL;

	// Grab the currently selected item from the GameViewManager
	m_sListState = GameContentManager::getInstance().getGameListSnapshot(false);

	if(m_sListState.CurrentGame != NULL)
	{
		m_pScreenshotInfo.ListSize = ScreenshotManager::getInstance().GetScreenshotCount(m_sListState.CurrentGame);
		ScreenshotManager::getInstance().CreateScreenshotList(m_sListState.CurrentGame);

		hr = InsertItems( 0, m_pScreenshotInfo.ListSize); 
	}

	if(hr == S_OK)
		DebugMsg("ScreenshotList","%d Items inserted into list", m_pScreenshotInfo.ListSize);

	//Set Selection to center of odd numbered lists
	int selection = 0;
	selection = (GetMaxVisibleItemCount() / 2);

	SetCurSel( 0 );

	m_pScreenshotInfo.CurrentIndex = GetCurSel();
	m_pScreenshotInfo.maxVisible = GetMaxVisibleItemCount();
	m_pScreenshotInfo.isRendered = false;
	//m_pScreenshotInfo.CurrentBrush = NULL;

	ScreenshotManager::getInstance().SetListInfo(&m_pScreenshotInfo);

	return S_OK;
}

HRESULT CScreenshotList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	// Notify the list of its current size
	pGetItemCountData->cItems = m_pScreenshotInfo.ListSize;
	// Notify threads that the list has been initialized
	ListVariables.m_bListInitialized = true;
	bHandled = true;

	return S_OK;
}

HRESULT CScreenshotList::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	m_pScreenshotInfo.CurrentIndex = pNotifySelChangedData->iItem;
	m_pScreenshotInfo.CurrentBrush = ScreenshotManager::getInstance().GetTextureBrush(pNotifySelChangedData->iItem);

	ScreenshotManager::getInstance().SetListInfo(&m_pScreenshotInfo);

	bHandled = true;
	return S_OK;
}

HRESULT CScreenshotList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled)
{
	bHandled = true;
	return S_OK;
}

HRESULT CScreenshotList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled)
{
	if( ( 10 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) ) 
	{
		if(m_pScreenshotInfo.ListSize > 0){
			HXUIBRUSH hBrush = ScreenshotManager::getInstance().GetTextureBrush(pGetSourceImageData->iItem);
			// Paint the texture to the image presenter
			pGetSourceImageData->hBrush = hBrush;
		}
	}

	bHandled = true;

	return S_OK;
}

HRESULT CScreenshotList::OnSetCurSel(XUIMessageSetCurSel *pSetCurSelData, BOOL &bHandled )
{

	int nFocalPoint = 2;

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