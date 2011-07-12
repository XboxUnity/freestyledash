#pragma once
#include "../../Threads/XeThread.h"
#include "../../XlinkKai/General/KaiManager.h"
#include "../../XlinkKai/KaiPlayer.h"
#include "../../HTTP/HTTPDownloader.h"
#include "../../HTTP/Base/MemoryBuffer.h"

// Macros for quickly unlocking and locking arena mode class, if necessary
#define ARENA_LOCK		EnterCriticalSection(&m_ArenaLock)
#define ARENA_UNLOCK	LeaveCriticalSection(&m_ArenaLock)

// This structure holds all the information about any sub arena items
typedef struct _SUB_ARENAITEM {
	string szArenaVector;
	wstring szArenaVectorW;
	string szArenaName;
	wstring szArenaNameW;
	string szDescription;
	wstring szDescriptionW;
	wstring szDisplayPlayers;
	wstring szDisplayPrivateArena;
	wstring szDisplayPlayerLimit;
	string szArenaAvatarPath;
	wstring szArenaAvatarPathW;
	int	nPrivateArenas;
	int nPlayers;
	int nPlayerLimit;
	bool bHasPassword;
	bool bIsPersonal;
	bool bIsLoaded;
} SUB_ARENAITEM;

// This structure holds all the information about any main level arena items
typedef struct _ARENAITEM {
	string szArenaVector;
	wstring szArenaVectorW;
	string szArenaName;
	wstring szArenaNameW;
	bool bHostingAllowed;
	int nSelectedPlayer;
	vector<SUB_ARENAITEM> vSubArenas;
	vector<CKaiPlayer> vArenaPlayers;
} ARENAITEM;

class ArenaManager : public iBuddyObserver, public iHttpItemRequester
{
public:
	static ArenaManager& getInstance()
	{
		static ArenaManager singleton;
		return singleton;
	}

	void DownloadCompleted(HttpItem* itm);

	// IBuddyObsverer Implementation
	void OnEnterArena(string aVector, BOOL bCanHost);
	void OnNewArena( string aVector, string aDescription, int nPlayers, int nPlayerLimit, int nPassword, int aSubVectors, bool bPersonal );
	void OnUpdateArena( string aVector, int nPlayers, int aSubVectors );
	void OnOpponentEnter(string aOpponent);
	void OnOpponentPing(string aOpponent, DWORD aPing, int aStatus, string aBearerCapability);
	void OnOpponentLeave(string aOpponent);
	void OnChat(string aVector, string aOpponent, string aMessage, bool bPrivate);

	HRESULT getArenaInfo(ARENAITEM *m_pArenaInfo);
	HRESULT setSelectedPlayer(int player);
	int getSubArenaSize();
	int getPlayerSize();
	HRESULT DownloadArenaAvatar(string aVector);
	HRESULT DownloadPlayerAvatar(string aPlayerURL, string aPlayerName, bool isBuddy);
	string FormatURL(string aVector);
	string getArenaAvatarSavePath(string aVector);
	string getPlayerAvatarSavePath(string aPlayerName, bool isBuddy);
	string FileSavePath(string aVector);
	void ClearArena();

	wstring& getChatBuffer(void);
	void clearChatBuffer(void);
	void StartChatChannel(void);
	void StopChatChannel(void);
	void sendChatMessage(wstring aMessage);

private:

	ArenaManager();
	~ArenaManager();
	ArenaManager(const ArenaManager&);		            // Prevent copy-construction
	ArenaManager& operator=(const ArenaManager&);		    // Prevent assignment

private:
	CRITICAL_SECTION m_ArenaLock;
	ARENAITEM m_arenaInfo;

	wstring m_szChatBuffer;
	bool m_bChatActive;

};