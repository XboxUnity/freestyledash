#include "../../XlinkKai/General/KaiManager.h"
#include "../KaiPlayer.h"

// Macros and Defintions
#define BUDDY_LOCK		EnterCriticalSection(&m_BuddyLock)
#define BUDDY_UNLOCK	LeaveCriticalSection(&m_BuddyLock)


class CKaiBuddyManager : public iBuddyObserver
{
public:
	static CKaiBuddyManager& getInstance()
	{
		static CKaiBuddyManager singleton;
		return singleton;
	}

	// IBuddyObsverer Implementation
	void OnContactOffline(string aFriend);
	void OnContactOnline(string aFriend);
	void OnContactPing(string aFriend, string aVector, DWORD aPing, int aStatus, string aBearerCapability);
	void OnContactAdd(string aFriend);
	void OnContactRemove(string aFriend);
	void OnUpdateOpponent(string aOpponent, string aAge, string aBandwidth, string aLocation, string aBio);
	void OnUpdateOpponent(string aOpponent, string aAvatarURL);
	void OnOpponentEnter(string aOpponent);
	void OnOpponentLeave(string aOpponent);
	void OnJoinsChat(string aOpponent);
	void OnLeavesChat(string aOpponent);
	void OnChatPrivateMessage(string aPlayer, string aMessage, string aVector, bool bContactList);

public:
	// Public Functions to access our buddy list
	int getBuddyListCount();
	HRESULT getBuddyInfo( std::map<string, CKaiPlayer>* pBuddyInfo );
	HRESULT getBuddyLookupInfo( std::vector<string> * pBuddyLookupInfo );
	bool isBuddy(string aOpponent);

private:
	CKaiBuddyManager();
	~CKaiBuddyManager();
	CKaiBuddyManager(const CKaiBuddyManager&);		            // Prevent copy-construction
	CKaiBuddyManager& operator=(const CKaiBuddyManager&);		    // Prevent assignment

private:
	std::map<string, CKaiPlayer> m_buddyInfo;
	typedef std::map<string, CKaiPlayer> BuddyItem;
	std::vector<string> m_buddyLookup;

	bool singleAlert;

	CRITICAL_SECTION m_BuddyLock;
};