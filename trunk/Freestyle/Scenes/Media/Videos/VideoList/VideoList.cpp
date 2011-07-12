#pragma once

#include "stdafx.h"
#include "VideoList.h"

#include "../../../../Tools/Settings/Settings.h"
#include "../../../../Tools/Managers/Video/VideoManager.h"

HRESULT CVideoList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	DebugMsg("VideoList","Video List Init");

	DebugMsg("VideoList", "Number of Videos: %d", VideoManager::getInstance().getVideoNum());
	InsertItems( 0, VideoManager::getInstance().getVideoNum()); 
	SetCurSel( 0 );

	return S_OK;
}

HRESULT CVideoList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
{
	if( ( 0 == pGetSourceTextData->iData ) && ( pGetSourceTextData->bItemData ) ) 
	{
		LPCWSTR title = strtowstr(VideoManager::getInstance().getVideoName(pGetSourceTextData->iItem)).c_str();
		pGetSourceTextData->szText = title;
		bHandled = TRUE;
    }
	else if( ( 1 == pGetSourceTextData->iData ) && ( pGetSourceTextData->bItemData ) ) 
	{
		LPCWSTR type = strtowstr(VideoManager::getInstance().getVideoType(pGetSourceTextData->iItem)).c_str();
		pGetSourceTextData->szText = type;
		bHandled = TRUE;
    }
	else if( ( 2 == pGetSourceTextData->iData ) && ( pGetSourceTextData->bItemData ) ) 
	{
		int Milliseconds = (int)VideoManager::getInstance().getVideoInfo(pGetSourceTextData->iItem).dwClipDuration;
		int Hours = Milliseconds / (1000*60*60);
		int Minutes = (Milliseconds % (1000*60*60)) / (1000*60);
		int Seconds = ((Milliseconds % (1000*60*60)) % (1000*60)) / 1000;
		

		LPCWSTR length = strtowstr(sprintfaA("%.2d:%.2d:%.2d",Hours,Minutes,Seconds)).c_str();
		pGetSourceTextData->szText = length;
		bHandled = TRUE;
    }
    return S_OK;
}

HRESULT CVideoList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	pGetItemCountData->cItems = VideoManager::getInstance().getVideoNum();
	bHandled = TRUE;
	return S_OK;
}

HRESULT CVideoList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData, BOOL& bHandled)
{
	if( ( 3 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) ) {
	
		LPCWSTR IconPath = strtowstr(VideoManager::getInstance().getIconPath(pGetSourceImageData->iItem)).c_str();
		pGetSourceImageData->szPath = IconPath;
		bHandled = TRUE;
	}
	return S_OK;
}