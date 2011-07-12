#pragma once

#include "stdafx.h"
#include "AchievementList.h"
#include "../../../Tools/Debug/Debug.h"
#include "../../../Tools/Settings/Settings.h"


HRESULT CAchievementList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	DebugMsg("AchievementList","AchievementList Init");

	AchievementIconPaths.szMissingAchievementIcon = strtowstr(AchievementManager::getInstance().GetAchievementSetting("MISSINGACHIEVEMENTPATH"));

	m_sListState = GameContentManager::getInstance().getGameListSnapshot(false);

	if(m_sListState.CurrentGame == NULL)
		return S_OK;

	wstring szGameID = m_sListState.CurrentGame->ContentRef->getId();

	m_AchievementList.ListSize = AchievementManager::getInstance().GetAchievementCount(szGameID);
	AchievementManager::getInstance().CreateAchievementList(szGameID);

	InsertItems( 0, m_AchievementList.ListSize);

	DebugMsg("AchievementList","%d Items inserted into list", m_AchievementList.ListSize);

	SetCurSel(0);
	
	return S_OK;
}

HRESULT CAchievementList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	// Notify the list of its current size
	pGetItemCountData->cItems = m_AchievementList.ListSize;

	bHandled = true;

	return S_OK;
}

HRESULT CAchievementList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData, BOOL& bHandled)
{
	if( ( 0 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) ) {

		HXUIBRUSH hBrush = AchievementManager::getInstance().GetTextureBrush(pGetSourceImageData->iItem);

		if(hBrush == NULL)
			pGetSourceImageData->szPath = AchievementIconPaths.szMissingAchievementIcon.c_str();
		else
			pGetSourceImageData->hBrush = hBrush;

		bHandled = TRUE;
	}
	return S_OK;
}




