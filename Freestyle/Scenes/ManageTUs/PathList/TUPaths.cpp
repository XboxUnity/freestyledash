#pragma once

#include "stdafx.h"
#include "TUPaths.h"
#include "../../../Tools/Debug/Debug.h"
#include "../../../Tools/GameContent/GameContentManager.h"
#include "../../../Tools/SQLite/FSDSql.h"
#include "../../../stdafx.h"
#include "../../../Tools/Managers/Drives/DrivesManager.h"

vector<TitleUpdatePath*> CTUPathList::TUPaths;

HRESULT CTUPathList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	DebugMsg("TUPathList","TUPath List Init");
	
	return S_OK;
}

HRESULT CTUPathList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
{

	if( ( 0 == pGetSourceTextData->iData ) && ( pGetSourceTextData->bItemData ) ) 
	{
		temp = strtowstr(TUPaths.at(pGetSourceTextData->iItem)->root);
		LPCWSTR title = temp.c_str();
		pGetSourceTextData->szText = title;
		bHandled = TRUE;
	}
    return S_OK;
}


HRESULT CTUPathList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	pGetItemCountData->cItems = TUPaths.size();
	bHandled = TRUE;
	return S_OK;
}
