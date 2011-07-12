#include "stdafx.h"

#include "ChatManager.h"

ChatManager::ChatManager() {
	// First thing we want to do is to register to the kai manager, so we receive kai messages
	bChatActive = false;
	KaiManager::getInstance().RegisterClass(*this);

	// Now we'll initialize a lot of our variables
	m_vChatPlayers.clear();
	

	// Create our critical section used to protect our entire class
	InitializeCriticalSection(&m_ChatLock);
}

ChatManager::~ChatManager() {
	// Here let's clear our information
	//ClearChatPlayers();
	
	// Delete our critical section from memory
	DeleteCriticalSection(&m_ChatLock);	
}

// Retrieve a copy of the ChatPlayers vector
HRESULT ChatManager::getChatPlayerInfo(ChatPlayer *m_pChatInfo) {
	if( m_pChatInfo == NULL) return S_FALSE;

	CHAT_LOCK;  // Let's grab exclusive rights here, so we can't update while we're trying to retrieve a copy
	*m_pChatInfo = m_vChatPlayers;
	CHAT_UNLOCK;
	return S_OK;
}

// Returns the current size of the ChatPlayers vector
int ChatManager::getChatPlayerCount() {
	return (int) m_vChatPlayers.size();
}

// Returns a reference to the chat buffer
wstring& ChatManager::getChatBuffer() {
	return m_szChatBuffer;
}

void ChatManager::clearChatBuffer() {
	m_szChatBuffer = L"";
}

void ChatManager::StartChatChannel() {
	
	CHAT_LOCK;		// Lock

	// Iterate through our chat player vector and clear the list
//	vector<CKaiPlayer*>::iterator itr;
//	for( itr = m_vChatPlayers.begin(); itr != m_vChatPlayers.end(); ++itr ) {
//		if( (*itr) != NULL ) delete (*itr);
//	}
	m_vChatPlayers.clear();

	// Clear our existing Chat Buffer
	m_szChatBuffer = L"";

	CHAT_UNLOCK;		//Unlock

	// Trigger our chat monitoring flag
	bChatActive = true;
}

void ChatManager::StopChatChannel() {
	// Trigger our chat monitoring flag
	bChatActive = false;

	CHAT_LOCK;		// Lock
	
	// Iterate through our chat player vector and clear the list
//	vector<CKaiPlayer*>::iterator itr;
//	for( itr = m_vChatPlayers.begin(); itr != m_vChatPlayers.end(); ++itr ) {
//		if( (*itr) != NULL ) delete (*itr);
//	}
	m_vChatPlayers.clear();

	// Clear our existing Chat Buffer
	m_szChatBuffer = L"";

	CHAT_UNLOCK;		// Unlock
}

// Send a chat message to the kai client
void ChatManager::SendChatMessage( wstring aMessage ) {
	KaiManager::getInstance().KaiChat( wstrtostr(aMessage) );
}

// Handles the kai message for a player joining the general chat
void ChatManager::OnJoinsChat( string aOpponent )
{
	// If the chat scene is not active, lets not handle any messages
	if(bChatActive == false) return;

	CHAT_LOCK;		// Lock

	// Iterator through the vector of CKaiPlayers to search for duplicates
	vector<CKaiPlayer>::iterator itr;
	for(itr = m_vChatPlayers.begin(); itr != m_vChatPlayers.end(); ++itr)
	{
		wstring aOpponentW = strtowstr(aOpponent);
		if(wcscmp((*itr).GetPlayerName(),aOpponentW.c_str()) == 0) { 
			CHAT_UNLOCK;
			return;
		}
	}

	// Player wasn't found so let's create a new player item
	CKaiPlayer * pPlayer = new CKaiPlayer(aOpponent);
	
	// Add him to the player list
	m_vChatPlayers.push_back(*pPlayer);

	delete pPlayer;

	CHAT_UNLOCK;		// Unlock
}

// handles the kai message when a player leaves the general chat
void ChatManager::OnLeavesChat( string aOpponent ) {

	if( bChatActive == false ) return;

	CHAT_LOCK;		// Lock
	
	// Iterate through the list of players to search for the player leaving
	vector<CKaiPlayer>::iterator itr; bool bFoundPlayer = false;
	for(itr = m_vChatPlayers.begin(); itr != m_vChatPlayers.end(); ++itr) {
		wstring aOpponentW = strtowstr(aOpponent);
		if(wcscmp((*itr).GetPlayerName(),aOpponentW.c_str()) == 0)	{
			bFoundPlayer = true;
			break;
		}
	}

	// Check if we found a matching player, and if so, erase it from the list
	if(bFoundPlayer == true ) {
		m_vChatPlayers.erase(itr);
	}

	CHAT_UNLOCK;		// Unlock

}

void ChatManager::OnChat(string aVector, string aOpponent, string aMessage, bool bPrivate)
{
	// First, let's check if the chat is active- ( we don't want to build our buffer up if we're not viewing it )
	if( bChatActive == false ) return;
	
	// This class doesn't handle private messages, so let's bail if it is one.
	if( bPrivate == true) return;
	
	CHAT_LOCK;			// Lock

	m_szChatBuffer.append( strtowstr( aOpponent ) );
	m_szChatBuffer.append( L" : " );
	m_szChatBuffer.append( strtowstr( aMessage ) );
	m_szChatBuffer.append( L"\n"  );

	CHAT_UNLOCK;		// Unlock
}