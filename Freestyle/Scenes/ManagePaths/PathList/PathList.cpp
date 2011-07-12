#pragma once

#include "stdafx.h"
#include "PathList.h"
#include "../../../Tools/Debug/Debug.h"
#include "../../../Tools/Settings/Settings.h"
#include "../../../Tools/SQLite/FSDSql.h"

vector<ScanPath> CPathList::ScanPaths;

HRESULT CPathList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	LoadSettings("PathList", *this);

	DebugMsg("PathList","Active List Init");
	ScanPaths = FSDSql::getInstance().getScanPaths();
	tabID = FSDSql::getInstance().getTabs();

	m_currentListSize = ScanPaths.size();

	ListIconPaths.szStandardFolderIcon = strtowstr(GetSetting("STANDARDFOLDERPATH", ""));
	
	return S_OK;
}

HRESULT CPathList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
{
	if( ( 0 == pGetSourceTextData->iData ) && ( pGetSourceTextData->bItemData ) ) 
	{
		temp = strtowstr(ScanPaths.at(pGetSourceTextData->iItem).Path);
		pGetSourceTextData->szText = temp.c_str();
		bHandled = TRUE;
    }
	if( ( 1 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) 
	{
		ScanPath TempPath = ScanPaths.at(pGetSourceTextData->iItem);
		string TempTitle = "Retail: None";
		for (int x = 0; x < (int)tabID.size(); x++)
		{
			if (tabID.at(x)->tabId == TempPath.RetailTabId)
			{
				TempTitle = sprintfaA("Retail: %s", tabID.at(x)->tabName.c_str());
			}
		}
		temp = strtowstr(TempTitle);
		pGetSourceTextData->szText = temp.c_str();
		bHandled = TRUE;
	}
	if( ( 2 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) 
	{
		ScanPath TempPath = ScanPaths.at(pGetSourceTextData->iItem);
		string TempTitle = "Devkit: None";
		for (int x = 0; x < (int)tabID.size(); x++)
		{
			if (tabID.at(x)->tabId == TempPath.DevkitTabId)
			{
				TempTitle = sprintfaA("Devkit: %s", tabID.at(x)->tabName.c_str());
			}
		}
		temp = strtowstr(TempTitle);
		pGetSourceTextData->szText = temp.c_str();
		bHandled = TRUE;
	}
	if( ( 3 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) 
	{
		ScanPath TempPath = ScanPaths.at(pGetSourceTextData->iItem);
		string TempTitle = "Scan Depth: N/A";
		string depth = "Infinite";
		if(TempPath.ScanDepth > 0) 
		depth = sprintfaA("%d", TempPath.ScanDepth);
		TempTitle = sprintfaA("Scan Depth: %s", depth.c_str());
		
		temp = strtowstr(TempTitle);
		pGetSourceTextData->szText = temp.c_str();
		bHandled = TRUE;
	}
    return S_OK;
}

HRESULT CPathList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData, BOOL& bHandled)
{
	//DebugMsg("OnGetSourceDataImage - %d",pGetSourceImageData->iData);
	// icon
	if( ( 3 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) ) {
		pGetSourceImageData->szPath = ListIconPaths.szStandardFolderIcon.c_str();
		bHandled = TRUE;
	}
	return S_OK;
}

HRESULT CPathList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	pGetItemCountData->cItems = m_currentListSize;
	bHandled = TRUE;
	return S_OK;
}

HRESULT CPathList::OnListRefresh(BOOL &bHandled)
{
	DeleteItems(0, m_currentListSize);
	ScanPaths.clear();
	ScanPaths = FSDSql::getInstance().getScanPaths();
	DebugMsg("ScanPaths", "%d number of items", ScanPaths.size());
	m_currentListSize = ScanPaths.size();

	InsertItems(0, m_currentListSize);

	bHandled = TRUE;
	return S_OK;
}




