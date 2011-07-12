#include "stdafx.h"

#include "../../Generic/xboxtools.h"
#include "../../Debug/Debug.h"
#include "../KaiNotifyManager.h"
#include "KaiBuddyManager.h"

CKaiBuddyManager::CKaiBuddyManager()
{
	// First thing we want to do is to register to the kai manager, so we receive kai messages
	KaiManager::getInstance().RegisterClass(*this);

	// Now we'll initialize a lot of our variables
	m_buddyInfo.clear();
	m_buddyLookup.clear();
	singleAlert = false;

	// Create our critical section used to protect our entire class
	InitializeCriticalSection(&m_BuddyLock);
}

CKaiBuddyManager::~CKaiBuddyManager()
{
	m_buddyInfo.clear();
	m_buddyLookup.clear();

	// Delete our critical section
	DeleteCriticalSection(&m_BuddyLock);
}

HRESULT CKaiBuddyManager::getBuddyInfo( std::map<string, CKaiPlayer> * pBuddyInfo ) {
	if( pBuddyInfo == NULL ) return S_FALSE;

	BUDDY_LOCK;		// Let's grab exclusive rights here, so we can't update while we're trying to retrieve a copy
	*pBuddyInfo = m_buddyInfo;
	BUDDY_UNLOCK;

	return S_OK;
}

HRESULT CKaiBuddyManager::getBuddyLookupInfo( std::vector<string> * pBuddyLookupInfo ) {
	if( pBuddyLookupInfo == NULL ) return S_FALSE;

	BUDDY_LOCK;		// Let's grab exclusive rights here, so we can't update while we're trying to retrieve a copy
	*pBuddyLookupInfo = m_buddyLookup;
	BUDDY_UNLOCK;

	return S_OK;
}

// Retrieves the size of the buddy info
int CKaiBuddyManager::getBuddyListCount( void ) {
	return m_buddyInfo.size();
}

// Retrieves whether or not a certain opponent is a buddy
bool CKaiBuddyManager::isBuddy( string aOpponent ) {
	if( m_buddyInfo.find(aOpponent) != m_buddyInfo.end() )
		return false;
	else
		return true;
}


void CKaiBuddyManager::OnContactAdd(string aFriend) {

	BUDDY_LOCK;		// Lock

	CKaiPlayer * buddy = new CKaiPlayer(aFriend);
	m_buddyInfo.insert(BuddyItem::value_type(aFriend, *buddy));
	delete buddy;
	
	//Add player name to lookup vector
	m_buddyLookup.push_back(aFriend);

	BUDDY_UNLOCK;		// Unlock
}

void CKaiBuddyManager::OnContactRemove(string aFriend) {

	BUDDY_LOCK;		// Lock

	// Remove a friend from our vector
	if(m_buddyInfo.find(aFriend) != m_buddyInfo.end())
		m_buddyInfo.erase(aFriend);
	
	// Iterate through our look up table and remove that entry as well
	vector<string>::iterator itr;
	for(itr = m_buddyLookup.begin(); itr != m_buddyLookup.end(); ++itr) {
		if(strcmp((*itr).c_str(), aFriend.c_str()) == 0) {
			m_buddyLookup.erase(itr);
			break;
		}
	}

	BUDDY_UNLOCK;		// Unlock
}

void CKaiBuddyManager::OnContactOffline(string aFriend) {
	// The buddy was not found in the list, so we need to bail
	if(m_buddyInfo.find(aFriend) == m_buddyInfo.end() ) return;

	BUDDY_LOCK;		// Lock
		
	// Set our online status of the buddy
	m_buddyInfo[aFriend].SetBuddyOnline(false);

	BUDDY_UNLOCK;		// Unlock

	// Send an notice request to the notice manager
	CKaiNotifyManager::getInstance().NotifyContactOffline(aFriend);

	// Set singleAlert Flag
	singleAlert = false;
}

void CKaiBuddyManager::OnContactOnline(string aFriend) {

	// The buddy was not found in the list, so we need to bail
	if(m_buddyInfo.find(aFriend) == m_buddyInfo.end() ) return;

	BUDDY_LOCK;		// Lock
		
	// Set our online status of the buddy
	m_buddyInfo[aFriend].SetBuddyOnline(true);

	// Send the message to update our avatar
	KaiManager::getInstance().KaiQueryAvatar(aFriend);

	BUDDY_UNLOCK;		// Unlock

	// Process notification
	if(!singleAlert)
	{
		CKaiNotifyManager::getInstance().NotifyContactOnline(aFriend);
		singleAlert = true;
	}
}

void CKaiBuddyManager::OnContactPing(string aFriend, string aVector, DWORD aPing, int aStatus, string aBearerCapability)
{
	// The buddy was not found in the list, so we need to bail
	if(m_buddyInfo.find(aFriend) == m_buddyInfo.end() ) return;

	BUDDY_LOCK;			// Lock

	// Set up our buddy information
	m_buddyInfo[aFriend].SetPlayerVector(aVector);
	m_buddyInfo[aFriend].SetPlayerPing(aPing);
	m_buddyInfo[aFriend].SetPlayerStatus(aStatus);
	m_buddyInfo[aFriend].SetPlayerCapabilities(aBearerCapability);

	BUDDY_UNLOCK;		// Unlock
}

void CKaiBuddyManager::OnUpdateOpponent(string aOpponent, string aAge, string aBandwidth, string aLocation, string aBio) {
	// The buddy was not found in the list, so we need to bail
	if(m_buddyInfo.find(aOpponent) == m_buddyInfo.end() ) return;

	BUDDY_LOCK;			// Lock

	// Set up our buddy information
	m_buddyInfo[aOpponent].SetPlayerAge(aAge);
	m_buddyInfo[aOpponent].SetPlayerBandwidth(aBandwidth);
	m_buddyInfo[aOpponent].SetPlayerLocation(aLocation);
	m_buddyInfo[aOpponent].SetPlayerBio(aBio);

	BUDDY_UNLOCK;		// Unlock
}

void CKaiBuddyManager::OnUpdateOpponent(string aOpponent, string aAvatarURL) {
	// The buddy was not found in the list, so we need to bail
	if(m_buddyInfo.find(aOpponent) == m_buddyInfo.end() ) return;

	BUDDY_LOCK;			// Lock

	// Handle Players Avatar Here
	DebugMsg("CKaiBuddyManager", "%s - Avatar URL Received - %s", aOpponent.c_str(), aAvatarURL.c_str());

	BUDDY_UNLOCK;		// Unlock
}

void CKaiBuddyManager::OnOpponentEnter(string aOpponent) {
	// The buddy was not found in the list, so we need to bail
	if(m_buddyInfo.find(aOpponent) == m_buddyInfo.end() ) return;

	BUDDY_LOCK;			// Lock

	// Handle Buddy entering the same vector you are in
	DebugMsg("CKaiBuddyManager", "Buddy:  %s - Has just entered the same vector", aOpponent.c_str());

	BUDDY_UNLOCK;
}

void CKaiBuddyManager::OnOpponentLeave(string aOpponent) {
	// The buddy was not found in the list, so we need to bail
	if(m_buddyInfo.find(aOpponent) == m_buddyInfo.end() ) return;

	BUDDY_LOCK;			// Lock
	
	// Handle buddy leaving the vector you are in
	DebugMsg("CKaiBuddyManager", "Buddy:  %s - Has just left the vector you were in.", aOpponent.c_str());

	BUDDY_UNLOCK;		// Unlock
}

void CKaiBuddyManager::OnJoinsChat(string aOpponent) {
	// The buddy was not found in the list, so we need to bail
	if(m_buddyInfo.find(aOpponent) == m_buddyInfo.end() ) return;

	BUDDY_LOCK;			// Lock
	
	// Handle buddy joining the same chatroom you are in
	DebugMsg("CKaiBuddyManager", "Buddy:  %s - Has joined the same chat room you are in.", aOpponent.c_str());

	BUDDY_UNLOCK;		// Unlock
}

void CKaiBuddyManager::OnChatPrivateMessage(string aPlayer, string aMessage, string aVector, bool bContactList) {
	// The buddy was not found in the list, so we need to bail
	if(m_buddyInfo.find(aPlayer) == m_buddyInfo.end() ) return;

	BUDDY_LOCK;			// Lock

	// Handle buddy joining the same chatroom you are in
	DebugMsg("CKaiBuddyManager", "Buddy:  %s - Just sent a private message", aPlayer.c_str());

	BUDDY_UNLOCK;

	// Message received - later determine if the message is from arena or contact list (doesnt seem to matter atm)
	CKaiNotifyManager::getInstance().NotifyReceivedContactPM(aPlayer, aMessage);

}

void CKaiBuddyManager::OnLeavesChat(string aOpponent)
{
	// The buddy was not found in the list, so we need to bail
	if(m_buddyInfo.find(aOpponent) == m_buddyInfo.end() ) return;

	BUDDY_LOCK;			// Lock

	// Handle buddy joining the same chatroom you are in
	DebugMsg("CKaiBuddyManager", "Buddy:  %s - Hes just left the chat room you are in.", aOpponent.c_str());

	BUDDY_UNLOCK;		// Unlock
}
