#pragma once

#include "stdafx.h"
#include "ActiveList.h"
#include "../../../Tools/Debug/Debug.h"
#include "../../../Tools/GameContent/GameContentManager.h"
#include "../../../Tools/SQLite/FSDSql.h"
#include "../../../stdafx.h"
#include "../../../Tools/Managers/Skin/SkinXMLReader/SkinXMLReader.h"

vector<SQLTitleUpdate*> CActiveList::ActiveTU;

HRESULT CActiveList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	DebugMsg("ActiveList","Active List Init");
	preparelist();
	SkinXMLReader SkinXML;
	SkinXML.LoadSettings("ActiveTUList", "ActiveTUList");
	ListIconPaths.szActiveTUIcon = strtowstr(SkinXML.GetSetting("ACTIVETUPATH", ""));
	ListIconPaths.szDisabledTUIcon = strtowstr(SkinXML.GetSetting("DISABLEDTUPATH", ""));

	return S_OK;
}

HRESULT CActiveList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
{
	if( ( 0 == pGetSourceTextData->iData ) && ( pGetSourceTextData->bItemData ) ) 
	{
		temp = ActiveTU.at(pGetSourceTextData->iItem)->updateName;
		LPCWSTR title = temp.c_str();
		pGetSourceTextData->szText = title;
	}
	if( ( 1 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) 
	{
		temp = strtowstr(ActiveTU.at(pGetSourceTextData->iItem)->filename);
		LPCWSTR title = temp.c_str();
		pGetSourceTextData->szText = title;
	}
	bHandled = TRUE;
    return S_OK;
}

HRESULT CActiveList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData, BOOL& bHandled)
{
	//DebugMsg("OnGetSourceDataImage - %d",pGetSourceImageData->iData);
	// icon
	if( ( 3 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) ) {
		if (ActiveTU.at(pGetSourceImageData->iItem)->isDisabled)
		{
			pGetSourceImageData->szPath = ListIconPaths.szDisabledTUIcon.c_str();
		} else {
			pGetSourceImageData->szPath = ListIconPaths.szActiveTUIcon.c_str();
		}
	}
	bHandled = TRUE;
	return S_OK;
}

HRESULT CActiveList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	pGetItemCountData->cItems = ActiveTU.size();
	bHandled = TRUE;
	return S_OK;
}

HRESULT CActiveList::OnListRefresh(BOOL &bHandled)
{
	preparelist();
	InsertItems(0, ActiveTU.size());
	
	bHandled = TRUE;
	return S_OK;

}

DWORD CActiveList::GetCurrentTitleID()
{
	GAMECONTENT_LIST_STATE listPack;
	listPack = GameContentManager::getInstance().getGameListSnapshot();
	DWORD retVal = listPack.CurrentGame->ContentRef->getTitleId();
	return retVal;
}
void CActiveList::Refresh()
{
	preparelist();
	InsertItems(0, ActiveTU.size());
}

void CActiveList::preparelist()
{
	DWORD szGameID = GetCurrentTitleID();
	DebugMsg("ScnManageTus", "List for TitleID [%08X]", szGameID);
	vector<SQLTitleUpdate*> vec = FSDSql::getInstance().getTitleUpdatesForTitleId(0, szGameID);
	vector<SQLTitleUpdate*> vec1;
	for (unsigned int x = 0; x < vec.size(); x++)
	{
		if (!vec.at(x)->isBackup){
			bool Found = false;
			for( unsigned int y = 0; y < vec1.size(); y++)
			{
				if (vec.at(x)->updateName == vec1.at(y)->updateName)
					Found = true;
			}
			if (!Found)
				vec1.push_back(vec.at(x));
		}
	}
	ActiveTU = vec1;

}