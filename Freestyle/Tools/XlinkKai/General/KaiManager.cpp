#pragma once
#include "stdafx.h"
#include "../../Generic/xboxtools.h"
#include "../../../Application/FreestyleApp.h"
#include "../Arena/ArenaManager.h"
#include "../Chat/ChatManager.h"
#include "../Buddy/KaiBuddyManager.h"

#include "../../SQLite/FSDSql.h"
#include "../KaiUser.h"

#include "KaiManager.h"

#define KAI_UNDEFINED			0xFF
#define KAI_LINKED				0xA1
#define KAI_UNLINKED			0xA2

KaiManager::KaiManager()
{
	// Obtain the root object for XUI Broadcasting
	m_hBaseObj = CFreestyleUIApp::getInstance().GetRootObj();
}

void KaiManager::RegisterClass(iBuddyObserver &ref)
{
	_ClassObservers.insert(ClassItem::value_type(&ref, &ref));
}

void KaiManager::UnregisterClass(iBuddyObserver &ref)
{
	_ClassObservers.erase(&ref);
}

HRESULT KaiManager::QueryProfileXuid(XUID xuid, CKaiUser ** kaiUser)
{	
/*	string strXuid = sprintfaA("0x%16.I64X", xuid);

	// Check to ensure the profile exists in the database
	ULONGLONG kaiUserId = FSDSql::getInstance().getKaiUserIdByXuid(strXuid);
	if(kaiUserId == 0){
		*kaiUser = (CKaiUser*)NULL;	
		return KAI_UNLINKED;
	}

	int nUserIndex;
	KaiUser * kai;
	
	vector<KaiUser*> vUsers = FSDSql::getInstance().getKaiUsers();
	for(nUserIndex = 0; nUserIndex < (int)vUsers.size(); nUserIndex++){
		if(kaiUserId == vUsers.at(nUserIndex)->id) {
			kai = vUsers.at(nUserIndex);
			break;
		}
	}
	// Grab information on the kaiUsername linked to the ProfileXuid
	string szUsername = kai->name;
	string szPassword = kai->password;
	string szLinkedXuid = kai->xuid;
	ULONGLONG ulUserId = kai->id;

	CKaiUser * newKaiUser = new CKaiUser(szUsername, szPassword, szLinkedXuid, ulUserId);
	*kaiUser = newKaiUser;
*/	
	return KAI_LINKED;
}

HRESULT KaiManager::GetDefaultKaiUser( CKaiUser ** kaiUser )
{
	// Grab default credentials from the the kaiclient
	string szUsername = "";
	string szPassword = "";
	string szLinkedXuid = "";
	ULONGLONG ulUserId = 0;

	CKaiUser * newKaiUser = new CKaiUser(szUsername, szPassword, szLinkedXuid, ulUserId);
	*kaiUser = newKaiUser;

	return S_OK;
}

HRESULT KaiManager::ShutdownKai( )
{
	CKaiClient::getInstance().KaiShutdown();
	return S_OK;
}

HRESULT KaiManager::LoginToKai(string szUsername, string szPassword)
{
	CKaiClient::getInstance().KaiLogin(szUsername.c_str(), szPassword.c_str());
	return S_OK;
}

HRESULT KaiManager::XlinkSignIn( CKaiUser * kaiUser )
{
	// Set KaiClient variables for logging into the network
	
	// Attach to server on network

	// Notify XUI that a user has signed in


	return S_OK;
}

HRESULT KaiManager::XlinkSignOut( void )
{
	// Clear KaiClient variables
	// Detach server

	// Notify XUI that a user has signed out

	return S_OK;
}

HRESULT KaiManager::KaiAttachToClient()
{
	//Start SubManagers
	ArenaManager::getInstance();
	ChatManager::getInstance();
	CKaiBuddyManager::getInstance();

	// Attach this manager to the KaiClient to receive the messages
	CKaiClient::getInstance().SetObserver(this, true);

	return S_OK;
}

HRESULT KaiManager::KaiLeaveArena(bool bReturnToRoot)
{
	if(bReturnToRoot) {
		CKaiClient::getInstance().KaiArenaMode();
	}else {
		CKaiClient::getInstance().KaiLeaveArena();
	}

	return S_OK;
}

void KaiManager::OnDeInitialise( void )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnDeInitialise();
	}

	// Broadcast this message to XUI
	KaiNotifyDeinitialise(m_hBaseObj, "", true);
}

void KaiManager::OnEngineDetached( void )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnEngineDetached();
	}

	// Broadcast this message to XUI
	KaiNotifyEngineDetached(m_hBaseObj, "", true);
}

void KaiManager::OnEngineAttached( void )
{
	DebugMsg("KaiManager", "Received On Engine Attached");
	// Send Notice to Classes first 
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnEngineAttached();
	}

	// Broadcast this message to XUI
	KaiNotifyEngineAttached(m_hBaseObj, "", true);
}

void KaiManager::OnEngineInUse( void )
{
	// Send Notice to Classes first 
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnEngineInUse();
	}

	// Broadcast this message to XUI
	KaiNotifyEngineInUse(m_hBaseObj, "", true);
}

void KaiManager::OnAccountNotLoggedIn(string aUsername, string aPassword)
{
	// Send Notice to Classes first 
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnAccountNotLoggedIn(aUsername, aPassword);
	}

	// Broadcast this message to XUI
	KaiNotifyAccountNotLoggedIn(m_hBaseObj, aUsername, aPassword, true);
}

void KaiManager::OnAccountLoggedIn( void )
{
	// Send Notice to Classes first 
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnAccountLoggedIn();
	}

	// Broadcast this message to XUI
	KaiNotifyAccountLoggedIn(m_hBaseObj, "", true);
}

void KaiManager::OnAuthenticationFailed( string aUsername )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnAuthenticationFailed( aUsername );
	}

	// Broadcast this message to XUI
	KaiNotifyAuthenticationFailed(m_hBaseObj, aUsername, true);
}

void KaiManager::OnNetworkError( string aError )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnNetworkError( aError );
	}

	// Broadcast this message to XUI
	KaiNotifyNetworkError(m_hBaseObj, aError, true);
}

void KaiManager::OnNetworkReachable( string aServerName )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnNetworkReachable( aServerName );
	}

	// Broadcast this message to XUI
	KaiNotifyNetworkReachable(m_hBaseObj, aServerName, true);
}

void KaiManager::OnContactOffline( string aFriend )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnContactOffline( aFriend );
	}

	// Broadcast this message to XUI
	KaiNotifyContactOffline(m_hBaseObj, aFriend, true);
}

void KaiManager::OnContactOnline( string aFriend )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnContactOnline( aFriend );
	}

	// Broadcast this message to XUI
	KaiNotifyContactOnline(m_hBaseObj, aFriend, true);
}

void KaiManager::OnContactsOnline( int nCount )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnContactsOnline( nCount );
	}

	// Broadcast this message to XUI
	KaiNotifyContactsOnline(m_hBaseObj, nCount, true);
}

void KaiManager::OnContactPing( string aFriend, string aVector, DWORD aPing, int aStatus, string aBearerCapability )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnContactPing( aFriend, aVector, aPing, aStatus, aBearerCapability );
	}

	// Broadcast this message to XUI
	KaiNotifyContactPing(m_hBaseObj, aFriend, aVector, aPing, aStatus, aBearerCapability, true);
}

void KaiManager::OnContactAdd( string aFriend )
{	
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnContactAdd( aFriend );
	}
	
	// Broadcast this message to XUI
	KaiNotifyContactAdd(m_hBaseObj, aFriend, true);
}

void KaiManager::OnContactRemove( string aFriend )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it) {
		if( it->first != NULL )
			it->first->OnContactRemove( aFriend );
	}

	// Broadcast this message to XUI
	KaiNotifyContactRemove(m_hBaseObj, aFriend, true);
}

void KaiManager::OnContactSpeexStatus( string aFriend, bool bSpeexEnabled )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it) {
		if( it->first != NULL )
			it->first->OnContactSpeexStatus( aFriend, bSpeexEnabled );
	}

	// Broadcast this message to XUI
	KaiNotifyContactSpeexStatus(m_hBaseObj, aFriend, bSpeexEnabled, true);
}

void KaiManager::OnContactSpeexRing( string aFriend )
{	
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it) {
		if( it->first != NULL )
			it->first->OnContactSpeexRing( aFriend );
	}

	// Broadcast this message to XUI
	KaiNotifyContactSpeexRing(m_hBaseObj, aFriend, true);
}

void KaiManager::OnContactSpeex( string aFriend )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it) {
		if( it->first != NULL )
			it->first->OnContactSpeex( aFriend );	
	}

	// Broadcast this message to XUI
	KaiNotifyContactSpeex(m_hBaseObj, aFriend, true);
}

void KaiManager::OnContactInvite( string aFriend, string aVector, string aTime, string aMessage )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it) {
		if( it->first != NULL )
			it->first->OnContactInvite( aFriend, aVector, aTime, aMessage );
	}

	// Broadcast this message to XUI
	KaiNotifyContactInvite(m_hBaseObj, aFriend, aVector, aTime, aMessage, true);
}

void KaiManager::OnEnterArena( string aVector, BOOL bCanHost )
{	
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnEnterArena( aVector, bCanHost );
	}

	// Broadcast this message to XUI
	KaiNotifyEnterArena(m_hBaseObj, aVector, bCanHost, true);
}

void KaiManager::OnEnterArenaFailed( string aVector, string aReason )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it) {
		if( it->first != NULL )
			it->first->OnEnterArenaFailed( aVector, aReason );
	}

	// Broadcast this message to XUI
	KaiNotifyEnterArenaFailed(m_hBaseObj, aVector, aReason, true);
}

void KaiManager::OnNewArena( string aVector, string aDescription, int nPlayers, int nPlayerLimit, int nPassword, int aSubVectors, bool bPersonal )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it) {
		if( it->first != NULL )
			it->first->OnNewArena( aVector, aDescription, nPlayers, nPlayerLimit, nPassword, aSubVectors, bPersonal );
	}

	// Broadcast this message to XUI
	KaiNotifyNewArena(m_hBaseObj, aVector, aDescription, nPlayers, nPlayerLimit, nPassword, bPersonal, aSubVectors, true);
}

void KaiManager::OnUpdateArena( string aVector, int nPlayers, int aSubVectors)
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it) {
		if( it->first != NULL )
			it->first->OnUpdateArena( aVector, nPlayers, aSubVectors );
	}

	// Broadcast this message to XUI
	KaiNotifyUpdateArena(m_hBaseObj, aVector, nPlayers, aSubVectors, true);
}

void KaiManager::OnUpdateOpponent( string aOpponent, string aAge, string aBandwidth, string aLocation, string aBio )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it) {
		if( it->first != NULL )
			it->first->OnUpdateOpponent( aOpponent, aAge, aBandwidth, aLocation, aBio );
	}

	// Broadcast this message to XUI
	KaiNotifyUpdateOpponentInfo(m_hBaseObj, aOpponent, aAge, aBandwidth, aLocation, aBio, true);
}

void KaiManager::OnUpdateOpponent( string aOpponent, string aAvatarURL )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it) {
		if( it->first != NULL )
			it->first->OnUpdateOpponent( aOpponent, aAvatarURL );
	}

	// Broadcast this message to XUI
	KaiNotifyUpdateOpponentAvatar(m_hBaseObj, aOpponent, aAvatarURL, true);
}

void KaiManager::OnUpdateHostingStatus( BOOL bIsHosting )
{
	//DebugMsg("DEBUG", "CRASH B");
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it) {
		if( it->first != NULL )
			it->first->OnUpdateHostingStatus( bIsHosting );
	}

//	/DebugMsg("DEBUG", "CRASH C");

	// Broadcast this message to XUI
	KaiNotifyUpdateHostingStatus(m_hBaseObj, bIsHosting, true);

//	DebugMsg("DEBUG", "CRASH D");
}

void KaiManager::OnOpponentEnter( string aOpponent )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnOpponentEnter( aOpponent );
	}

	// Broadcast this message to XUI
	KaiNotifyOpponentEnter(m_hBaseObj, aOpponent, true);
}

void KaiManager::OnOpponentPing( string aOpponent, DWORD aPing, int aStatus, string aBearerCapability )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnOpponentPing( aOpponent, aPing, aStatus, aBearerCapability );
	}

	// Broadcast this message to XUI
	KaiNotifyOpponentPing(m_hBaseObj, aOpponent, aPing, aStatus, aBearerCapability, true);
}

void KaiManager::OnOpponentLeave( string aOpponent )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnOpponentLeave( aOpponent );
	}

	// Broadcast this message to XUI
	KaiNotifyOpponentLeave(m_hBaseObj, aOpponent, true);
}

void KaiManager::OnSupportedTitle( DWORD aTitleId, string aVector )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnSupportedTitle( aTitleId, aVector );
	}

	// Broadcast this message to XUI
	KaiNotifySupportedTitle(m_hBaseObj, aTitleId, aVector, true);
}

void KaiManager::OnJoinsChat( string aOpponent )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnJoinsChat( aOpponent );
	}

	// Broadcast this message to XUI
	KaiNotifyJoinsChat(m_hBaseObj, aOpponent, true);
}

void KaiManager::OnChat( string aVector, string aOpponent, string aMessage, bool bPrivate )
{	
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnChat( aVector, aOpponent, aMessage, bPrivate );
	}

	// Broadcast this message to XUI
	KaiNotifyChat(m_hBaseObj, aVector, aOpponent, aMessage, bPrivate, true);
}

void KaiManager::OnLeavesChat( string aOpponent )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnLeavesChat( aOpponent );
	}

	// Broadcast this message to XUI
	KaiNotifyLeavesChat(m_hBaseObj, aOpponent, true);
}

void KaiManager::OnChatPrivateMessage( string aPlayer, string aMessage, string aVector, bool bContactList )
{
	// Send Notice to Classes first
	for(ClassItem::const_iterator it = _ClassObservers.begin(); it != _ClassObservers.end(); ++it)  {
		if( it->first != NULL )
			it->first->OnChatPrivateMessage( aPlayer, aMessage, aVector, bContactList );
	}

	// Broadcast this messsage to XUI
	KaiNotifyChatPrivateMessage( m_hBaseObj, aPlayer, aMessage, aVector, bContactList, true );
}

//////////////////////////////////////////////
//											//
//   Here is where we make our custom		//
//	 commands, ideally this is the only		//
//	 class that talks to CKaiClient			//
//											//
//////////////////////////////////////////////

void KaiManager::SendPrivateMessage( string szPlayer, string szMessage )
{
	DebugMsg("KaiManager", "Sending Private Message");
	CKaiClient::getInstance().KaiSendPM( szPlayer, szMessage );
}