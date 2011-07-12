//***********************************************************************************//
//*		The original code for this KaiClient came from Team XBMC and has been		*//
//*		modified and repurposed by Team FSD.  This code is subject to the same		*//
//*		GNU General Public License that the rest of FSD2.0 is licensed under.		*//
//*		Please give credit where credit is due if reusing this code.				*//
//***********************************************************************************//

#pragma once
#include "UdpClient.h"
#include "KaiRequestList.h"
#ifdef HAS_KAI_VOICE
#include "KaiVoice.h"
#include "Xbox/VoiceManager.h"
#include "Xbox/MediaPacketQueue.h"
#include "Stopwatch.h"
#endif


#define STRTOKEN(x, y, z, w)		if(z < y.size()){ if(y.at(z) != "") x = y.at(z); else x = w; } else x = w;

#define KAI_SYSTEM_PORT  34522
#define KAI_SYSTEM_ROOT  "Arena"
#define KAI_XBOX360_ROOT "Arena\XBox 360"
#define KAI_XBOXCLASSIC_ROOT "Arena\XBox"

class iBuddyObserver;
using namespace std;

typedef enum _ClientState { 
	Discovering, 
	Attaching, 
	Querying, 
	LoggingIn, 
	Authenticated, 
	Disconnected 
} ClientState;

typedef enum _Item { 
	Unknown, Player, Arena 
}Item;

typedef struct _ServerInfo {
	SOCKADDR_IN SocketAddress;
	string IpAddress;
	DWORD Flags;
} ServerInfo;

typedef struct _ClientTimers {
	double m_dwSettlingTimer;
	double m_dwReachableTimer;
}ClientTimers;

typedef struct _ClientFlags {
	BOOL m_bHeadset;
	BOOL m_bHosting;
	BOOL m_bContactsSettling;
	BOOL m_bReachable;
	BOOL m_bChangeChatChannel;
	BOOL m_bChatEnabled;
	BOOL m_bClientRunning;
	BOOL m_bClientLoggedIn;
	BOOL m_bInPrivate;
} ClientFlags;

typedef struct _ClientInfo {
	ClientState State;
	ClientFlags Flags;
	ClientTimers Timers;
	string Vector;
	string Username;
	string Password;
	iBuddyObserver * Observer;
	int FriendsOnline;
} ClientInfo;

class iBuddyObserver
{
public:
	virtual ~iBuddyObserver() {}
	virtual void OnDeInitialise(){};
	virtual void OnEngineDetached(){};
	virtual void OnEngineAttached(){};
	virtual void OnEngineInUse(){};
	virtual void OnAccountNotLoggedIn(string aUsername, string aPassword) {};
	virtual void OnAccountLoggedIn() {};
	virtual void OnAuthenticationFailed(string aUsername) {};
	virtual void OnNetworkError(string aError) {};
	virtual void OnNetworkReachable(string aOrbitalServerName) {};
	virtual void OnContactOffline(string aContact) {};
	virtual void OnContactOnline(string aContact) {};
	virtual void OnContactsOnline(int nCount) {};
	virtual void OnContactPing(string aContact, string aVector, DWORD aPing, int aStatus, string aBearerCapability) {};
	virtual void OnContactAdd(string aContact) {};
	virtual void OnContactRemove(string aContact) {};
	virtual void OnContactSpeexStatus(string aContact, bool bSpeexEnabled) {};
	virtual void OnContactSpeexRing(string aContact) {};
	virtual void OnContactSpeex(string aContact) {};
	virtual void OnContactInvite(string aContact, string aVector, string aTime, string aMessage) {};
	virtual void OnSupportedTitle(DWORD dwTitle, string aVector) {};
	virtual void OnEnterArena(string aVector, BOOL bCanCreate) {};
	virtual void OnEnterArenaFailed(string aVector, string aReason) {};
	virtual void OnOpponentEnter(string aContact) {};
	virtual void OnOpponentPing(string aOpponent, DWORD aPing, int aStatus, string aBearerCapability) {};
	virtual void OnOpponentLeave(string aContact) {};
	virtual void OnNewArena(string aVector, string aDescription,
							int nPlayers, int nPlayerLimit, int nPassword, int aSubVectors, bool bPersonal) {};
	virtual void OnUpdateArena(string aVector, int nPlayers, int aSubVectors) {};
	virtual void OnUpdateOpponent(string aOpponent, string aAge, string aBandwidth,
								  string aLocation, string aBio) {};
	virtual void OnUpdateOpponent(string aOpponent, string aAvatarURL) {};
	virtual void OnUpdateHostingStatus(BOOL bHosting) {};
	virtual void OnJoinsChat(string aOpponent) {};
	virtual void OnChat(string aVector, string aOpponent, string aMessage, bool bPrivate) {};
	virtual void OnLeavesChat(string aOpponent) {};
	virtual void OnChatPrivateMessage(string aPlayer, string aMessage, string aVector, bool bContactList) {};
};


class CKaiClient : public CUdpClient
{
public:
	CKaiClient();
	~CKaiClient();

	ServerInfo Server;
	ClientInfo Client;

	static CKaiClient& getInstance() {
		static CKaiClient singleton;
		return singleton;
	}

	// Observer registration and removal methods
	void SetObserver(iBuddyObserver* aObserver, bool bConnect = false);
	void RemoveObserver( void );
	
	// Kai Related Commands - Network/Connection
	void KaiDiscover(void );
	void KaiAttach(SOCKADDR_IN& aAddress);
	void KaiTakeover( void );
	void KaiConnect( void );
	void ResetServer( void );
	void KaiDetach( void );
	void KaiShutdown( void );
	void KaiLogin(LPCSTR aUsername, LPCSTR aPassword);
	void KaiLogOut( void );
	bool IsEngineConnected( void );
	bool IsNetworkReachable( void );
	BOOL IsInPrivate( void );

	// Kai Related Commands - Modes and Preset Shortcuts
	void KaiMessengerMode( void );
	void KaiArenaMode( void );
	void KaiXbox360Mode( void ) {}			// Not Implemented
	void KaiXboxClassicMode( void ) {}		// Not Implemented

	// Kai Related Commands - Vectors / Arenas
	void KaiEnterVector(string aVector, string aPassword);
	void KaiLeaveVector( void );
	void KaiEnterArena(string aVector, string aPassword,  bool isAbsolute = false);
	void KaiLeaveArena( void );
	void KaiGetSubVectors(string aVector);
	string KaiGetCurrentVector( void );
	void KaiHost( void );
	void KaiHostArena(string aPassword, string aDescription, int aPlayerLimit);
	void KaiSetHostingStatus(BOOL bIsHosting);
	void KaiSetBearerCaps(BOOL bIsHeadsetPresent);
	inline BOOL KaiIsHosting( void ) const { return Client.Flags.m_bHosting; }
	
	// Kai Related Commands - Queries
	void KaiQuery( void );
	void KaiQueryClientMetrics( void );
	void KaiQueryVectorPlayerCount(string aVector);
	void KaiQueryAvatar(string aPlayerName);
	void KaiQueryUserProfile(string aPlayerName);

	// Kai Related Commands - Chat
	void KaiEnableChatMode();
	void KaiLeaveChatMode();
	void KaiSendMessage(string aMessage);
	void KaiChangeChatmode( string aVector, bool bSyncToVector );
	void KaiChat(string aMessage);
	void KaiJoinTextChat( void );

	// Kai Related Commands - Contacts/Buddies
	void KaiAddContact(string aContact);
	void KaiRemoveContact(string aContact);
	void KaiInvite(string aPlayer, string aVector, string aMessage);
	void KaiSendPM(string aPlayer, string aMessage);

	// Kai Related Commands - Voice / Speex
	void KaiEnableContactVoice(string aContactName, BOOL bEnable = TRUE);
	void KaiVoiceChatStart( void );
	void KaiVoiceChatStop( void );	
	void KaiQueueContactVoice(string aContactName, DWORD aPlayerId, LPBYTE pMessage, DWORD dwMessageLength);
	void KaiOnVoiceData( DWORD dwPort, DWORD dwSize, VOID* pvData );
	void KaiSendVoiceDataToEngine( void );
	#ifdef HAS_KAI_VOICE
		void OnCommunicatorEvent( DWORD dwPort, VOICE_COMMUNICATOR_EVENT event );
	#endif

	void DoWork( void );

	#ifdef HAS_KAI_VOICE
	// Public so that CVoiceManager can get to them
	static void VoiceDataCallback( DWORD dwPort, DWORD dwSize, VOID* pvData, VOID* pContext );
	static void CommunicatorCallback( DWORD dwPort, VOICE_COMMUNICATOR_EVENT event, VOID* pContext );
	#endif

protected:
	virtual void OnMessage(SOCKADDR_IN& aRemoteAddress, string aMessage,
						 LPBYTE pMessage, DWORD dwMessageLength);

	CKaiClient(const CKaiClient&);		            // Prevent copy-construction
	CKaiClient& operator=(const CKaiClient&);		    // Prevent assignment

private:
	DWORD Crc32FromString(string aString);

	ATG::Timer Timer;
	#ifdef HAS_KAI_VOICE
		// KAI Speex support
		LPDIRECTSOUND8 m_pDSound;
		CMediaPacketQueue* m_pEgress;
		CStopWatch m_VoiceTimer;
	#endif
	
	// KAI Requests
	CKaiRequestList * m_pRequestList;
};
