#pragma once
#include "stdafx.h"

#include "XlinkKaiPlayerList.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/XlinkKai/General/KaiManager.h"

#define LIST_UPDATE_INTERVAL	500
#define TM_LISTUPDATE			75

CXlinkPlayerList::CXlinkPlayerList()
{
	// Initialize our variables
	bIsLoaded = false; 
	bInitializeList = false; 
	bListChanged = false;
}

CXlinkPlayerList::~CXlinkPlayerList()
{
	// Destroy our timer
	KillTimer(TM_LISTUPDATE);
}

HRESULT CXlinkPlayerList::OnInit(XUIMessageInit * pInitData, BOOL& bHandled) {

	DebugMsg("CXlinkPlayerList", "CXlinkPlayerList::OnInit Called");

	// Let's grab a copy of our current Arena Informtion
	ArenaManager::getInstance().getArenaInfo( &m_pArenaInfo );

	// Next we'll set our local size variable
	m_nCurrentListSize = (int)m_pArenaInfo.vArenaPlayers.size();

	// Set up our state flags
	bIsLoaded = false; bInitializeList = true; bListChanged = false;

	// Let's load our list elements now
	LoadPlayerList();

	// Start our updating timer, to have constant list updates
	SetTimer(TM_LISTUPDATE, LIST_UPDATE_INTERVAL);
	
	return S_OK;
}

HRESULT CXlinkPlayerList::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	// this is where we want to refresh our list, and we only want to refresh if a change has been triggered
	switch (pTimer->nId) {
		case TM_LISTUPDATE:
			// Update has triggered- lets do some work
			if( bListChanged == false ) break;
			// Reset our flag, so it can be triggered again by another thread
			bListChanged = false;

			LoadPlayerList();
			break;
	};

	return S_OK;
}

HRESULT CXlinkPlayerList::OnGetSourceDataText(XUIMessageGetSourceText * pGetSourceTextData, BOOL& bHandled)
{
	// If the list isn't loaded, or is in the middle of a swap, let's bail
	if( bIsLoaded == false ) {
		return S_OK;
	}

	// Handle the displaying of the Player's name
	if( ( pGetSourceTextData->iData == 0 ) && ( pGetSourceTextData->bItemData ) ) {
		if( pGetSourceTextData->iItem < (int)m_pArenaInfo.vArenaPlayers.size() ) {
			// Convert our string path to wstring
			pGetSourceTextData->szText = m_pArenaInfo.vArenaPlayers.at( pGetSourceTextData->iItem ).GetPlayerName();
			bHandled = TRUE;
		}
	}
	// Handle the displaying of the Player's ping
	else if( ( pGetSourceTextData->iData == 1 ) && ( pGetSourceTextData->bItemData ) ) {
		if( pGetSourceTextData->iItem < (int)m_pArenaInfo.vArenaPlayers.size() ) {
			// Convert our string path to wstring
			pGetSourceTextData->szText = m_pArenaInfo.vArenaPlayers.at( pGetSourceTextData->iItem ).GetPlayerPingText();
			bHandled = TRUE;
		}
	}
	// Handle the displaying of the Player's status
	else if( ( pGetSourceTextData->iData == 2 ) && ( pGetSourceTextData->bItemData ) ) {
		if( pGetSourceTextData->iItem < (int)m_pArenaInfo.vArenaPlayers.size() ) {
			// Convert our string path to wstring
			pGetSourceTextData->szText = m_pArenaInfo.vArenaPlayers.at( pGetSourceTextData->iItem ).GetPlayerStatusText();
			bHandled = TRUE;
		}
	}

	// Return Successfully
    return S_OK;
}

HRESULT CXlinkPlayerList::OnGetSourceDataImage(XUIMessageGetSourceImage * pGetSourceImageData, BOOL& bHandled)
{
	// If the list isn't loaded, or is in the middle of a swap, let's bail
	if( bIsLoaded == false ) {
		return S_OK;
	}
	/*
	if( ( 0 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) ) 
	{
		if(pGetSourceImageData->iItem < m_currentListSize) {
			string player = wstrtostr(m_pArenaInfo.vArenaPlayers.at(pGetSourceImageData->iItem)->GetPlayerName());
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

HRESULT CXlinkPlayerList::OnGetItemCountAll(XUIMessageGetItemCount * pGetItemCountData, BOOL& bHandled) {
	// Update our list count with our counter variable
	pGetItemCountData->cItems = m_nCurrentListSize;
	bHandled = TRUE;
	return S_OK;
}

// Here is where we refresh our list elements
HRESULT CXlinkPlayerList::LoadPlayerList( void ) {
	// Our list is swapping, let's block out changes
	bIsLoaded = false;
	
	// Refresh our copy of the arena item
	ArenaManager::getInstance().getArenaInfo(&m_pArenaInfo);
	int nSize = m_nCurrentListSize = m_pArenaInfo.vArenaPlayers.size();

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


HRESULT CXlinkPlayerList::OnEnterArena(KAIMessageOnEnterArena * pData, BOOL& bHandled)
{
	// Set our flag to let our timer know the list has changed
	bListChanged = true;

	// Return handled
	bHandled = TRUE;
	return S_OK;
}

HRESULT CXlinkPlayerList::OnOpponentPing(KAIMessageOnOpponentPing * pData, BOOL& bHandled)
{
	// Set our flag to let our timer know the list has changed
	bListChanged = true;

	// Return handled
	bHandled = TRUE;
	return S_OK;
}

HRESULT CXlinkPlayerList::OnOpponentEnter(KAIMessageOnOpponentEnter * pData, BOOL& bHandled)
{
	// Set our flag to let our timer know the list has changed
	bListChanged = true;

	// Return handled
	bHandled = TRUE;
	return S_OK;
}
	
HRESULT CXlinkPlayerList::OnOpponentLeave(KAIMessageOnOpponentLeave * pData, BOOL& bHandled)
{
	// Set our flag to let our timer know the list has changed
	bListChanged = true;

	// Return handled
	bHandled = TRUE;
	return S_OK;
}