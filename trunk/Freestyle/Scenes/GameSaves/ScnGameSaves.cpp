#include <map>
#include "stdafx.h"

#include "ScnGameSaves.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Generic/tools.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Managers/VariablesCache/VariablesCache.h"
#include "../../Tools/GameContent/GameContentManager.h"

CScnGameSaves::~CScnGameSaves()
{

}

HRESULT CScnGameSaves::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{	
	listPack = GameContentManager::getInstance().getGameListSnapshot(false);
	if(listPack.CurrentGame == NULL)
		return S_OK;

	curTitleID = wstrtostr(listPack.CurrentGame->ContentRef->getId());

	HRESULT hr = GetChildById(L"SavedGamesList", &m_List);
	if(hr == S_OK)
	{
		DebugMsg("GameList", "Saved Game List Found");
	}

    return S_OK;
}

HRESULT CScnGameSaves::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled)
{	
	GameListContentPtr vListContent = GameContentManager::getInstance().getCurrentListContent();
	
	// Display Banner Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEBANNER && !pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEBANNER, NULL,  vListContent );
		bHandled = true;
	}

	// Display Background Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEBACKGROUND && !pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEBACKGROUND, NULL,  vListContent );
		bHandled = true;
	}

	return S_OK;
}

HRESULT CScnGameSaves::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{

	return S_OK;   
} 

