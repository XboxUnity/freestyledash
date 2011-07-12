//***********************************************************************************//
//*		The original code for this KaiClient came from Team XBMC and has been		*//
//*		modified and repurposed by Team FSD.  This code is subject to the same		*//
//*		GNU General Public License that the rest of FSD2.0 is licensed under.		*//
//*		Please give credit where credit is due if reusing this code.				*//
//***********************************************************************************//
#pragma once
#include "stdafx.h"
#include "KaiClient.h"
#include "KaiRequestList.h"
#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../../Tools/Debug/Debug.h"

#ifdef HAS_KAI_VOICE
#include "dsstdfx.h"
#define SPEEX_LOOPBACK					1
#endif

#define KAI_CONTACT_SETTLING_PERIOD		5000L
#define KAI_REACHABLE_QUERY_PERIOD		15000L

#ifdef SPEEX_LOOPBACK
DWORD g_speexLoopbackPlayerId = 0xDEADBEEF;
#endif

CKaiClient::CKaiClient()
{
	// Constructor
	DebugMsg("KaiClient", "KAICLIENT: Instantiating...");
	Client.Observer = NULL;
	Client.Flags.m_bHeadset = FALSE;
	Client.Flags.m_bHosting = FALSE;
	Client.Flags.m_bContactsSettling = TRUE;
	Client.Flags.m_bReachable = FALSE;
	Client.Flags.m_bChangeChatChannel = FALSE;
	Client.Flags.m_bChatEnabled = FALSE;
	Client.Flags.m_bClientRunning = TRUE;
	Client.Flags.m_bClientLoggedIn = FALSE;
	Client.Flags.m_bInPrivate = FALSE;
	Client.FriendsOnline = 0;
	
	Server.Flags = 0;
	Server.IpAddress = "";

	// Create a new request list pointer
	m_pRequestList = new CKaiRequestList();

	#ifdef HAS_KAI_VOICE
		m_pDSound = NULL;

		// outbound packet queue, collects compressed audio until sufficient to send
		string strEgress = "egress";
		m_pEgress = new CMediaPacketQueue(strEgress);
	#endif

	// Create the UDPClient Thread for accepting/transmitting xlink messages
	Create();

	DebugMsg("KaiClient", "KAICLIENT: Ready.");

}

CKaiClient::~CKaiClient()
{
	// Deconstructor
}

void CKaiClient::ResetServer()
{
	if(!Client.Flags.m_bClientRunning)
	{
		DebugMsg("KaiClient", "KAICLIENT: Resetting...");
		Client.Flags.m_bHeadset = FALSE;
		Client.Flags.m_bHosting = FALSE;
		Client.Flags.m_bContactsSettling = TRUE;
		Client.Flags.m_bReachable = FALSE;
		Client.Flags.m_bChangeChatChannel = FALSE;
		Client.Flags.m_bChatEnabled = FALSE;
		Client.Flags.m_bClientRunning = TRUE;
		Client.Flags.m_bClientLoggedIn = FALSE;
		Client.Flags.m_bInPrivate = FALSE;
		Client.State = Disconnected;
		Client.FriendsOnline = 0;

		Server.Flags = 0;
		Server.IpAddress = "";

		// Create a new request list pointer
		m_pRequestList = new CKaiRequestList();

		// Create the UDPClient Thread for accepting/transmitting xlink messages
		Create();

		DebugMsg("KaiClient", "KAICLIENT: Ready.");
	}
}

void CKaiClient::KaiQueryVectorPlayerCount(string aVector)
{
	string szQueryMessage;
	szQueryMessage = sprintfaA("KAI_CLIENT_SPECIFIC_COUNT;%s;", aVector.c_str());
	m_pRequestList->QueueRequest(szQueryMessage);
}

void CKaiClient::KaiVoiceChatStart()
{
	#ifdef HAS_KAI_VOICE
		if (!m_pDSound)
		{
			DebugMsg("KaiClient", "KAICLIENT: Initializing DirectSound...");
			if (FAILED(DirectSoundCreate( NULL, &m_pDSound, NULL )))
			{
				//CLog::Log(LOGERROR, "KAICLIENT: Failed to initialize DirectSound.");
			}

			DSEFFECTIMAGELOC dsImageLoc;
			dsImageLoc.dwI3DL2ReverbIndex = GraphI3DL2_I3DL2Reverb;
			dsImageLoc.dwCrosstalkIndex = GraphXTalk_XTalk;

			DebugMsg("KaiClient", "KAICLIENT: Loading sound effects image...");
			LPDSEFFECTIMAGEDESC pdsImageDesc;

			if (FAILED(XAudioDownloadEffectsImage( "dsstdfx", &dsImageLoc, XAUDIO_DOWNLOADFX_XBESECTION, &pdsImageDesc )))
			{
				//CLog::Log(LOGERROR, "KAICLIENT: Failed to load image.");
			}

			// Configure the voice manager.
			VOICE_MANAGER_CONFIG VoiceConfig;
			VoiceConfig.dwVoicePacketTime = 20;      // 20ms per microphone callback
			VoiceConfig.dwMaxRemotePlayers = 12;      // 12 remote players
			VoiceConfig.dwFirstSRCEffectIndex = GraphVoice_Voice_0;
			VoiceConfig.pEffectImageDesc = pdsImageDesc;
			VoiceConfig.pDSound = m_pDSound;
			VoiceConfig.dwMaxStoredPackets = MAX_VOICE_PER_SEND;

			VoiceConfig.pCallbackContext = this;
			VoiceConfig.pfnCommunicatorCallback = CommunicatorCallback;
			VoiceConfig.pfnVoiceDataCallback = VoiceDataCallback;

			//CLog::Log(LOGNOTICE, "KAICLIENT: Initializing voice manager...");
			if (FAILED(g_VoiceManager.Initialize( &VoiceConfig )))
			{
				//CLog::Log(LOGERROR, "KAICLIENT: Failed to initialize voice manager.");
			}

			DebugMsg("KaiClient", "KAICLIENT: Voice chat enabled.");

			g_VoiceManager.EnterChatSession();
			m_VoiceTimer.StartZero();

			#ifdef SPEEX_LOOPBACK
				g_VoiceManager.AddChatter(g_speexLoopbackPlayerId);
			#endif
		}
	#endif
}

void CKaiClient::KaiVoiceChatStop()
{
	#ifdef HAS_KAI_VOICE
		if (m_pDSound)
		{
			DebugMsg("KaiClient", "KAICLIENT: Releasing DirectSound...");
			g_VoiceManager.LeaveChatSession();
			// TODO: we really ought to shutdown, but somethings up - I can never seem to reinit properly.
			//  g_VoiceManager.Shutdown();
			m_pDSound->Release();
			m_pDSound = NULL;
			m_pEgress->Flush();
			DebugMsg("KaiClient", "KAICLIENT: Voice chat disabled.");
		}
	#endif
}

void CKaiClient::KaiShutdown()
{
	DebugMsg("KaiClient","KaiClient Destroyed");
	#ifdef HAS_KAI_VOICE
	KaiVoiceChatStop();
	KaiLeaveChatMode();
	#endif
	
	// LogOut of the client
	if (Client.State == Authenticated)
	{
		KaiLogOut();
	}

	Client.State = Disconnected;
	RemoveObserver();

	// Remove UDP listener
	Destroy();

	//Set flag so client will know to restart on next sign in
	Client.Flags.m_bClientRunning = FALSE;

	#ifdef HAS_KAI_VOICE
		if (m_pEgress)
			delete m_pEgress;
		m_pEgress=NULL;
	#endif

	if (m_pRequestList)
		delete m_pRequestList;
	m_pRequestList=NULL;
}

void CKaiClient::SetObserver(iBuddyObserver* aObserver, bool bConnect)
{
	if (Client.Observer != aObserver)
	{
		DebugMsg("KaiClient", "New Observer Created [0x%08X]", aObserver);

		// Enable call back methods
		Client.Observer = aObserver;

		// Check if Client is running. If not reset
		ResetServer();

		if(bConnect == true) 
		{
			// Mark time when observer was attached for timeout variables
			Client.Timers.m_dwSettlingTimer = Timer.GetAbsoluteTime();
			Client.Timers.m_dwReachableTimer = Timer.GetAbsoluteTime();

			// Check to ensure that the information is present
			if(Server.IpAddress.length() > 0 && Server.Flags == 0) {
				SOCKADDR_IN sockAddr;
				memset(&sockAddr, 0, sizeof(sockAddr));
				sockAddr.sin_family = AF_INET;
				sockAddr.sin_addr.s_addr = inet_addr(Server.IpAddress.c_str());
				sockAddr.sin_port = htons(KAI_SYSTEM_PORT);

				KaiAttach(sockAddr);
			}
			else
			{
				KaiDiscover();
			}
		}
		else
		{
			// bConnect flag = false,  do not proceed to connect to xlink engine
		}
	}
}

void CKaiClient::RemoveObserver( void )
{
	// Check to ensure the observer is not NULL and is valid
	if(Client.Observer != NULL) {
		//Detach Client
		KaiDetach();

		// Broadcast that the observer is being removed
		Client.Observer->OnDeInitialise();

		Client.Observer = NULL;

		if(Client.Observer == NULL)
			DebugMsg("KaiClient","Observer Removed");
	}
}

void CKaiClient::KaiEnterArena(string aVector, string aPassword, bool isAbsolute)
{
	if (Client.State == Authenticated)
	{
		DebugMsg("KaiClient","Client Vector %s", Client.Vector.c_str());
		if(strcmp(Client.Vector.c_str(), "") == 0) {
			string szArena = KAI_SYSTEM_ROOT;
			string szPass = "";
			Client.Flags.m_bChangeChatChannel = TRUE;
			KaiEnterVector(szArena, szPass);

			Client.Vector = szArena;
		}
		
		if(isAbsolute) // we have a absolute path... lets pass it
		{
			DebugMsg("KaiClient","Absolute Path");
			Client.Flags.m_bChangeChatChannel = TRUE;
			KaiEnterVector(aVector, aPassword);
		}
		else
		{
			string aNewVector;
			aNewVector = sprintfaA("%s/%s", Client.Vector.c_str(), aVector.c_str());
			Client.Flags.m_bChangeChatChannel = TRUE;
			KaiEnterVector(aNewVector, aPassword);
		}
	}
}

void CKaiClient::KaiLeaveArena( void )
{
	if(Client.State == Authenticated){
		Client.Flags.m_bChangeChatChannel = TRUE;
		KaiLeaveVector();
	}
}

void CKaiClient::KaiEnterVector(string aVector, string aPassword)
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send the KaiMessage
		string szVectorMessage;
		szVectorMessage = sprintfaA("KAI_CLIENT_VECTOR;%s;%s;", aVector.c_str(), aPassword.c_str());
		Send(Server.SocketAddress, szVectorMessage);
	}
}

void CKaiClient::KaiMessengerMode( void )
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		Send(Server.SocketAddress, "KAI_CLIENT_VECTOR;;;");
		Sleep(10);
		Send(Server.SocketAddress, "KAI_CLIENT_GET_VECTORS;;");
		Sleep(10);
		Send(Server.SocketAddress, "KAI_CLIENT_CHATMODE;General Chat;");
	}
}

void CKaiClient::KaiArenaMode( void )
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		Send(Server.SocketAddress, "KAI_CLIENT_CHATMODE;Arena;");
		Sleep(100);
		Send(Server.SocketAddress, "KAI_CLIENT_VECTOR;Arena;;");		
	}
}

void CKaiClient::KaiChangeChatmode( string aVector, bool bSyncToVector )
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		if(bSyncToVector == true) {
			string szChatmodeMessage;
			szChatmodeMessage = sprintfaA("KAI_CLIENT_CHATMODE;%s;", Client.Vector.c_str());
			Send(Server.SocketAddress, szChatmodeMessage);
		} else {
			string szChatmodeMessage;
			szChatmodeMessage = sprintfaA("KAI_CLIENT_CHATMODE;%s;", aVector.c_str());
			Send(Server.SocketAddress, szChatmodeMessage);
		}
	}
}

void CKaiClient::KaiEnableChatMode()
{
	if (Client.State == Authenticated)
	{
		Client.Flags.m_bChatEnabled = TRUE;
		KaiJoinTextChat();
	}
}

void CKaiClient::KaiJoinTextChat()
{
	if (Client.State == Authenticated)
	{
		string szVectorMessage;
		szVectorMessage = sprintfaA("KAI_CLIENT_CHATMODE;%s;", Client.Vector.c_str());
		Send(Server.SocketAddress, szVectorMessage);
	}
}

void CKaiClient::KaiLeaveChatMode( void )
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		Client.Flags.m_bChatEnabled = FALSE;
		string szChatmodeMessage = "KAI_CLIENT_CHATMODE;;";
		Send(Server.SocketAddress, szChatmodeMessage);
	}
}

void CKaiClient::KaiSendMessage(string aMessage)
{
	DebugMsg("CKaiClient", "Sending Message:  %s", aMessage.c_str());
	Send(Server.SocketAddress, aMessage);
}

void CKaiClient::KaiChat(string aMessage)
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		string szChatMessage;
		szChatMessage = sprintfaA("KAI_CLIENT_CHAT;%s;", aMessage.c_str());
		Send(Server.SocketAddress, szChatMessage);
	}
}

void CKaiClient::KaiSendPM(string aPlayer, string aMessage)
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		string szPrivateMessage;
		szPrivateMessage = sprintfaA("KAI_CLIENT_PM;%s;%s;", aPlayer.c_str(), aMessage.c_str());
		Send(Server.SocketAddress, szPrivateMessage);
	}
}

void CKaiClient::KaiAddContact(string aContact)
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		string szContact;
		szContact = sprintfaA("KAI_CLIENT_ADD_CONTACT;%s;", aContact.c_str());
		Send(Server.SocketAddress, szContact);
	}
}

void CKaiClient::KaiRemoveContact(string aContact)
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		string szContact;
		szContact = sprintfaA("KAI_CLIENT_REMOVE_CONTACT;%s;", aContact.c_str());
		Send(Server.SocketAddress, szContact);
	}
}

void CKaiClient::KaiInvite(string aPlayer, string aVector, string aMessage)
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		string szInvitationMessage;
		szInvitationMessage = sprintfaA("KAI_CLIENT_INVITE;%s;%s;%s;", aPlayer.c_str(), aVector.c_str(), aMessage.c_str());
		Send(Server.SocketAddress, szInvitationMessage);
	}
}

void CKaiClient::KaiHost()
{
	if (Client.State == Authenticated)
	{
		KaiSetHostingStatus(TRUE);
	}
}

BOOL CKaiClient::IsInPrivate()
{
	if (Client.State == Authenticated)
	{
		return Client.Flags.m_bInPrivate;
	}

	return FALSE;
}

void CKaiClient::KaiHostArena(string aPassword, string aDescription, int aPlayerLimit)
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		string szHostMessage;
		szHostMessage = sprintfaA("KAI_CLIENT_CREATE_VECTOR;%d;%s;%s;", aPlayerLimit, aDescription.c_str(), aPassword.c_str());
		//Set Hosting var to TRUE
		Client.Flags.m_bInPrivate = TRUE;
		Send(Server.SocketAddress, szHostMessage);
	}
}

void CKaiClient::KaiSetHostingStatus( BOOL bIsHosting )
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		string szStatusMessage;
		szStatusMessage = sprintfaA("KAI_CLIENT_ARENA_STATUS;%d;%d;", bIsHosting ? 2 : 1, 1);
		Send(Server.SocketAddress, szStatusMessage);
	}
}

void CKaiClient::KaiGetSubVectors(string aVector)
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		string szGetSubVectorMessage;
		szGetSubVectorMessage = sprintfaA("KAI_CLIENT_GET_VECTORS;%s;", aVector.c_str());
		Send(Server.SocketAddress, szGetSubVectorMessage);
	}
}

void CKaiClient::KaiLeaveVector()
{
	if (Client.State == Authenticated)
	{
		int vectorDelimiter = Client.Vector.rfind('/');
		if (vectorDelimiter > 0)
		{
			string previousVector = Client.Vector.substr(0, vectorDelimiter);
			string strPassword = "";
			KaiEnterVector(previousVector, strPassword);
		}
		else
		{
			DebugMsg("KaiClient","Error in Up %s client_vector", Client.Vector.c_str());
		}
	}
}

string CKaiClient::KaiGetCurrentVector()
{
	return Client.Vector;
}

void CKaiClient::KaiDiscover( void )
{
	DebugMsg("CKaiClient", "KAICLIENT:  Discovering Kai engine via UDP Broadcast.");
	// Set the client state to reflect our state
	Client.State = Discovering;
	
	// Send KaiMessages
	string szInitiateDiscoveryMessage = "KAI_CLIENT_DISCOVER;";
	Broadcast(KAI_SYSTEM_PORT, szInitiateDiscoveryMessage);
}

void CKaiClient::KaiDetach( void )
{
	// Ensure that the client user has been authenticated
	if(Client.State == Authenticated) {
		// Send KaiMessages
		string szDisconnectionMessage = "KAI_CLIENT_DETACH;";
		Send(Server.SocketAddress, szDisconnectionMessage);
	}
}

void CKaiClient::KaiAttach(SOCKADDR_IN& aAddress)
{
	char ip[16];
	IN_ADDR host;
	memcpy(&host, &aAddress.sin_addr, sizeof(host));

	#ifdef HAS_KAI_VOICE
		XNetInAddrToString(host, szIP, 32);
	#endif
	sprintf_s(ip, 16, "%d.%d.%d.%d", (byte)host.S_un.S_un_b.s_b1,
									 (byte)host.S_un.S_un_b.s_b2,
									 (byte)host.S_un.S_un_b.s_b3,
									 (byte)host.S_un.S_un_b.s_b4);

	DebugMsg("KaiClient", "KAICLIENT: Attach to Kai engine host at %s", ip);
	Server.IpAddress = ip;

	Client.State = Attaching;
	string szAttachMessage = "KAI_CLIENT_ATTACH;";
	Server.SocketAddress = aAddress;
	Send(Server.SocketAddress, szAttachMessage);
}

void CKaiClient::KaiTakeover()
{
	Client.State = Attaching;
	string szTakeOverMessage = "KAI_CLIENT_TAKEOVER;";
	Send(Server.SocketAddress, szTakeOverMessage);
}

void CKaiClient::KaiQuery()
{
	Client.State = Querying;
	string szQueryMessage = "KAI_CLIENT_GETSTATE;";
	Send(Server.SocketAddress, szQueryMessage);
}

void CKaiClient::KaiLogin(LPCSTR aUsername, LPCSTR aPassword)
{
	Client.State = LoggingIn;
	string szLoginMessage;
	szLoginMessage = sprintfaA("KAI_CLIENT_LOGIN;%s;%s;", aUsername, aPassword);
	Send(Server.SocketAddress, szLoginMessage);
}

void CKaiClient::KaiLogOut()
{
	string szLogoutMessage = "KAI_CLIENT_LOGOUT;";
	Send(Server.SocketAddress, szLogoutMessage);
}

bool CKaiClient::IsEngineConnected( void )
{
	return Client.State == Authenticated;
}

bool CKaiClient::IsNetworkReachable()
{
	return (Client.Flags.m_bReachable && Client.State == Authenticated);
}

void CKaiClient::KaiQueryUserProfile(string aPlayerName)
{
	if (Client.State == Authenticated)
	{
		string szQueryMessage;
		szQueryMessage = sprintfaA("KAI_CLIENT_GET_PROFILE;%s;", aPlayerName.c_str());
		Send(Server.SocketAddress, szQueryMessage);
	}
}

void CKaiClient::KaiQueryAvatar(string aPlayerName)
{
	if (Client.State == Authenticated)
	{
		string szQueryMessage;
		szQueryMessage = sprintfaA("KAI_CLIENT_AVATAR;%s;", aPlayerName.c_str());
		Send(Server.SocketAddress, szQueryMessage);
	}
}

void CKaiClient::KaiQueryClientMetrics()
{
	if (Client.State == Authenticated)
	{
		string szQueryMessage = "KAI_CLIENT_GET_METRICS;";
		Send(Server.SocketAddress, szQueryMessage);
	}
}

void CKaiClient::KaiSetBearerCaps(BOOL bIsHeadsetPresent)
{
	if (Client.State == Authenticated)
	{
		string szStatusMessage;
		szStatusMessage = sprintfaA("KAI_CLIENT_CAPS;01%s3;", bIsHeadsetPresent ? "2" : "" );
		Send(Server.SocketAddress, szStatusMessage);
	}
}

void CKaiClient::KaiEnableContactVoice(string aContactName, BOOL bEnable)
{
	if (Client.State == Authenticated)
	{
		string szVoiceMessage;
		szVoiceMessage = sprintfaA("KAI_CLIENT_SPEEX_%s;%s;", bEnable ? "ON" : "OFF",
									aContactName.c_str());
		Send(Server.SocketAddress, szVoiceMessage);
	}
}

void CKaiClient::KaiQueueContactVoice(string aContactName, DWORD aPlayerId, LPBYTE pMessage, DWORD dwMessageLength)
{
	#ifdef HAS_KAI_VOICE
	if (client_state == Authenticated)
	{
		string header;
		header.sprintfaA("KAI_CLIENT_SPEEX;%s;", aContactName.c_str());

		int frames = 0;
		for (DWORD messageOffset = header.length(); messageOffset < dwMessageLength; messageOffset += COMPRESSED_FRAME_SIZE)
		{
			frames++;

			// we need to persist this buffer before we can have it queued
			g_VoiceManager.ReceivePacket(
			aPlayerId,
			(LPVOID) &pMessage[messageOffset],
			COMPRESSED_FRAME_SIZE );
		}

		//char szDebug[128];
		//sprintf(szDebug,"RX KAI SPEEX %d frames, total: %u bytes.\r\n",frames,dwMessageLength);
		//OutputDebugString(szDebug);
	}
	#endif
}


void CKaiClient::KaiSendVoiceDataToEngine()
{
	#ifdef HAS_KAI_VOICE
	if (client_state == Authenticated)
	{
		if (!m_pEgress->IsEmpty())
		{
			CHAR* buffer = new CHAR[700];

			#ifndef SPEEX_LOOPBACK
				sprintf(buffer, "KAI_CLIENT_SPEEX;");
			#else
				sprintf(buffer, "KAI_CLIENT_SPEEX;SpeexLoopback;");
			#endif

			int nPackets = m_pEgress->Size();

			//char szDebug[128];
			//sprintf(szDebug,"Sending %d packets\r\n", nPackets);
			//OutputDebugString(szDebug);

			int messageOffset = strlen(buffer);
			for (int i = 0; i < nPackets; i++, messageOffset += COMPRESSED_FRAME_SIZE)
			{
				XMEDIAPACKET outboundPacket;
				m_pEgress->Read(outboundPacket);

				memcpy((LPVOID)&buffer[messageOffset], outboundPacket.pvBuffer, COMPRESSED_FRAME_SIZE);

				*outboundPacket.pdwStatus = XMEDIAPACKET_STATUS_SUCCESS;
			}

			#ifndef SPEEX_LOOPBACK
				Send(server_addr, (LPBYTE)buffer, messageOffset);
			#else
				string strName = "SpeexLoopback";
				QueueContactVoice(strName, g_speexLoopbackPlayerId, (LPBYTE)buffer, messageOffset);
			#endif
		}
	}

	#endif
	// reset buffer and stopwatch
	//m_VoiceTimer.StartZero();
}

void CKaiClient::OnMessage(SOCKADDR_IN& aRemoteAddress, string aMessage, LPBYTE pMessage, DWORD dwMessageLength)
{
	vector<string> vParameters;
	StringSplit(aMessage, ";", &vParameters, true);
	string szMessage = "";
	STRTOKEN(szMessage, vParameters, 0, "INVALID_MESSAGE");

	//DebugMsg("Message", "Message:  %s", aMessage.c_str());

	// now depending on state...
	switch (Client.State)
	{
	case Discovering:
		if (strcmp(szMessage.c_str(), "KAI_CLIENT_ENGINE_HERE") == 0)
		{
			IN_ADDR server;
			memcpy(&server, &aRemoteAddress.sin_addr, sizeof(server));

			string szKaiServerIP = sprintfaA("%d.%d.%d.%d", (byte)server.S_un.S_un_b.s_b1,
														 (byte)server.S_un.S_un_b.s_b2,
														 (byte)server.S_un.S_un_b.s_b3,
														 (byte)server.S_un.S_un_b.s_b4);
			#ifdef HAS_KAI_VOICE
				XNetInAddrToString(server, strKaiServer, 32);
			#endif
		}

		KaiAttach(aRemoteAddress);
		break;

	case Attaching:
		if (strcmp(szMessage.c_str(), "KAI_CLIENT_ATTACH") == 0)
		{
			DebugMsg("CKaiClient", "KAICLIENT:  Engine has attached to FSD Xlink client.");
			if(Client.Observer != NULL) {
				Client.Observer->OnEngineAttached();
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_ENGINE_IN_USE") == 0)
		{
			DebugMsg("CKaiClient", "KAICLIENT:  Engine is in use by another client, issuing 'Takeover' command.");
			if(Client.Observer != NULL){
				Client.Observer->OnEngineInUse();
			}
			KaiTakeover();
		}
		break;

	case Querying:
		if (strcmp(szMessage.c_str(), "KAI_CLIENT_LOGGED_IN") == 0)
		{
			Client.State = Authenticated;
			Client.Timers.m_dwSettlingTimer = Timer.GetAbsoluteTime();
			Client.Timers.m_dwReachableTimer = Timer.GetAbsoluteTime();
			
			DebugMsg("CKaiClient", "KAICLIENT:  Client is authenticated and logged in.");
			if(Client.Observer != NULL){
				Sleep(2000);
				Client.Observer->OnAccountLoggedIn();
				Client.Flags.m_bClientLoggedIn = TRUE;
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_NOT_LOGGED_IN") == 0)
		{
			STRTOKEN(Client.Username, vParameters, 1, "");
			STRTOKEN(Client.Password, vParameters, 2, "");
			
			DebugMsg("CKaiClient", "KAICLIENT:  Client is not logged in and requires username/password.");
			if(Client.Observer != NULL){
				Client.Observer->OnAccountNotLoggedIn(Client.Username, Client.Password);
			}
		}
		break;

	case LoggingIn:
		if (strcmp(szMessage.c_str(), "KAI_CLIENT_USER_DATA") == 0)
		{
			Client.State = Authenticated;

			Client.Timers.m_dwSettlingTimer = Timer.GetAbsoluteTime();
			Client.Timers.m_dwReachableTimer = Timer.GetAbsoluteTime();

			string szCaseCorrectedName;
			STRTOKEN(szCaseCorrectedName, vParameters, 1, "");

			// If a case corrected name is returned, update our global username
			if (szCaseCorrectedName.length() > 0)
				Client.Username = szCaseCorrectedName;
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_AUTHENTICATION_FAILED") == 0)
		{
			if (Client.Observer != NULL)
			{
				string szUsername = Client.Username;
				Client.Observer->OnAuthenticationFailed(szUsername);
			}
		}
    break;

	case Authenticated:
		//DebugMsg("UDPCLIENT", "MESSAGE RX:  %s", aMessage.c_str());
		if ((strcmp(szMessage.c_str(), "KAI_CLIENT_ADD_CONTACT") == 0))
		{
			if(Client.Observer != NULL)
			{
				string szContactName;
				STRTOKEN(szContactName, vParameters, 1, "");
				Client.Observer->OnContactAdd(szContactName);
			}
		}
		else if ((strcmp(szMessage.c_str(), "KAI_CLIENT_REMOVE_CONTACT") == 0))
		{
			if(Client.Observer != NULL)
			{
				string szContactName;
				STRTOKEN(szContactName, vParameters, 1, "");
				Client.Observer->OnContactRemove(szContactName);
			}
		}
		else if ((strcmp(szMessage.c_str(), "KAI_CLIENT_CONTACT_OFFLINE") == 0))
		{
			Client.FriendsOnline--;
			if (Client.Observer != NULL)
			{
				string szContactName;
				STRTOKEN(szContactName, vParameters, 1, "");
				Client.Observer->OnContactOffline( szContactName );
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_CONTACT_ONLINE") == 0)
		{
			Client.FriendsOnline++;
			if (Client.Observer != NULL)
			{
				string szContactName;
				STRTOKEN(szContactName, vParameters, 1, "");
				Client.Observer->OnContactOnline( szContactName );
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_SPEEX_ON") == 0)
		{
			if (Client.Observer != NULL)
			{
				string szContactName;
				STRTOKEN(szContactName, vParameters, 1, "");
				Client.Observer->OnContactSpeexStatus(szContactName, true);
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_SPEEX_OFF") == 0)
		{
			if (Client.Observer != NULL)
			{
				string szContactName;
				STRTOKEN(szContactName, vParameters, 1, "");

				Client.Observer->OnContactSpeexStatus(szContactName, false);
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_SPEEX_START") == 0)
		{
			// Future implementation of voice chat support
			#ifdef HAS_KAI_VOICE
			KaiVoiceChatStart();
			#endif
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_SPEEX") == 0)
		{
			string szContactName;
			STRTOKEN(szContactName, vParameters, 1, "");

			#ifdef HAS_KAI_VOICE
			if (m_pDSound)
			{
				DWORD playerId = Crc32FromString(strContactName);
				BOOL bRegistered = g_VoiceManager.IsPlayerRegistered( playerId );
				BOOL bAvailable = TRUE;

				if (!bRegistered)
				{
					//string strDebug;
					//strDebug.sprintfaA("Adding chatter %s\r\n",strContactName.c_str());
					//OutputDebugString(strDebug.c_str());
					bAvailable = SUCCEEDED(g_VoiceManager.AddChatter(playerId));
				}
				if (bAvailable)
				{
					QueueContactVoice(strContactName, playerId, pMessage, dwMessageLength);
					if (observer != NULL)
					{
						observer->OnContactSpeex(strContactName);
					}
				}
				else
				{
					//CLog::Log(LOGERROR, "Failed to queue contact voice.");
				}
			}
			#endif
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_SPEEX_STOP") == 0)
		{
			// Future implementation of voice chat support
			#ifdef HAS_KAI_VOICE
			KaiVoiceChatStop();
			#endif
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_SPEEX_RING") == 0)
		{
			if (Client.Observer != NULL)
			{
				string szContactName;
				STRTOKEN(szContactName, vParameters, 1, "");
				Client.Observer->OnContactSpeexRing( szContactName );
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_CONTACT_PING") == 0)
		{

			string szContactName, szVector, szPing, szCaps;

			STRTOKEN(szContactName, vParameters, 1, "");
			STRTOKEN(szVector, vParameters, 2, "Home");
			STRTOKEN(szPing, vParameters, 3, "");
			STRTOKEN(szCaps, vParameters, 4, "");
			if (Client.Observer != NULL) {
				Client.Observer->OnContactPing(szContactName, szVector,
									strtoul(szPing.c_str(), NULL, 10), 0, szCaps);
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_INVITE") == 0)
		{

			string szContactName, szVector, szTime, szMessage;

			STRTOKEN(szContactName, vParameters, 1, "");
			STRTOKEN(szVector, vParameters, 2, "");
			STRTOKEN(szMessage, vParameters, 3, "");
			STRTOKEN(szTime, vParameters, 4, "");
			
			if (Client.Observer != NULL) {
				Client.Observer->OnContactInvite( szContactName, szVector, szTime, szMessage );
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_VECTOR") == 0)
		{
			string szVector, szCanCreate;
			STRTOKEN(szVector, vParameters, 1, "");
			STRTOKEN(szCanCreate, vParameters, 2, "");

			bool isInPrivate = szVector.find(Client.Username) > 0;

			// Check to see if we've left our private room, and happen to still be hosting
			if (Client.Flags.m_bHosting) {
				if (!isInPrivate) {
					// set our status to looking to join games as opposed to hosting them
					KaiSetHostingStatus(FALSE);
				}
			}
			else {
				if (isInPrivate) {
					// set our status to looking to host games as opposed to joining them
					KaiSetHostingStatus(TRUE);
				}
			}
			Client.Vector = szVector;

			// If the vector is not blank, make a call to enumerate subvectors
			if(strcmp(szVector.c_str(), "") != 0)
				KaiGetSubVectors(Client.Vector);

			if(Client.Flags.m_bChangeChatChannel && Client.Flags.m_bChatEnabled) {
				KaiChangeChatmode(szVector, true);
				Client.Flags.m_bChangeChatChannel = FALSE;
			}

			if (Client.Observer != NULL) {
				Client.Observer->OnEnterArena(szVector, atoi(szCanCreate.c_str()));
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_SUB_VECTOR") == 0)
		{
			string szVector, szPlayers, szSubVectors, szIsPrivate, szPlayerLimit;

			STRTOKEN(szVector, vParameters, 1, "");
			STRTOKEN(szPlayers, vParameters, 2, "");
			STRTOKEN(szSubVectors, vParameters, 3, "");
			STRTOKEN(szIsPrivate, vParameters, 4, "");
			STRTOKEN(szPlayerLimit, vParameters, 5, "");
			string szDescription = "Public Arena";

			if (Client.Observer != NULL) {
				Client.Observer->OnNewArena( szVector,
									  szDescription,
									  atoi(szPlayers.c_str()),
									  atoi(szPlayerLimit.c_str()),
									  atoi(szIsPrivate.c_str()),
									  atoi(szSubVectors.c_str()),
									  false );
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_ARENA_STATUS") == 0)
		{
			string szMode;
			STRTOKEN(szMode, vParameters, 1, "0");

			Client.Flags.m_bHosting = atoi(szMode.c_str()) > 1;
			
			//DebugMsg("KaiClient", Client.Flags.m_bHosting ? "KAICLIENT: User intends to host games." : "KAICLIENT: User intends to join games.");
			
			//DebugMsg("DEBUG", "CRASH A");
			if (Client.Observer != NULL) {
				Client.Observer->OnUpdateHostingStatus(Client.Flags.m_bHosting);
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_STATUS") == 0)
		{
			string status;
			STRTOKEN(status, vParameters, 1, "");

			//Client is online check state again
			if(!Client.Flags.m_bClientLoggedIn)
			{
				if(strcmp(status.c_str(), "XLink Kai is Online..") == 0)
				{
					KaiQuery();
				}
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_USER_SUB_VECTOR") == 0)
		{
			string szVector, szPlayers, szSubVectors, szIsPrivate, szPlayerLimit, szDescription;
			
			STRTOKEN(szVector, vParameters, 1, "");
			STRTOKEN(szPlayers, vParameters, 2, "");
			STRTOKEN(szSubVectors, vParameters, 3, "");
			STRTOKEN(szIsPrivate, vParameters, 4, "");
			STRTOKEN(szPlayerLimit, vParameters, 5, "");
			STRTOKEN(szDescription, vParameters, 6, "");
			
			if (Client.Observer != NULL) {
				Client.Observer->OnNewArena( szVector,
									  szDescription,
									  atoi(szPlayers.c_str()),
									  atoi(szPlayerLimit.c_str()),
									  atoi(szIsPrivate.c_str()),
									  atoi(szSubVectors.c_str()),
									  true );
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_SUB_VECTOR_UPDATE") == 0)
		{
			string szVector, szPlayers, szSubVectors;
			STRTOKEN(szVector, vParameters, 1, "");
			STRTOKEN(szPlayers, vParameters, 2, "");	
			STRTOKEN(szSubVectors, vParameters, 3, "");

			if (Client.Observer != NULL) {
				Client.Observer->OnUpdateArena( szVector,
										 atoi(szPlayers.c_str()),
										 atoi(szSubVectors.c_str()));
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_VECTOR_DISALLOWED") == 0)
		{
			string szVector, szReason;

			STRTOKEN(szVector, vParameters, 1, "");
			STRTOKEN(szReason, vParameters, 2, "");

			if (Client.Observer != NULL) {
				Client.Observer->OnEnterArenaFailed( szVector, szReason );
			}
		}

		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_JOINS_VECTOR") == 0)
		{
			string szContactName;
			STRTOKEN(szContactName, vParameters, 1, "");

			if (Client.Observer != NULL) {		
				Client.Observer->OnOpponentEnter( szContactName );
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_ARENA_PING") == 0)
		{
			string szContactName, szPing, szStatus, szPlayers, szCaps;

			STRTOKEN(szContactName, vParameters, 1, "");
			STRTOKEN(szPing, vParameters, 2, "");
			STRTOKEN(szStatus, vParameters, 3, "");
			STRTOKEN(szPlayers, vParameters, 4, "");
			STRTOKEN(szCaps, vParameters, 5, "");
			if (Client.Observer != NULL) {
				Client.Observer->OnOpponentPing(szContactName, strtoul(szPing.c_str(), NULL, 10), 
					atoi(szStatus.c_str()), szCaps);
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_USER_PROFILE") == 0)
		{
			string szOpponent, szAge, szBandwidth, szLocation, szXBOX, szGCN, szPS2, szBio;

			STRTOKEN(szOpponent, vParameters, 1, "");
			STRTOKEN(szAge, vParameters, 2, "");
			STRTOKEN(szBandwidth, vParameters, 3, "");
			STRTOKEN(szLocation, vParameters, 4, "");
			STRTOKEN(szXBOX, vParameters, 5, "");
			STRTOKEN(szGCN, vParameters, 6, "");
			STRTOKEN(szPS2, vParameters, 7, "");
			STRTOKEN(szBio, vParameters, 8, "");
		
			if (Client.Observer != NULL) {
				Client.Observer->OnUpdateOpponent(szOpponent, szAge, szBandwidth, szLocation, szBio );
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_AVATAR") == 0)
		{

			string szOpponent, szUrl;
			
			STRTOKEN(szOpponent, vParameters, 1, "");
			STRTOKEN(szUrl, vParameters, 2, "");

			if (szUrl.length() > 0) {
				if (Client.Observer != NULL) {
					Client.Observer->OnUpdateOpponent(szOpponent, szUrl);
				}
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_LEAVES_VECTOR") == 0)
		{
			string szContactName;
			STRTOKEN(szContactName, vParameters, 1, "");
			if (Client.Observer != NULL) {
				Client.Observer->OnOpponentLeave( szContactName );
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_APP_SPECIFIC") == 0)
		{
//			if (Client.Observer != NULL)
//			{
//				string szQuestion, szAnswer;
//				STRTOKEN(szQuestion, vParameters, 1, "");
//				STRTOKEN(szAnswer, vParameters, 2, "");
//
//				// Not yet implemented - future use
//			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_SPECIFIC_COUNT") == 0)
		{
			string szVector, szPlayers, szSubVectors;

			STRTOKEN(szVector, vParameters, 1, "");
			STRTOKEN(szPlayers, vParameters, 2, "");
			STRTOKEN(szSubVectors, vParameters, 2, "");
			if (Client.Observer != NULL) {
				Client.Observer->OnUpdateArena(szVector, atoi(szPlayers.c_str()), atoi(szSubVectors.c_str()));
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_JOINS_CHAT") == 0)
		{
			string szVector, szOpponent; 

			STRTOKEN(szVector, vParameters, 1, "");
			STRTOKEN(szOpponent, vParameters, 2, "");

			if( strcmp(szVector.c_str(), Client.Vector.c_str()) == 0 ||
				strcmp(szVector.c_str(), "General Chat") == 0 ) {
					// This guy is definitely joining our chat
					if(Client.Observer != NULL)
						Client.Observer->OnJoinsChat(szOpponent);

					//DebugMsg("CRASH", "CRASH HERE");
			}

			
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_CHAT") == 0)
		{

			string szVector, szOpponent, szMessage;
			STRTOKEN(szVector, vParameters, 1, "");
			STRTOKEN(szOpponent, vParameters, 2, "");
			STRTOKEN(szMessage, vParameters, 3, "");

			if (Client.Observer != NULL) {
				Client.Observer->OnChat(szVector, szOpponent, szMessage, false);
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_ARENA_PM") == 0)
		{
			string szOpponent, szMessage;

			STRTOKEN(szOpponent, vParameters, 1, "");
			STRTOKEN(szMessage, vParameters, 2, "");
		
			if (Client.Observer != NULL) {
				Client.Observer->OnChatPrivateMessage(szOpponent, szMessage, Client.Vector, false);
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_PM") == 0)
		{
			string szOpponent, szMessage;

			STRTOKEN(szOpponent, vParameters, 1, "");
			STRTOKEN(szMessage, vParameters, 2, "");

			if (Client.Observer != NULL) {
				Client.Observer->OnChatPrivateMessage(szOpponent, szMessage, "Home", true);
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_LEAVES_CHAT") == 0)
		{
			string szVector, szOpponent;

			STRTOKEN(szVector, vParameters, 1, "");
			STRTOKEN(szOpponent, vParameters, 2, "");

			if( strcmp(szVector.c_str(), Client.Vector.c_str()) == 0 ||
				strcmp(szVector.c_str(), "General Chat") == 0 ) {
			{
				if (Client.Observer != NULL) {			
					Client.Observer->OnLeavesChat(szOpponent);
				}
			}
			
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_METRICS") == 0)
		{
				string szServer, szReachable;

				STRTOKEN(szServer, vParameters, 1, "");
				STRTOKEN(szReachable, vParameters, 2, "");

				if (szReachable.find("Yes") >= 0)
				{
					Client.Flags.m_bReachable = TRUE;
					int nameIsLong = szServer.find('(') - 1;

					string szName = nameIsLong > 0 ? szServer.substr(0,nameIsLong) : szServer;
					if (Client.Observer != NULL) {
						Client.Observer->OnNetworkReachable(szName);
					}
				}
			}
		}
		else if (strcmp(szMessage.c_str(), "KAI_CLIENT_DETACH") == 0)
		{
			Client.State = Disconnected;
			if (Client.Observer != NULL) {
				Client.Observer->OnEngineDetached();
			}
		}
		break;
	}

	// do infrequent work
	if (Client.State == Authenticated)
	{
		if (!Client.Flags.m_bContactsSettling)
		{
			BOOL bHeadset = FALSE;
			#ifdef HAS_KAI_VOICE
			bHeadset = CVoiceManager::IsHeadsetConnected();
			#endif
			
			if (bHeadset != Client.Flags.m_bHeadset)
			{
				KaiSetBearerCaps(bHeadset);
				Client.Flags.m_bHeadset = bHeadset;
			}
			else
			{
				string szRequest = "";
				if (m_pRequestList->GetNext(szRequest))
				{
					Send(Server.SocketAddress, szRequest);
				}
			}
		}
	}
}

// do frequent work
void CKaiClient::DoWork()
{
	// generate our own timed events
	if (Client.Observer != NULL && Client.State == Authenticated)
	{
		double dwCurrentTime = Timer.GetAbsoluteTime();
		if (Client.Flags.m_bContactsSettling && (dwCurrentTime - Client.Timers.m_dwSettlingTimer > KAI_CONTACT_SETTLING_PERIOD) )
		{
			Client.Flags.m_bContactsSettling = FALSE;
			Client.Observer->OnContactsOnline(Client.FriendsOnline);

			#ifdef HAS_KAI_VOICE
				m_bHeadset = CVoiceManager::IsHeadsetConnected();
				SetBearerCaps(m_bHeadset);
			#endif
		}

		if (!Client.Flags.m_bReachable && (dwCurrentTime - Client.Timers.m_dwReachableTimer > KAI_REACHABLE_QUERY_PERIOD) )
		{
			Client.Timers.m_dwReachableTimer = dwCurrentTime;
			KaiQueryClientMetrics();
		}
	}

	#ifdef HAS_KAI_VOICE
		if (m_pDSound)
		{
			g_VoiceManager.ProcessVoice();

			// Make sure we send voice data at an appropriate rate
			if ( m_VoiceTimer.GetElapsedSeconds() > VOICE_SEND_INTERVAL )
			{
				SendVoiceDataToEngine();
			}
		}
	#endif
}

#ifdef HAS_KAI_VOICE
// Called whenever voice data is produced by the voice system
void CKaiClient::VoiceDataCallback( DWORD dwPort, DWORD dwSize, VOID* pvData, VOID* pContext )
{
	CKaiClient* pThis = (CKaiClient*)pContext;
	pThis->OnVoiceData( dwPort, dwSize, pvData );
}
#endif

void CKaiClient::KaiOnVoiceData( DWORD dwControllerPort, DWORD dwSize, VOID* pvData )
{
	#ifdef HAS_KAI_VOICE
		if (m_pDSound)
		{
			m_pEgress->Write((LPBYTE)pvData);

			// We've set up our voice timer such that it SHOULD cause us to send out
			// our buffered voice data before the buffer fills up.  However, things
			// like framerate glitches, etc., could cause us to fill up before we
			// notice the timer has fired.
			if ( m_pEgress->Size() == MAX_VOICE_PER_SEND )
			{
				SendVoiceDataToEngine();
			}
		}
	#endif
}



#ifdef HAS_KAI_VOICE
// Called whenever a voice communicator event occurs e.g. insertions/removals
void CKaiClient::CommunicatorCallback( DWORD dwPort, VOICE_COMMUNICATOR_EVENT event, VOID* pContext )
{
	CKaiClient* pThis = (CKaiClient*)pContext;
	pThis->OnCommunicatorEvent( dwPort, event );
}

void CKaiClient::OnCommunicatorEvent( DWORD dwControllerPort, VOICE_COMMUNICATOR_EVENT event )
{
	switch ( event )
	{
		case VOICE_COMMUNICATOR_INSERTED:
		DebugMsg("KaiClient", "Voice communicator inserted.");
		g_VoiceManager.SetLoopback( dwControllerPort, FALSE );
		SetBearerCaps(TRUE);
		break;

		case VOICE_COMMUNICATOR_REMOVED:
		DebugMsg("KaiClient", "Voice communicator removed.");
		SetBearerCaps(FALSE);
		break;
	}
}
#endif
