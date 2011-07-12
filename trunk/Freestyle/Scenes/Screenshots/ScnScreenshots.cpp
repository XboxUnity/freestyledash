#include "stdafx.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Debug/debug.h"
#include "../../Tools/Managers/Screenshot/ScreenshotManager.h"
#include "../../Tools/GameContent/GameContentManager.h"
#include "../../Tools/Threads/ThreadLock.h"
#include "ScnScreenshots.h"


HRESULT CScnScreenshots::OnInit( XUIMessageInit *pInitData, BOOL &bHandled )
{
	DebugMsg("ScnScreenshots", "ScnScreenshots Initialized");

	HRESULT HR = GetChildById( L"ScreenshotViewer", &m_Screenshots);
	isNested = HR == S_OK;

	if(isNested)
	{
		m_Screenshots.GetChildById( L"ScreenshotList", &m_ScreenshotList );
	}
	else
	{
		// Grab Xui Handles for all of the controls
		GetChildById( L"FullScreenPresenter", &m_FullScreen );
		GetChildById( L"ScreenshotList", &m_ScreenshotList );
	}
	GetChildById( L"Back", &m_Back );
	GetChildById( L"ShowFullScreen", &m_showFullScreen );
	// Grab Xui Handles for all of the controls
	GetChildById( L"FullScreenPresenter", &m_FullScreen );

		
	//Hide Full Screen Presenter
	m_bFullscreenMode = false;
	m_FullScreen.SetShow(false);

	// Grab Current List Snapshot
	GAMECONTENT_LIST_STATE listSnapshot;
	listSnapshot = GameContentManager::getInstance().getGameListSnapshot(false);
	if(listSnapshot.CurrentGame != NULL) {
		pGameContent = listSnapshot.CurrentGame;
	} else {
		pGameContent = NULL;
	}

	// Get the Screenshot List Information
	ScreenshotManager::getInstance().GetListInfo(&ScreenshotListInfo);

	if(ScreenshotListInfo.ListSize < 1) {
		m_showFullScreen.SetEnable(FALSE);
		m_showFullScreen.SetShow(FALSE);
	}

	CenterList();

	return S_OK;
}

HRESULT CScnScreenshots::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if(hObjPressed == m_showFullScreen) {
		
		bool test = isNested;
		m_bFullscreenMode = true;
		m_FullScreen.SetShow(true);

		bHandled = TRUE;
	} else if(hObjPressed == m_Back) {
		
		if(m_bFullscreenMode) {
			m_bFullscreenMode = false;
			m_FullScreen.SetShow(false);
			bHandled = TRUE;
		} else {
			NavigateBack(XUSER_INDEX_ANY);
			bHandled = TRUE;
		}
	}
	return S_OK;
}

HRESULT CScnScreenshots::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled)
{
	if(m_bFullscreenMode) {
		if( ( 200 == pGetSourceImageData->iData ) && ( !pGetSourceImageData->bItemData ) ) {

			// Grab the current screenshot info
			ScreenshotManager::getInstance().GetListInfo(&ScreenshotListInfo);
			if(ScreenshotListInfo.listLoaded)
			{
				//Check if Brush is Null (OnLoad)
				if(ScreenshotListInfo.CurrentBrush == NULL)	{
					ScreenshotListInfo.CurrentBrush = ScreenshotManager::getInstance().GetTextureBrush(ScreenshotListInfo.CurrentIndex);
				}

				// Paint the texture to the image presenter
				pGetSourceImageData->hBrush = ScreenshotListInfo.CurrentBrush;
			}
		}
	} 
	else
	{
		if( ( 100 == pGetSourceImageData->iData ) && ( !pGetSourceImageData->bItemData ) ) {	
			ScreenshotManager::getInstance().GetListInfo(&ScreenshotListInfo);
			if(ScreenshotListInfo.listLoaded)
			{
				//Check if Brush is Null (OnLoad)
				if(ScreenshotListInfo.CurrentBrush == NULL) {
					ScreenshotListInfo.CurrentBrush = ScreenshotManager::getInstance().GetTextureBrush(ScreenshotListInfo.CurrentIndex);
				}

				// Paint the texture to the image presenter
				pGetSourceImageData->hBrush = ScreenshotListInfo.CurrentBrush;
			}

		}
	}
	if( ( FSDIMAGE_GAMEBACKGROUND == pGetSourceImageData->iData ) && ( !pGetSourceImageData->bItemData ) ) 
	{
		//Check if Brush is Null (OnLoad)
		if(pGameContent != NULL) {
			if(pGameContent->Textures.m_BkgTexture != NULL) {
				pGetSourceImageData->hBrush = pGameContent->Textures.m_BkgTexture->GetTextureBrush();
			}
		}
	}
	bHandled = TRUE;
	return S_OK;
}

void CScnScreenshots::CenterList()
{
	if(ScreenshotListInfo.ListSize < ScreenshotListInfo.maxVisible)
	{
		float height, width, itemWidth;
		m_ScreenshotList.GetBounds(&width, &height);

		//Get list item width
		itemWidth = width / ScreenshotListInfo.maxVisible;

		//Set List Size Dynamically
		width = ((float)ScreenshotListInfo.ListSize * itemWidth);
		m_ScreenshotList.SetBounds(width, height);


		D3DXVECTOR3 newPosition, oldPosition;
		m_ScreenshotList.GetPosition(&oldPosition);

		//Find New Position
		//  - Centered in X
		//  - Y stays the same
		newPosition.x = (1280 - width) / 2;
		newPosition.y = oldPosition.y;

		m_ScreenshotList.SetPosition(&newPosition);
	}
}