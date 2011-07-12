#pragma once
#include "stdafx.h"

#include "XlinkKaiArenaList.h"
#include "../../Tools/XlinkKai/General/KaiManager.h"

#define LIST_UPDATE_INTERVAL	10
#define TM_LISTUPDATE			70

CXlinkArenaList::CXlinkArenaList()
{
	// Initialize our variables
	bIsLoaded = false; 
	bInitializeList = false; 
	bListChanged = false;
}

CXlinkArenaList::~CXlinkArenaList()
{
	// Destroy our timer
	KillTimer(TM_LISTUPDATE);
}

HRESULT CXlinkArenaList::OnInit(XUIMessageInit * pInitData, BOOL& bHandled)
{
	DebugMsg("CXlinkArenaList", "CXlinkArenaList::OnInit Called");

	// Let's grab a copy of our current Arena Informtion
	ArenaManager::getInstance().getArenaInfo( &m_pArenaInfo );

	// Next we'll set our local size variable
	m_nCurrentListSize = (int)m_pArenaInfo.vSubArenas.size();

	// Set up our state flags
	bIsLoaded = false; bInitializeList = true; bListChanged = false;

	// Let's load our list elements now
	LoadArenaList();

	// Start our updating timer, to have constant list updates
	SetTimer(TM_LISTUPDATE, LIST_UPDATE_INTERVAL);
	
	return S_OK;
}

HRESULT CXlinkArenaList::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	// this is where we want to refresh our list, and we only want to refresh if a change has been triggered
	switch (pTimer->nId) {
		case TM_LISTUPDATE:
			// Update has triggered- lets do some work
			if( bListChanged == false ) break;
			// Reset our flag, so it can be triggered again by another thread
			bListChanged = false;

			LoadArenaList();
			break;
	};

	return S_OK;
}

HRESULT CXlinkArenaList::OnGetSourceDataText(XUIMessageGetSourceText * pGetSourceTextData, BOOL& bHandled)
{
	// If the list isn't loaded, or is in the middle of a swap, let's bail
	if( bIsLoaded == false ) {
		return S_OK;
	}

	// Handle the displaying of the Arena Name
	if( ( pGetSourceTextData->iData == 0 ) && ( pGetSourceTextData->bItemData ) ) {
		if( pGetSourceTextData->iItem < (int)m_nCurrentListSize ) {
			// Convert our string path to wstring
			SUB_ARENAITEM * subItem = &m_pArenaInfo.vSubArenas.at( pGetSourceTextData->iItem );
			pGetSourceTextData->szText = subItem->szArenaNameW.c_str();
			bHandled = TRUE;
		}
	}
	// Handle the displaying of the Arena Description
	else if( ( pGetSourceTextData->iData == 1 ) && ( pGetSourceTextData->bItemData ) ) {
		if( pGetSourceTextData->iItem < (int)m_nCurrentListSize ) {
			// Convert our string path to wstring
			SUB_ARENAITEM * subItem = &m_pArenaInfo.vSubArenas.at( pGetSourceTextData->iItem );
			pGetSourceTextData->szText = subItem->szDescriptionW.c_str();
			bHandled = TRUE;
		}
	}	
	// Handle the displaying of the Player count
	else if( ( pGetSourceTextData->iData == 2 ) && ( pGetSourceTextData->bItemData ) ) {
		if( pGetSourceTextData->iItem < (int)m_nCurrentListSize ) {
			// Convert our string path to wstring
			SUB_ARENAITEM * subItem = &m_pArenaInfo.vSubArenas.at( pGetSourceTextData->iItem );
			pGetSourceTextData->szText = subItem->szDisplayPlayers.c_str();
			bHandled = TRUE;
		}
	}
	// Handle the displaying of the arena statistics
	else if( ( pGetSourceTextData->iData == 3 ) && ( pGetSourceTextData->bItemData ) ) {
		if( pGetSourceTextData->iItem < (int)m_nCurrentListSize ) {
			// Convert our string path to wstring
			SUB_ARENAITEM * subItem = &m_pArenaInfo.vSubArenas.at( pGetSourceTextData->iItem );
			if( subItem->bIsPersonal == true ) {
				pGetSourceTextData->szText = subItem->szDisplayPlayerLimit.c_str();
			} else {
				pGetSourceTextData->szText = subItem->szDisplayPrivateArena.c_str();
			}
			bHandled = TRUE;
		}
	}

	// Return successfully
    return S_OK;
}

HRESULT CXlinkArenaList::OnGetSourceDataImage(XUIMessageGetSourceImage * pGetSourceImageData, BOOL& bHandled)
{
	// If the list isn't loaded, or is in the middle of a swap, let's bail
	if( bIsLoaded == false ) {
		return S_OK;
	}

	// Handle the loading of the appropriate arena avatar image
	if( ( pGetSourceImageData->iData == 0 ) && ( pGetSourceImageData->bItemData ) ) {
		if( pGetSourceImageData->iItem < (int)m_nCurrentListSize ) {
			// Convert our string path to wstring
			SUB_ARENAITEM * subItem = &m_pArenaInfo.vSubArenas.at( pGetSourceImageData->iItem );
			if( subItem->bIsLoaded == true ) {
				pGetSourceImageData->szPath = subItem->szArenaAvatarPathW.c_str();
				bHandled = TRUE;
			}
		}
	}
	
	// Return Successfully
	return S_OK;
}

// Here is where we refresh our list elements
HRESULT CXlinkArenaList::LoadArenaList( void )
{
	// Our list is swapping, let's block out changes
	bIsLoaded = false;
	
	// Refresh our copy of the arena item
	ArenaManager::getInstance().getArenaInfo(&m_pArenaInfo);
	int nSize = m_nCurrentListSize = m_pArenaInfo.vSubArenas.size();

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

HRESULT CXlinkArenaList::OnGetItemCountAll(XUIMessageGetItemCount * pGetItemCountData, BOOL& bHandled) {
	// Update our list count with our counter variable
	pGetItemCountData->cItems = m_nCurrentListSize;
	bHandled = TRUE;
	return S_OK;
}

//Joined New Arena
HRESULT CXlinkArenaList::OnEnterArena(KAIMessageOnEnterArena * pData, BOOL& bHandled)
{
	// Set our flag to let our timer know the list has changed
	bListChanged = true;

	// Return handled
	bHandled = TRUE;
	return S_OK;
}
	
HRESULT CXlinkArenaList::OnNewArena(KAIMessageOnNewArena * pData, BOOL& bHandled)
{
	bListChanged = true;

	// Return Handled
	bHandled = TRUE;
	return S_OK;
}