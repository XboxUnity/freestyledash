#pragma once
#include "../../Threads/XeThread.h"
#include "../../XlinkKai/General/kaiManager.h"
#include "../KaiPlayer.h"

// Macros for quickly unlocking and locking arena mode class, if necessary
#define CHAT_LOCK		EnterCriticalSection(&m_ChatLock)
#define CHAT_UNLOCK		LeaveCriticalSection(&m_ChatLock)

typedef vector<CKaiPlayer> ChatPlayer;

class ChatManager : public iBuddyObserver
{
public:
	static ChatManager& getInstance()
	{
		static ChatManager singleton;
		return singleton;
	}

	// IBuddyObsverer Implementation
	void OnJoinsChat(string aOpponent);
	void OnLeavesChat(string aOpponent);
	void OnChat(string aVector, string aOpponent, string aMessage, bool bPrivate);	
	
	// Accessor information
	HRESULT getChatPlayerInfo( ChatPlayer * m_pvChatInfo );
	int getChatPlayerCount( void );
	void StartChatChannel( void );
	void StopChatChannel( void );
	void SendChatMessage( wstring aMessage );

	wstring& getChatBuffer( void );
	void clearChatBuffer( void );

private:

	ChatManager();
	~ChatManager();
	ChatManager(const ChatManager&);		            // Prevent copy-construction
	ChatManager& operator=(const ChatManager&);		    // Prevent assignment

private:
	CRITICAL_SECTION m_ChatLock;
	ChatPlayer m_vChatPlayers;

	wstring m_szChatBuffer;
	bool bChatActive;
};