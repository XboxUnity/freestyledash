#pragma once
#include "../../Generic/xboxtools.h"
#include "../../Generic/tools.h"
#include "../KaiClient.h"
#include "../KaiMessage.h"
#include "../KaiUser.h"

typedef struct
{
	DWORD titleID;
	bool launchGame;
} VECTOR_INFO, *pVECTOR_INFO;

class KaiManager : public iBuddyObserver
{
public:
	static KaiManager& getInstance()
	{
		static KaiManager singleton;
		return singleton;
	}

	// Wrapper for CKaiClient
	HRESULT KaiLeaveChatmode( void ) { CKaiClient::getInstance().KaiLeaveChatMode(); return S_OK; }
	HRESULT KaiEnableChatmode( void ) { CKaiClient::getInstance().KaiEnableChatMode(); return S_OK; }
	HRESULT KaiQueryClientState( void ) { CKaiClient::getInstance().KaiQuery(); return S_OK; }
	HRESULT KaiLogin( string szUsername, string szPassword ) { CKaiClient::getInstance().KaiLogin(szUsername.c_str(), szPassword.c_str()); return S_OK; }
	HRESULT KaiShutdown( void ) { CKaiClient::getInstance().KaiShutdown(); return S_OK; }
	HRESULT KaiAttachToClient( void );
	HRESULT KaiEnterArena(string szVector, string szPassword, bool isAbsolute = false) { CKaiClient::getInstance().KaiEnterArena( szVector, szPassword, isAbsolute); return S_OK; }
	HRESULT KaiLeaveArena(bool bReturnToRoot = false);
	HRESULT KaiChangeChatMode( string szChatmode, bool bSyncToVector = false) { CKaiClient::getInstance().KaiChangeChatmode( szChatmode, bSyncToVector ); return S_OK; }
	HRESULT KaiQueryAvatar(string szPlayerName) { CKaiClient::getInstance().KaiQueryAvatar(szPlayerName); return S_OK; }
	HRESULT KaiQueryUserProfile(string szPlayerName) { CKaiClient::getInstance().KaiQueryUserProfile(szPlayerName); return S_OK; }
	HRESULT KaiChat(string szMessage) { CKaiClient::getInstance().KaiChat(szMessage); return S_OK; }
	HRESULT KaiMessengerMode( void ) { CKaiClient::getInstance().KaiMessengerMode(); return S_OK; }
	HRESULT KaiArenaMode( void ) { CKaiClient::getInstance().KaiArenaMode(); return S_OK; }
	HRESULT KaiHost( void ) { CKaiClient::getInstance().KaiHost(); return S_OK; }
	HRESULT KaiHostArena( string szPassword, string szDescription, int nPlayerLimit) { CKaiClient::getInstance().KaiHostArena(szPassword, szDescription, nPlayerLimit); return S_OK; }
	HRESULT KaiSendMessage( string szMessage ) { CKaiClient::getInstance().KaiSendMessage(szMessage); return S_OK; }
	HRESULT KaiBuddyAdd( string szPlayerName ) { CKaiClient::getInstance().KaiAddContact(szPlayerName); return S_OK; }
	HRESULT KaiBuddyRemove( string szPlayerName ) { CKaiClient::getInstance().KaiRemoveContact(szPlayerName); return S_OK; }
	HRESULT KaiArenaInvite( string szPlayerName, string szArena, string szMessage ) { CKaiClient::getInstance().KaiInvite(szPlayerName, szArena, szMessage); return S_OK; }
	HRESULT KaiSendPM ( string szPlayerName, string szMessage ) { CKaiClient::getInstance().KaiSendPM(szPlayerName, szMessage); return S_OK; }

	bool KaiIsEngineConnected( void ) { return CKaiClient::getInstance().IsEngineConnected(); }
	BOOL KaiIsInPrivate( void ) { return CKaiClient::getInstance().IsInPrivate(); }

	string KaiGetCurrentUsername( void ) { return CKaiClient::getInstance().Client.Username; }


	// Control Routines
	void SendPrivateMessage( string szPlayer, string szMessage );

	HRESULT XlinkSignOut( void );
	HRESULT XlinkSignIn( CKaiUser * kaiUser );
	HRESULT GetDefaultKaiUser( CKaiUser ** kaiUser );
	HRESULT QueryProfileXuid(XUID xuid, CKaiUser ** kaiUser);
	HRESULT SearchForEngine();
	HRESULT QueryClientState() { CKaiClient::getInstance().KaiQuery(); return S_OK; }
	HRESULT ShutdownKai( );
	HRESULT LoginToKai(string szUsername, string szPassword);


	// Register Classes
	void RegisterClass(iBuddyObserver &ref);
	void UnregisterClass(iBuddyObserver &ref);

	// IBuddyObsverer Implementation
	void OnDeInitialise();
	void OnEngineDetached();
	void OnEngineAttached();
	void OnEngineInUse();
	void OnAccountNotLoggedIn(string aUsername, string aPassword);
	void OnAccountLoggedIn();
	void OnAuthenticationFailed(string aUsername);
	void OnNetworkError(string aError);
	void OnNetworkReachable(string aServerName);
	void OnContactOffline(string aFriend);
	void OnContactOnline(string aFriend);
	void OnContactsOnline(int nCount);
	void OnContactPing(string aFriend, string aVector, DWORD aPing, int aStatus, string aBearerCapability);
	void OnContactAdd(string aFriend);
	void OnContactRemove(string aFriend);
	void OnContactSpeexStatus(string aFriend, bool bSpeexEnabled);
	void OnContactSpeexRing(string aFriend);
	void OnContactSpeex(string aFriend);
	void OnContactInvite(string aFriend, string aVector, string aTime, string aMessage);
	void OnEnterArena(string aVector, BOOL bCanHost);
	void OnEnterArenaFailed(string aVector, string aReason);
	void OnNewArena( string aVector, string aDescription,
						   int nPlayers, int nPlayerLimit, int nPassword, int aSubVectors, bool bPersonal );
	void OnUpdateArena( string aVector, int nPlayers, int aSubVectors );
	void OnUpdateOpponent(string aOpponent, string aAge,
								string aBandwidth, string aLocation, string aBio);
	void OnUpdateOpponent(string aOpponent, string aAvatarURL);
	void OnUpdateHostingStatus(BOOL bIsHosting);
	void OnOpponentEnter(string aOpponent);
	void OnOpponentPing(string aOpponent, DWORD aPing, int aStatus, string aBearerCapability);
	void OnOpponentLeave(string aOpponent);
	void OnSupportedTitle(DWORD aTitleId, string aVector);
	void OnJoinsChat(string aOpponent);
	void OnChat(string aVector, string aOpponent, string aMessage, bool bPrivate);
	void OnLeavesChat(string aOpponent);
	void OnChatPrivateMessage(string aPlayer, string aMessage, string aVector, bool bContactList);

private:	

	//Root Object HXUIOBJ
	HXUIOBJ m_hBaseObj;

	std::map<iBuddyObserver* const,iBuddyObserver* const> _ClassObservers;
	typedef std::map<iBuddyObserver* const, iBuddyObserver* const> ClassItem;
	
	KaiManager();
	~KaiManager( ) {}
	KaiManager(const KaiManager&);		            // Prevent copy-construction
	KaiManager& operator=(const KaiManager&);		    // Prevent assignment

};