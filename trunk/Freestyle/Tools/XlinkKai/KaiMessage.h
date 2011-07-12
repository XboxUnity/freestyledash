#pragma once

#define KAI_ON_DEINITIALISE				XM_USER + 5000
#define KAI_ON_ENGINE_DETACHED			XM_USER + 5001
#define KAI_ON_ENGINE_ATTACHED			XM_USER + 5002
#define KAI_ON_ENGINE_INUSE				XM_USER + 5003
#define KAI_ON_ACCOUNT_NOT_LOGGED_IN	XM_USER + 5004
#define KAI_ON_ACCOUNT_LOGGED_IN		XM_USER + 5005
#define KAI_ON_AUTHENTICATION_FAILED	XM_USER + 5006
#define KAI_ON_NETWORK_ERROR			XM_USER + 5007
#define KAI_ON_NETWORK_REACHABLE		XM_USER + 5008
#define KAI_ON_CONTACT_OFFLINE			XM_USER + 5009
#define KAI_ON_CONTACT_ONLINE			XM_USER + 5010
#define KAI_ON_CONTACTS_ONLINE			XM_USER + 5011
#define KAI_ON_CONTACT_PING				XM_USER + 5012
#define KAI_ON_CONTACT_ADD				XM_USER + 5013
#define KAI_ON_CONTACT_REMOVE			XM_USER + 5014
#define KAI_ON_CONTACT_SPEEX_STATUS		XM_USER + 5015
#define KAI_ON_CONTACT_SPEEX_RING		XM_USER + 5016
#define KAI_ON_CONTACT_SPEEX			XM_USER + 5017
#define KAI_ON_CONTACT_INVITE			XM_USER + 5018
#define KAI_ON_ENTER_ARENA				XM_USER + 5019
#define KAI_ON_ENTER_ARENA_FAILED		XM_USER + 5020
#define KAI_ON_NEW_ARENA				XM_USER + 5021
#define KAI_ON_UPDATE_ARENA				XM_USER + 5022
#define KAI_ON_UPDATE_OPPONENT_INFO		XM_USER + 5023
#define KAI_ON_UPDATE_OPPONENT_AVATAR	XM_USER + 5024
#define KAI_ON_UPDATE_HOSTING_STATUS	XM_USER + 5025
#define KAI_ON_OPPONENT_ENTER			XM_USER + 5026
#define KAI_ON_OPPONENT_PING			XM_USER + 5027
#define KAI_ON_OPPONENT_LEAVE			XM_USER + 5028
#define KAI_ON_SUPPORTED_TITLE			XM_USER + 5029
#define KAI_ON_JOINS_CHAT				XM_USER + 5030
#define KAI_ON_CHAT						XM_USER + 5031
#define KAI_ON_LEAVES_CHAT				XM_USER + 5032
#define KAI_ON_CHAT_PRIVATE_MESSAGE		XM_USER + 5033

typedef struct { string unusedData; } KAIMessageOnDeinitialise;
typedef struct { string unusedData; } KAIMessageOnEngineDetached;
typedef struct { string unusedData; } KAIMessageOnEngineAttached;
typedef struct { string unusedData; } KAIMessageOnEngineInUse;
typedef struct { string szUsername; string szPassword; } KAIMessageOnAccountNotLoggedIn;
typedef struct { string unusedData; } KAIMessageOnAccountLoggedIn;
typedef struct { string szUsername; } KAIMessageOnAuthenticationFailed;
typedef struct { string szError; } KAIMessageOnNetworkError;
typedef struct { string szServerName; } KAIMessageOnNetworkReachable;
typedef struct { string szFriend; } KAIMessageOnContactOffline;
typedef struct { string szFriend; } KAIMessageOnContactOnline;
typedef struct { int nContacts; } KAIMessageOnContactsOnline;
typedef struct { string szFriend; string szVector; DWORD dwPing; int nStatus; string szBearerCapability; } KAIMessageOnContactPing;
typedef struct { string szFriend; } KAIMessageOnContactAdd;
typedef struct { string szFriend; } KAIMessageOnContactRemove;
typedef struct { string szFriend; BOOL bSpeexEnabled; } KAIMessageOnContactSpeexStatus;
typedef struct { string szFriend; } KAIMessageOnContactSpeexRing;
typedef struct { string szFriend; } KAIMessageOnContactSpeex;
typedef struct { string szFriend; string szVector; string szTime; string szMessage; } KAIMessageOnContactInvite;
typedef struct { string szVector; BOOL bCanHost; } KAIMessageOnEnterArena;
typedef struct { string szVector; string szReason; } KAIMessageOnEnterArenaFailed;
typedef struct { string szVector; string szDescription; int nPlayers; int nPlayerLimit; int nPassword; int szSubVectors; bool bPersonal; } KAIMessageOnNewArena;
typedef struct { string szVector; int nPlayers; int szSubVectors; } KAIMessageOnUpdateArena;
typedef struct { string szOpponent; string szAge; string szBandwidth; string szLocation; string szBio; } KAIMessageOnUpdateOpponentInfo;
typedef struct { string szOpponent; string szAvatarURL; } KAIMessageOnUpdateOpponentAvatar;
typedef struct { BOOL bIsHosting; } KAIMessageOnUpdateHostingStatus;
typedef struct { string szOpponent; } KAIMessageOnOpponentEnter;
typedef struct { string szOpponent; DWORD dwPing; int nStatus; string szBearerCapability; } KAIMessageOnOpponentPing;
typedef struct { string szOpponent; } KAIMessageOnOpponentLeave;
typedef struct { DWORD dwTitleId; string szVector; } KAIMessageOnSupportedTitle;
typedef struct { string szOpponent; } KAIMessageOnJoinsChat;
typedef struct { string szVector; string szOpponent; string szMessage; BOOL bPrivate; } KAIMessageOnChat;
typedef struct { string szOpponent; } KAIMessageOnLeavesChat;
typedef struct { string szPlayer; string szMessage; string szVector; bool bContactList; } KAIMessageOnChatPrivateMessage;

#define KAI_ON_NOTIFY_DEINITIALISE(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_DEINITIALISE)\
	{\
		KAIMessageOnDeinitialise * pData = (KAIMessageOnDeinitialise *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_ENGINE_DETACHED(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_ENGINE_DETACHED)\
	{\
		KAIMessageOnEngineDetached * pData = (KAIMessageOnEngineDetached *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_ENGINE_ATTACHED(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_ENGINE_ATTACHED)\
	{\
		KAIMessageOnEngineAttached * pData = (KAIMessageOnEngineAttached *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_ENGINE_INUSE(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_ENGINE_INUSE)\
	{\
		KAIMessageOnEngineInUse * pData = (KAIMessageOnEngineInUse *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_ACCOUNT_NOT_LOGGED_IN(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_ACCOUNT_NOT_LOGGED_IN)\
	{\
		KAIMessageOnAccountNotLoggedIn * pData = (KAIMessageOnAccountNotLoggedIn *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_ACCOUNT_LOGGED_IN(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_ACCOUNT_LOGGED_IN)\
	{\
		KAIMessageOnAccountLoggedIn * pData = (KAIMessageOnAccountLoggedIn *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_AUTHENTICATION_FAILED(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_AUTHENTICATION_FAILED)\
	{\
		KAIMessageOnAuthenticationFailed *pData = (KAIMessageOnAuthenticationFailed *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_NETWORK_ERROR(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_NETWORK_ERROR)\
	{\
		KAIMessageOnNetworkError *pData = (KAIMessageOnNetworkError *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_NETWORK_REACHABLE(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_NETWORK_REACHABLE)\
	{\
		KAIMessageOnNetworkReachable *pData = (KAIMessageOnNetworkReachable *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_CONTACT_OFFLINE(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_CONTACT_OFFLINE)\
	{\
		KAIMessageOnContactOffline *pData = (KAIMessageOnContactOffline *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_CONTACT_ONLINE(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_CONTACT_ONLINE)\
	{\
		KAIMessageOnContactOnline *pData = (KAIMessageOnContactOnline *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_CONTACTS_ONLINE(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_CONTACTS_ONLINE)\
	{\
		KAIMessageOnContactsOnline *pData = (KAIMessageOnContactsOnline *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_CONTACT_PING(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_CONTACT_PING)\
	{\
		KAIMessageOnContactPing *pData = (KAIMessageOnContactPing *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_CONTACT_ADD(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_CONTACT_ADD)\
	{\
		KAIMessageOnContactAdd *pData = (KAIMessageOnContactAdd *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_CONTACT_REMOVE(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_CONTACT_REMOVE)\
	{\
		KAIMessageOnContactRemove *pData = (KAIMessageOnContactRemove *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_CONTACT_SPEEX_STATUS(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_CONTACT_SPEEX_STATUS)\
	{\
		KAIMessageOnContactSpeexStatus *pData = (KAIMessageOnContactSpeexStatus *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_CONTACT_SPEEX_RING(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_CONTACT_SPEEX_RING)\
	{\
		KAIMessageOnContactSpeexRing *pData = (KAIMessageOnContactSpeexRing *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_CONTACT_SPEEX(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_CONTACT_SPEEX)\
	{\
		KAIMessageOnContactSpeex *pData = (KAIMessageOnContactSpeex *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_CONTACT_INVITE(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_CONTACT_INVITE)\
	{\
		KAIMessageOnContactInvite *pData = (KAIMessageOnContactInvite *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_ENTER_ARENA(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_ENTER_ARENA)\
	{\
		KAIMessageOnEnterArena *pData = (KAIMessageOnEnterArena *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_ENTER_ARENA_FAILED(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_ENTER_ARENA_FAILED)\
	{\
		KAIMessageOnEnterArenaFailed *pData = (KAIMessageOnEnterArenaFailed *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_NEW_ARENA(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_NEW_ARENA)\
	{\
		KAIMessageOnNewArena *pData = (KAIMessageOnNewArena *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_UPDATE_ARENA(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_UPDATE_ARENA)\
	{\
		KAIMessageOnUpdateArena *pData = (KAIMessageOnUpdateArena *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_UPDATE_OPPONENT_INFO(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_UPDATE_OPPONENT_INFO)\
	{\
		KAIMessageOnUpdateOpponentInfo *pData = (KAIMessageOnUpdateOpponentInfo *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_UPDATE_OPPONENT_AVATAR(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_UPDATE_OPPONENT_AVATAR)\
	{\
		KAIMessageOnUpdateOpponentAvatar *pData = (KAIMessageOnUpdateOpponentAvatar *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_UPDATE_HOSTING_STATUS(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_UPDATE_HOSTING_STATUS)\
	{\
		KAIMessageOnUpdateHostingStatus *pData = (KAIMessageOnUpdateHostingStatus *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_OPPONENT_ENTER(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_OPPONENT_ENTER)\
	{\
		KAIMessageOnOpponentEnter *pData = (KAIMessageOnOpponentEnter *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_OPPONENT_PING(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_OPPONENT_PING)\
	{\
		KAIMessageOnOpponentPing *pData = (KAIMessageOnOpponentPing *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_OPPONENT_LEAVE(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_OPPONENT_LEAVE)\
	{\
		KAIMessageOnOpponentLeave *pData = (KAIMessageOnOpponentLeave *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_SUPPORTED_TITLE(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_SUPPORTED_TITLE)\
	{\
		KAIMessageOnSupportedTitle *pData = (KAIMessageOnSupportedTitle *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_JOINS_CHAT(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_JOINS_CHAT)\
	{\
		KAIMessageOnJoinsChat *pData = (KAIMessageOnJoinsChat *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_CHAT(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_CHAT)\
	{\
		KAIMessageOnChat *pData = (KAIMessageOnChat *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_LEAVES_CHAT(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_LEAVES_CHAT)\
	{\
		KAIMessageOnLeavesChat *pData = (KAIMessageOnLeavesChat *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define KAI_ON_NOTIFY_CHAT_PRIVATE_MESSAGE(MemberFunc)\
	if( pMessage->dwMessage == KAI_ON_CHAT_PRIVATE_MESSAGE)\
	{\
		KAIMessageOnChatPrivateMessage *pData = (KAIMessageOnChatPrivateMessage *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}


// Message Accesors
static __declspec(noinline) void KaiMessageOnDeinitialise(XUIMessage *pMsg, KAIMessageOnDeinitialise * pData, string szUnused)
{
    XuiMessage(pMsg, KAI_ON_DEINITIALISE);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->unusedData = szUnused;
}

static __declspec(noinline) void KaiMessageOnEngineDetached(XUIMessage *pMsg, KAIMessageOnEngineDetached * pData, string szUnused)
{
    XuiMessage(pMsg, KAI_ON_ENGINE_DETACHED);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->unusedData = szUnused;
}

static __declspec(noinline) void KaiMessageOnEngineAttached(XUIMessage *pMsg, KAIMessageOnEngineAttached * pData, string szUnused)
{
    XuiMessage(pMsg, KAI_ON_ENGINE_ATTACHED);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->unusedData = szUnused;
}

static __declspec(noinline) void KaiMessageOnEngineInUse(XUIMessage *pMsg, KAIMessageOnEngineInUse * pData, string szUnused)
{
    XuiMessage(pMsg, KAI_ON_ENGINE_INUSE);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->unusedData = szUnused;
}

static __declspec(noinline) void KaiMessageOnAccountNotLoggedIn(XUIMessage *pMsg, KAIMessageOnAccountNotLoggedIn * pData, string szUsername, string szPassword)
{
    XuiMessage(pMsg, KAI_ON_ACCOUNT_NOT_LOGGED_IN);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szUsername = szUsername;
	pData->szPassword = szPassword;
}

static __declspec(noinline) void KaiMessageOnAccountLoggedIn(XUIMessage *pMsg, KAIMessageOnAccountLoggedIn * pData, string szUnused)
{
    XuiMessage(pMsg, KAI_ON_ACCOUNT_LOGGED_IN);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->unusedData = szUnused;
}


static __declspec(noinline) void KaiMessageOnAuthenticationFailed(XUIMessage *pMsg, KAIMessageOnAuthenticationFailed * pData, string szUsername)
{
    XuiMessage(pMsg, KAI_ON_AUTHENTICATION_FAILED);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szUsername = szUsername;
}

static __declspec(noinline) void KaiMessageOnNetworkError(XUIMessage *pMsg, KAIMessageOnNetworkError * pData, string szError)
{
    XuiMessage(pMsg, KAI_ON_NETWORK_ERROR);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szError = szError;
}

static __declspec(noinline) void KaiMessageOnNetworkReachable(XUIMessage *pMsg, KAIMessageOnNetworkReachable * pData, string szServerName)
{
    XuiMessage(pMsg, KAI_ON_NETWORK_REACHABLE);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szServerName = szServerName;
}

static __declspec(noinline) void KaiMessageOnContactOffline(XUIMessage *pMsg, KAIMessageOnContactOffline * pData, string szFriend)
{
    XuiMessage(pMsg, KAI_ON_CONTACT_OFFLINE);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szFriend = szFriend;
}

static __declspec(noinline) void KaiMessageOnContactOnline(XUIMessage *pMsg, KAIMessageOnContactOnline * pData, string szFriend)
{
    XuiMessage(pMsg, KAI_ON_CONTACT_ONLINE);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szFriend = szFriend;
}

static __declspec(noinline) void KaiMessageOnContactsOnline(XUIMessage *pMsg, KAIMessageOnContactsOnline * pData, int nContacts)
{
    XuiMessage(pMsg, KAI_ON_CONTACTS_ONLINE);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->nContacts = nContacts;
}

static __declspec(noinline) void KaiMessageOnContactPing(XUIMessage *pMsg, KAIMessageOnContactPing * pData, string szFriend, string szVector, DWORD dwPing, int nStatus, string szBearerCapability )
{
    XuiMessage(pMsg, KAI_ON_CONTACT_PING);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szFriend = szFriend;
	pData->szVector = szVector;
	pData->dwPing = dwPing;
	pData->nStatus = nStatus;
	pData->szBearerCapability = szBearerCapability;
}

static __declspec(noinline) void KaiMessageOnContactAdd(XUIMessage *pMsg, KAIMessageOnContactAdd * pData, string szFriend)
{
    XuiMessage(pMsg, KAI_ON_CONTACT_REMOVE);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szFriend = szFriend;
}


static __declspec(noinline) void KaiMessageOnContactRemove(XUIMessage *pMsg, KAIMessageOnContactRemove * pData, string szFriend)
{
    XuiMessage(pMsg, KAI_ON_CONTACT_REMOVE);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szFriend = szFriend;
}

static __declspec(noinline) void KaiMessageOnContactSpeexStatus(XUIMessage *pMsg, KAIMessageOnContactSpeexStatus * pData, string szFriend, BOOL bSpeexEnabled)
{
    XuiMessage(pMsg, KAI_ON_CONTACT_SPEEX_STATUS);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szFriend = szFriend;
	pData->bSpeexEnabled = bSpeexEnabled;
}

static __declspec(noinline) void KaiMessageOnContactSpeexRing(XUIMessage *pMsg, KAIMessageOnContactSpeexRing * pData, string szFriend)
{
    XuiMessage(pMsg, KAI_ON_CONTACT_SPEEX_RING);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szFriend = szFriend;
}

static __declspec(noinline) void KaiMessageOnContactSpeex(XUIMessage *pMsg, KAIMessageOnContactSpeex * pData, string szFriend)
{
    XuiMessage(pMsg, KAI_ON_CONTACT_SPEEX);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szFriend = szFriend;
}

static __declspec(noinline) void KaiMessageOnContactInvite(XUIMessage *pMsg, KAIMessageOnContactInvite * pData, string szFriend, string szVector, string szTime, string szMessage)
{
    XuiMessage(pMsg, KAI_ON_CONTACT_INVITE);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szFriend = szFriend;
	pData->szVector = szVector;
	pData->szTime = szTime;
	pData->szMessage = szMessage;
}

static __declspec(noinline) void KaiMessageOnEnterArena(XUIMessage *pMsg, KAIMessageOnEnterArena * pData, string szVector, BOOL bCanHost)
{
    XuiMessage(pMsg, KAI_ON_ENTER_ARENA);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szVector = szVector;
	pData->bCanHost = bCanHost;
}

static __declspec(noinline) void KaiMessageOnEnterArenaFailed(XUIMessage *pMsg, KAIMessageOnEnterArenaFailed * pData, string szVector, string szReason)
{
    XuiMessage(pMsg, KAI_ON_ENTER_ARENA_FAILED);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szVector = szVector;
	pData->szReason = szReason;
}

static __declspec(noinline) void KaiMessageOnNewArena(XUIMessage *pMsg, KAIMessageOnNewArena * pData, string szVector, string szDescription, int nPlayers, int nPlayerLimit, int nPassword, int szSubVectors, bool bPersonal)
{
    XuiMessage(pMsg, KAI_ON_NEW_ARENA);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szVector = szVector;
	pData->szDescription = szDescription;
	pData->nPlayers = nPlayers;
	pData->nPlayerLimit = nPlayerLimit;
	pData->nPassword = nPassword;
	pData->bPersonal = bPersonal;
	pData->szSubVectors = szSubVectors;
}

static __declspec(noinline) void KaiMessageOnUpdateArena(XUIMessage *pMsg, KAIMessageOnUpdateArena * pData, string szVector, int nPlayers, int szSubVectors)
{
    XuiMessage(pMsg, KAI_ON_UPDATE_ARENA);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szVector = szVector;
	pData->nPlayers = nPlayers;
	pData->szSubVectors = szSubVectors;
}

static __declspec(noinline) void KaiMessageOnUpdateOpponentInfo(XUIMessage *pMsg, KAIMessageOnUpdateOpponentInfo * pData, string szOpponent, string szAge, string szBandwidth, string szLocation, string szBio)
{
    XuiMessage(pMsg, KAI_ON_UPDATE_OPPONENT_INFO);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szOpponent = szOpponent;
	pData->szAge = szAge;
	pData->szBandwidth = szBandwidth;
	pData->szLocation = szLocation;
	pData->szBio = szBio;
}

static __declspec(noinline) void KaiMessageOnUpdateOpponentAvatar(XUIMessage *pMsg, KAIMessageOnUpdateOpponentAvatar * pData, string szOpponent, string szAvatarURL)
{
    XuiMessage(pMsg, KAI_ON_UPDATE_OPPONENT_AVATAR);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szOpponent = szOpponent;
	pData->szAvatarURL = szAvatarURL;
}

static __declspec(noinline) void KaiMessageOnUpdateHostingStatus(XUIMessage *pMsg, KAIMessageOnUpdateHostingStatus * pData, BOOL bIsHosting)
{
    XuiMessage(pMsg, KAI_ON_UPDATE_HOSTING_STATUS);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->bIsHosting = bIsHosting;
}

static __declspec(noinline) void KaiMessageOnOpponentEnter(XUIMessage *pMsg, KAIMessageOnOpponentEnter * pData, string szOpponent)
{
    XuiMessage(pMsg, KAI_ON_OPPONENT_ENTER);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szOpponent = szOpponent;
}

static __declspec(noinline) void KaiMessageOnOpponentPing(XUIMessage *pMsg, KAIMessageOnOpponentPing * pData, string szOpponent, DWORD dwPing, int nStatus, string szBearerCapability)
{
    XuiMessage(pMsg, KAI_ON_OPPONENT_PING);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szOpponent = szOpponent;
	pData->dwPing = dwPing;
	pData->nStatus = nStatus;
	pData->szBearerCapability = szBearerCapability;
}

static __declspec(noinline) void KaiMessageOnOpponentLeave(XUIMessage *pMsg, KAIMessageOnOpponentLeave * pData, string szOpponent)
{
    XuiMessage(pMsg, KAI_ON_OPPONENT_LEAVE);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szOpponent = szOpponent;
}

static __declspec(noinline) void KaiMessageOnSupportedTitle(XUIMessage *pMsg, KAIMessageOnSupportedTitle * pData, DWORD dwTitleId, string szVector)
{
    XuiMessage(pMsg, KAI_ON_SUPPORTED_TITLE);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->dwTitleId = dwTitleId;
	pData->szVector = szVector;
}

static __declspec(noinline) void KaiMessageOnJoinsChat(XUIMessage *pMsg, KAIMessageOnJoinsChat * pData, string szOpponent)
{
    XuiMessage(pMsg, KAI_ON_JOINS_CHAT);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szOpponent = szOpponent;
}

static __declspec(noinline) void KaiMessageOnChat(XUIMessage *pMsg, KAIMessageOnChat * pData, string szVector, string szOpponent, string szMessage, BOOL bPrivate)
{
    XuiMessage(pMsg, KAI_ON_CHAT);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szVector = szVector;
	pData->szOpponent = szOpponent;
	pData->szMessage = szMessage;
	pData->bPrivate = bPrivate;
}

static __declspec(noinline) void KaiMessageOnLeavesChat(XUIMessage *pMsg, KAIMessageOnLeavesChat * pData, string szOpponent)
{
    XuiMessage(pMsg, KAI_ON_LEAVES_CHAT);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szOpponent = szOpponent;
}

static __declspec(noinline) void KaiMessageOnChatPrivateMessage(XUIMessage *pMsg, KAIMessageOnChatPrivateMessage * pData, string szPlayer, string szMessage, string szVector, bool bContactList)
{
    XuiMessage(pMsg, KAI_ON_CHAT_PRIVATE_MESSAGE);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->szPlayer = szPlayer;
	pData->szMessage = szMessage;
	pData->szVector = szVector;
	pData->bContactList = bContactList;
}

// End User Commands
static __declspec(noinline) HRESULT KaiNotifyDeinitialise(HXUIOBJ hObj, string szUnused, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnDeinitialise pKaiMsg;
	KaiMessageOnDeinitialise(&pMsg, &pKaiMsg, szUnused);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyEngineDetached(HXUIOBJ hObj, string szUnused, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnEngineDetached pKaiMsg;
	KaiMessageOnEngineDetached(&pMsg, &pKaiMsg, szUnused);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyEngineAttached(HXUIOBJ hObj, string szUnused, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnEngineAttached pKaiMsg;
	KaiMessageOnEngineAttached(&pMsg, &pKaiMsg, szUnused);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyEngineInUse(HXUIOBJ hObj, string szUnused, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnEngineInUse pKaiMsg;
	KaiMessageOnEngineInUse(&pMsg, &pKaiMsg, szUnused);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyAccountNotLoggedIn(HXUIOBJ hObj, string szUsername, string szPassword, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnAccountNotLoggedIn pKaiMsg;
	KaiMessageOnAccountNotLoggedIn(&pMsg, &pKaiMsg, szUsername, szPassword);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyAccountLoggedIn(HXUIOBJ hObj, string szUnused, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnAccountLoggedIn pKaiMsg;
	KaiMessageOnAccountLoggedIn(&pMsg, &pKaiMsg, szUnused);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyAuthenticationFailed(HXUIOBJ hObj, string szUsername, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnAuthenticationFailed pKaiMsg;
	KaiMessageOnAuthenticationFailed(&pMsg, &pKaiMsg, szUsername);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyNetworkError(HXUIOBJ hObj, string szError, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnNetworkError pKaiMsg;
	KaiMessageOnNetworkError(&pMsg, &pKaiMsg, szError);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyNetworkReachable(HXUIOBJ hObj, string szServerName, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnNetworkReachable pKaiMsg;
	KaiMessageOnNetworkReachable(&pMsg, &pKaiMsg, szServerName);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyContactOffline(HXUIOBJ hObj, string szFriend, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnContactOffline pKaiMsg;
	KaiMessageOnContactOffline(&pMsg, &pKaiMsg, szFriend);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyContactOnline(HXUIOBJ hObj, string szFriend, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnContactOnline pKaiMsg;
	KaiMessageOnContactOnline(&pMsg, &pKaiMsg, szFriend);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyContactsOnline(HXUIOBJ hObj, int nContacts, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnContactsOnline pKaiMsg;
	KaiMessageOnContactsOnline(&pMsg, &pKaiMsg, nContacts);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyContactPing(HXUIOBJ hObj, string szFriend, string szVector, DWORD dwPing, int nStatus, string szBearerCapability, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnContactPing pKaiMsg;
	KaiMessageOnContactPing(&pMsg, &pKaiMsg, szFriend, szVector, dwPing, nStatus, szBearerCapability);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyContactAdd(HXUIOBJ hObj, string szFriend, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnContactAdd pKaiMsg;
	KaiMessageOnContactAdd(&pMsg, &pKaiMsg, szFriend);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}


static __declspec(noinline) HRESULT KaiNotifyContactRemove(HXUIOBJ hObj, string szFriend, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnContactRemove pKaiMsg;
	KaiMessageOnContactRemove(&pMsg, &pKaiMsg, szFriend);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyContactSpeexStatus(HXUIOBJ hObj, string szFriend, BOOL bSpeexEnabled, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnContactSpeexStatus pKaiMsg;
	KaiMessageOnContactSpeexStatus(&pMsg, &pKaiMsg, szFriend, bSpeexEnabled);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyContactSpeexRing(HXUIOBJ hObj, string szFriend, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnContactSpeexRing pKaiMsg;
	KaiMessageOnContactSpeexRing(&pMsg, &pKaiMsg, szFriend);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyContactSpeex(HXUIOBJ hObj, string szFriend, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnContactSpeex pKaiMsg;
	KaiMessageOnContactSpeex(&pMsg, &pKaiMsg, szFriend);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyContactInvite(HXUIOBJ hObj, string szFriend, string szVector, string szTime, string szMessage, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnContactInvite pKaiMsg;
	KaiMessageOnContactInvite(&pMsg, &pKaiMsg, szFriend, szVector, szTime, szMessage);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyEnterArena(HXUIOBJ hObj, string szVector, BOOL bCanHost, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnEnterArena pKaiMsg;
	KaiMessageOnEnterArena(&pMsg, &pKaiMsg, szVector, bCanHost);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyEnterArenaFailed(HXUIOBJ hObj, string szVector, string szReason, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnEnterArenaFailed pKaiMsg;
	KaiMessageOnEnterArenaFailed(&pMsg, &pKaiMsg, szVector, szReason);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyNewArena(HXUIOBJ hObj, string szVector, string szDescription, int nPlayers, int nPlayerLimit, int nPassword, int aSubVectors, bool bPersonal, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnNewArena pKaiMsg;
	KaiMessageOnNewArena(&pMsg, &pKaiMsg, szVector, szDescription, nPlayers, nPlayerLimit, nPassword, aSubVectors, bPersonal);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyUpdateArena(HXUIOBJ hObj, string szVector, int nPlayers, int aSubVectors, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnUpdateArena pKaiMsg;
	KaiMessageOnUpdateArena(&pMsg, &pKaiMsg, szVector, nPlayers, aSubVectors);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyUpdateOpponentInfo(HXUIOBJ hObj, string szOpponent, string szAge, string szBandwidth, string szLocation, string szBio, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnUpdateOpponentInfo pKaiMsg;
	KaiMessageOnUpdateOpponentInfo(&pMsg, &pKaiMsg, szOpponent, szAge, szBandwidth, szLocation, szBio);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyUpdateOpponentAvatar(HXUIOBJ hObj, string szOpponent, string szAvatarURL, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnUpdateOpponentAvatar pKaiMsg;
	KaiMessageOnUpdateOpponentAvatar(&pMsg, &pKaiMsg, szOpponent, szAvatarURL);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyUpdateHostingStatus(HXUIOBJ hObj, BOOL bIsHosting, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnUpdateHostingStatus pKaiMsg;
	KaiMessageOnUpdateHostingStatus(&pMsg, &pKaiMsg, bIsHosting);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyOpponentEnter(HXUIOBJ hObj, string szOpponent, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnOpponentEnter pKaiMsg;
	KaiMessageOnOpponentEnter(&pMsg, &pKaiMsg, szOpponent);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyOpponentPing(HXUIOBJ hObj, string szOpponent, DWORD dwPing, int nStatus, string szBearerCapability, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnOpponentPing pKaiMsg;
	KaiMessageOnOpponentPing(&pMsg, &pKaiMsg, szOpponent, dwPing, nStatus, szBearerCapability);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyOpponentLeave(HXUIOBJ hObj, string szOpponent, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnOpponentLeave pKaiMsg;
	KaiMessageOnOpponentLeave(&pMsg, &pKaiMsg, szOpponent);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifySupportedTitle(HXUIOBJ hObj, DWORD dwTitleId, string szVector, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnSupportedTitle pKaiMsg;
	KaiMessageOnSupportedTitle(&pMsg, &pKaiMsg, dwTitleId, szVector);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyJoinsChat(HXUIOBJ hObj, string szOpponent, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnJoinsChat pKaiMsg;
	KaiMessageOnJoinsChat(&pMsg, &pKaiMsg, szOpponent);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyChat(HXUIOBJ hObj, string szVector, string szOpponent, string szMessage, BOOL bPrivate, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnChat pKaiMsg;
	KaiMessageOnChat(&pMsg, &pKaiMsg, szVector, szOpponent, szMessage, bPrivate);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyLeavesChat(HXUIOBJ hObj, string szOpponent, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnLeavesChat pKaiMsg;
	KaiMessageOnLeavesChat(&pMsg, &pKaiMsg, szOpponent);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT KaiNotifyChatPrivateMessage(HXUIOBJ hObj, string szPlayer, string szMessage, string szVector, bool bContactList, bool broadcastMsg = false)
{
	XUIMessage pMsg;
	KAIMessageOnChatPrivateMessage pKaiMsg;
	KaiMessageOnChatPrivateMessage(&pMsg, &pKaiMsg, szPlayer, szMessage, szVector, bContactList);
	if( !XuiHandleIsValid( hObj ) ) return S_FALSE;
	if(broadcastMsg)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}