#pragma once
#include "stdafx.h"

#include "XlinkKaiBuddyList.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/XlinkKai/General/KaiManager.h"
#include "../../Tools/XlinkKai/Buddy/KaiBuddyManager.h"

#define LIST_UPDATE_INTERVAL	500
#define TM_LISTUPDATE			75

CXlinkBuddyList::CXlinkBuddyList()
{
	// Initialize our variables
	bIsLoaded = false; 
	bInitializeList = false; 
	bListChanged = false;
}

CXlinkBuddyList::~CXlinkBuddyList()
{
	// Destroy our timer
	KillTimer(TM_LISTUPDATE);
}

HRESULT CXlinkBuddyList::OnInit(XUIMessageInit * pInitData, BOOL& bHandled) {

	DebugMsg("CXlinkBuddyList", "CXlinkBuddyList::OnInit Called");

	// Let's grab a copy of our current Arena Informtion
	CKaiBuddyManager::getInstance().getBuddyInfo(&m_pBuddyList);
	CKaiBuddyManager::getInstance().getBuddyLookupInfo(&m_BuddyNames);	

	// Next we'll set our local size variable
	m_nCurrentListSize = CKaiBuddyManager::getInstance().getBuddyListCount();

	// Set up our state flags
	bIsLoaded = false; bInitializeList = true; bListChanged = false;

	// Let's load our list elements now
	LoadBuddyList();

	// Start our updating timer, to have constant list updates
	SetTimer(TM_LISTUPDATE, LIST_UPDATE_INTERVAL);
	
	return S_OK;
}

HRESULT CXlinkBuddyList::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	// this is where we want to refresh our list, and we only want to refresh if a change has been triggered
	switch (pTimer->nId) {
		case TM_LISTUPDATE:
			// Update has triggered- lets do some work
			if( bListChanged == false ) break;
			// Reset our flag, so it can be triggered again by another thread
			bListChanged = false;

			LoadBuddyList();
			break;
	};

	return S_OK;
}


HRESULT CXlinkBuddyList::OnGetSourceDataText(XUIMessageGetSourceText * pGetSourceTextData, BOOL& bHandled)
{
	// If the list isn't loaded, or is in the middle of a swap, let's bail
	if( bIsLoaded == false ) {
		return S_OK;
	}

	// Handle the displaying of the Player's name
	if( ( pGetSourceTextData->iData == 0 ) && ( pGetSourceTextData->bItemData ) ) {
		if( pGetSourceTextData->iItem < (int)m_pBuddyList.size() ) {
			// Convert our string path to wstring
			pGetSourceTextData->szText = m_pBuddyList[m_BuddyNames.at( pGetSourceTextData->iItem )].GetPlayerName();
			bHandled = TRUE;
		}
	}
	// Handle the displaying of the Player's ping
	else if( ( pGetSourceTextData->iData == 1 ) && ( pGetSourceTextData->bItemData ) ) {
		if( pGetSourceTextData->iItem < (int)m_pBuddyList.size() ) {
			// Convert our string path to wstring
			pGetSourceTextData->szText =  m_pBuddyList[m_BuddyNames.at( pGetSourceTextData->iItem )].GetPlayerPingText();
			bHandled = TRUE;
		}
	}
	// Handle the displaying of the Player's status
	else if( ( pGetSourceTextData->iData == 2 ) && ( pGetSourceTextData->bItemData ) ) {
		if( pGetSourceTextData->iItem < (int)m_pBuddyList.size() ) {
			// Convert our string path to wstring
			pGetSourceTextData->szText = m_pBuddyList[m_BuddyNames.at( pGetSourceTextData->iItem )].GetPlayerStatusText();
			bHandled = TRUE;
		}
	}
	// Handle the displaying of the Player's Location
	else if( ( pGetSourceTextData->iData == 3 ) && ( pGetSourceTextData->bItemData ) ) {
		if( pGetSourceTextData->iItem < (int)m_pBuddyList.size() ) {
			if( m_pBuddyList[m_BuddyNames.at(pGetSourceTextData->iItem)].GetPlayerStatus() > -1 && m_pBuddyList[m_BuddyNames.at(pGetSourceTextData->iItem)].GetBuddyOnline()) {
				pGetSourceTextData->szText = m_pBuddyList[m_BuddyNames.at(pGetSourceTextData->iItem)].GetPlayerVector();
			} else {
				pGetSourceTextData->szText = L"Offline";
			}
			bHandled = TRUE;
		}
	}
	
	// Return Successfully
    return S_OK;
}

HRESULT CXlinkBuddyList::OnGetSourceDataImage(XUIMessageGetSourceImage * pGetSourceImageData, BOOL& bHandled)
{
	// If the list isn't loaded, or is in the middle of a swap, let's bail
	if( bIsLoaded == false ) {
		return S_OK;
	}
	/*
	if( ( 0 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) ) 
	{
		if(pGetSourceImageData->iItem < m_currentListSize) {
			string player = wstrtostr(m_pBuddyList[m_BuddyNames.at(pGetSourceImageData->iItem)].GetPlayerName());
			string dataPath = SETTINGS::getInstance().getDataPath();
			dataPath = sprintfaA("file://%s\\%s\\%s\\%s.dds",dataPath.c_str(),"XlinkKaiData", "Player", player.c_str());
			dataPath = str_replaceallA(dataPath,"\\\\","\\");

			temp = strtowstr(dataPath);

			pGetSourceImageData->szPath = temp.c_str();
		}
	}

	bHandled = TRUE;
	*/
	return S_OK;
}

HRESULT CXlinkBuddyList::OnGetItemCountAll(XUIMessageGetItemCount * pGetItemCountData, BOOL& bHandled) {
	// Update our list count with our counter variable
	pGetItemCountData->cItems = m_nCurrentListSize;
	bHandled = TRUE;
	return S_OK;
}

HRESULT CXlinkBuddyList::LoadBuddyList(void)
{
	// Our list is swapping, let's block out changes
	bIsLoaded = false;
	
	// Refresh our copy of the arena item
	CKaiBuddyManager::getInstance().getBuddyInfo(&m_pBuddyList);
	CKaiBuddyManager::getInstance().getBuddyLookupInfo(&m_BuddyNames);

	int nSize = m_nCurrentListSize = (int)m_pBuddyList.size();

	// Local Variables
	unsigned int nCurrentSel = 0;
	unsigned int nCurrentTop = 0;

	// Set our current selection 
	if( nSize > 0 && GetCurSel() == -1 )
		nCurrentSel = 0;
	else
		nCurrentSel = GetCurSel();

	// Set our top most item
	nCurrentTop = GetTopItem();

	// Now let's clear our entire list, so we can update it
	DeleteItems(0, GetItemCount());

	// Now let's add in our new list items
	if( nSize > 0 ) {
		InsertItems( 0, nSize );
		if( bInitializeList == true ) {
			nCurrentTop = 0;
			bInitializeList = false;
		} else if( (int)nCurrentTop > nSize ) {
			nCurrentTop = nSize;
		}
	}

	// Restore list position
	SetTopItem(nCurrentTop);
	SetCurSel(nCurrentSel);

	// Notify that the list has been loaded
	if(nSize > 0 ) bIsLoaded = true;

	// Return Successfully
	return S_OK;
}


HRESULT CXlinkBuddyList::OnContactPing(KAIMessageOnContactPing * pData, BOOL& bHandled)
{
	CKaiBuddyManager::getInstance().getBuddyInfo(&m_pBuddyList);

	bHandled = TRUE;
	return S_OK;
}

HRESULT CXlinkBuddyList::OnContactAdd(KAIMessageOnContactAdd * pData, BOOL& bHandled)
{
	// Set our flag to let our timer know the list has changed
	bListChanged = true;

	// Return handled
	bHandled = TRUE;
	return S_OK;
}

HRESULT CXlinkBuddyList::OnContactRemove(KAIMessageOnContactRemove * pData, BOOL& bHandled)
{
	// Set our flag to let our timer know the list has changed
	bListChanged = true;

	// Return handled
	bHandled = TRUE;
	return S_OK;
}

HRESULT CXlinkBuddyList::OnContactOnline(KAIMessageOnContactOnline * pData, BOOL& bHandled)
{
	// Set our flag to let our timer know the list has changed
	bListChanged = true;

	// Return handled
	bHandled = TRUE;
	return S_OK;
}

HRESULT CXlinkBuddyList::OnContactOffline(KAIMessageOnContactOffline * pData, BOOL& bHandled)
{
	// Set our flag to let our timer know the list has changed
	bListChanged = true;

	// Return handled
	bHandled = TRUE;
	return S_OK;
}