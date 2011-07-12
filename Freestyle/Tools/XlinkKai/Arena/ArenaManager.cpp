#include "stdafx.h"

#include "ArenaManager.h"
#include "../../Settings/Settings.h"
#include "../../HTTP/HTTPDownloader.h"
#include "../../HTTP/Base/MemoryBuffer.h"
#include "../../GameContent/QueueThreads/FileTexQueue.h"

ArenaManager::ArenaManager() {

	m_bChatActive = false;
	m_szChatBuffer = L"";

	// First thing we want to do is to register to the kai manager, so we receive kai messages
	KaiManager::getInstance().RegisterClass(*this);

	// Now we'll initialize a lot of our variables
	m_arenaInfo.szArenaVector = "";
	m_arenaInfo.szArenaVectorW = L"";
	m_arenaInfo.szArenaName = "";
	m_arenaInfo.szArenaNameW = L"";
	m_arenaInfo.bHostingAllowed = false;
	m_arenaInfo.nSelectedPlayer = -1;
	m_arenaInfo.vArenaPlayers.clear();
	m_arenaInfo.vSubArenas.clear();

	// Create our critical section used to protect our entire class
	InitializeCriticalSection(&m_ArenaLock);
}

ArenaManager::~ArenaManager() {
	// Here let's clear our information
	ClearArena();
	
	// Delete our critical section from memory
	DeleteCriticalSection(&m_ArenaLock);	
}

// Update our manager's reference to the currently selected player
HRESULT ArenaManager::setSelectedPlayer( int nPlayerIndex ) {
	m_arenaInfo.nSelectedPlayer = nPlayerIndex;
	return S_OK;
}

// Retrieve the number of players in the current arena
int ArenaManager::getPlayerSize( void ) {
	return (int)m_arenaInfo.vArenaPlayers.size();
}

// Retrive the number of sub arenas in the current arena
int ArenaManager::getSubArenaSize( void ) {
	return (int)m_arenaInfo.vSubArenas.size();
}

// Grab a copy of the current Arena state
HRESULT ArenaManager::getArenaInfo( ARENAITEM * m_pArenaInfo ) {
	
	if( m_pArenaInfo == NULL ) return S_FALSE;
	
	ARENA_LOCK;		// Let's grab exclusive rights here, so we can't update while we're trying to retrieve a copy
	*m_pArenaInfo = m_arenaInfo;
	ARENA_UNLOCK;
	
	return S_OK;
}

// Handle the Kai Message for Entering an Arena
void ArenaManager::OnEnterArena( string aVector, BOOL bCanHost )
{
	ARENA_LOCK;

	// Let's find the last '/' in the vector path
	int nDelimiter = aVector.rfind('/');
	if( nDelimiter > 0 ) {
		// Store the vector name
		m_arenaInfo.szArenaName = aVector.substr( nDelimiter + 1, aVector.length() - 1);
	} else {
		m_arenaInfo.szArenaName = aVector;
	}

	// Create a wstring version for use in UI
	m_arenaInfo.szArenaNameW = strtowstr(m_arenaInfo.szArenaName);

	// Initialize any other information we have regarding this arena
	m_arenaInfo.szArenaVector = aVector;
	m_arenaInfo.szArenaVectorW = strtowstr(aVector);

	m_arenaInfo.bHostingAllowed = bCanHost == TRUE ? true : false;

	// Clear previous sub arenas
	m_arenaInfo.vSubArenas.clear();

	// Clear previous player list - deleting them as required
//	vector<CKaiPlayer*>::iterator itr;
//	for(itr = m_arenaInfo.vArenaPlayers.begin(); itr != m_arenaInfo.vArenaPlayers.end(); ++itr)	{
//		if( *itr != NULL )
//			delete *itr;
		
//		*itr = NULL;
//	}
	m_arenaInfo.vArenaPlayers.clear();

	// Reset remaining variables
	m_arenaInfo.nSelectedPlayer = -1;

	ARENA_UNLOCK;
}


// Handle the kai message for the enumeration of SubVectors
void ArenaManager::OnNewArena( string aVector, string aDescription, int nPlayers, int nPlayerLimit, int nPassword, int aSubVectors, bool bPersonal )
{
	ARENA_LOCK;

	SUB_ARENAITEM subArenaItem;

	// Let's find the last '/' in the vector path
	int nDelimiter = aVector.rfind('/');
	if( nDelimiter > 0 ) {
		// Store the vector name
		subArenaItem.szArenaName = aVector.substr( nDelimiter + 1, aVector.length() - 1);
	} else {
		subArenaItem.szArenaName = aVector;
	}

	// Create a wstring version for use in UI
	subArenaItem.szArenaNameW = strtowstr(subArenaItem.szArenaName);

	// Initialize any other information we have regarding this sub arena
	subArenaItem.szArenaVector = aVector;
	subArenaItem.szArenaVectorW = strtowstr(aVector);
	subArenaItem.szDescription = aDescription;
	subArenaItem.szDescriptionW = strtowstr(aDescription);
	subArenaItem.nPlayers = nPlayers;
	subArenaItem.nPlayerLimit = nPlayerLimit;
	subArenaItem.bHasPassword = nPassword == 1 ? true : false;
	subArenaItem.bIsPersonal = bPersonal;
	subArenaItem.szDisplayPlayers = nPlayers != -1 ? sprintfaW(L"%d Players", nPlayers) : L"? Players";
	subArenaItem.szDisplayPlayerLimit = nPlayerLimit != -1 ? sprintfaW(L"%d Max Players", nPlayerLimit) : L"? Max Players";
	subArenaItem.szDisplayPrivateArena = aSubVectors != -1 ? sprintfaW(L"%d Private Arenas", aSubVectors) : L"? Private Arenas";

	// Set up our sub arena avatar paths
	subArenaItem.szArenaAvatarPath = sprintfaA("file://%s", FileSavePath( aVector ).c_str());
	subArenaItem.szArenaAvatarPathW = strtowstr( subArenaItem.szArenaAvatarPath );
	subArenaItem.bIsLoaded = false;

	// If this is not a personal arena, let's download our arena avatar
	if( subArenaItem.bIsPersonal == false ) {
		if( DownloadArenaAvatar( subArenaItem.szArenaVector ) == S_FALSE ) {
			// If this function returns false, it means the avatar was already downloaded
			subArenaItem.bIsLoaded = true;
		}
	}

	// Add this sub arena to our sub arena list for the current arena
	m_arenaInfo.vSubArenas.push_back( subArenaItem );

	// Unlock
	ARENA_UNLOCK;
}

// Handle the kai message to update the current arena
void ArenaManager::OnUpdateArena( string aVector, int nPlayers, int aSubVectors )
{
	ARENA_LOCK;		// Lock 

	// Iterate through the list of sub arenas and update the proper sub arena
	vector<SUB_ARENAITEM>::iterator iter;
	for( iter = m_arenaInfo.vSubArenas.begin(); iter != m_arenaInfo.vSubArenas.end(); ++iter ) {
		// If the vectors match, let's update our information
		if( strcmp( (*iter).szArenaVector.c_str(), aVector.c_str()) == 0 ) {
			(*iter).nPlayers = nPlayers;
			(*iter).nPrivateArenas = aSubVectors;
			(*iter).szDisplayPlayers = nPlayers != -1 ? sprintfaW(L"%d Players", nPlayers) : L"? Players";
			(*iter).szDisplayPrivateArena = aSubVectors != -1 ? sprintfaW(L"%d Private Arenas", aSubVectors) : L"? Private Arenas";
		}
	}

	ARENA_UNLOCK;	// Unlock
}

// Handle the kai message for an opponent update within the current arena
void ArenaManager::OnOpponentPing(string aOpponent, DWORD aPing, int aStatus, string aBearerCapability)
{
	ARENA_LOCK;		// Lock

	// Iterate through the active players to find the appropriate player
	vector<CKaiPlayer>::iterator iter;
	for( iter = m_arenaInfo.vArenaPlayers.begin(); iter != m_arenaInfo.vArenaPlayers.end(); ++iter ) {
		CKaiPlayer& player = (*iter);
		wstring aOpponentW = strtowstr(aOpponent);
		// Check if we have found the correct player
		if( wcscmp( player.GetPlayerName(), aOpponentW.c_str() ) == 0 ) {
			player.SetPlayerPing(aPing);
			player.SetPlayerStatus(aStatus);
			player.SetPlayerCapabilities( aBearerCapability );

			// Set up our status text
			string szStatusText = "Unknown";
			switch (aStatus ) {
				case 0: szStatusText = "Idle"; break;
				case 1: szStatusText = "Busy"; break;
				case 2: szStatusText = "Hosting"; break;
				case 3: szStatusText = "Dedicated"; break;
			};

			// Assign the status text to the player item
			player.SetPlayerStatusText( szStatusText );
		}
	}
	ARENA_UNLOCK;	// Unlock
}

// Handle the kai message for when a new player enters the current arena
void ArenaManager::OnOpponentEnter( string aOpponent )
{
	ARENA_LOCK;		// Lock
	// Iterate through the existing player list to check for duplicates
	vector<CKaiPlayer>::iterator iter;
	for( iter = m_arenaInfo.vArenaPlayers.begin(); iter != m_arenaInfo.vArenaPlayers.end(); ++iter ) {
		CKaiPlayer& player = (*iter);
		wstring aOpponentW = strtowstr(aOpponent);
		// Check if we have found the correct player
		if( wcscmp( player.GetPlayerName(), aOpponentW.c_str() ) == 0 ) {
			// We have found this player in our list, let's return without adding
			ARENA_UNLOCK;
			return;
		}
	}
	// Create a new player object
	CKaiPlayer * newPlayer = new CKaiPlayer(aOpponent);
	
	// Add the player to our vector of players
	m_arenaInfo.vArenaPlayers.push_back(*newPlayer);

	delete newPlayer;
	
	ARENA_UNLOCK;		// Unlock
}

// Handle the kai message for when an existing player leaves the current arena
void ArenaManager::OnOpponentLeave( string aOpponent )
{
	ARENA_LOCK;		// Lock

	bool bFoundPlayer = false; CKaiPlayer& player = CKaiPlayer("");
	// Iterate through the existing player list to find the player
	vector<CKaiPlayer>::iterator iter;
	for( iter = m_arenaInfo.vArenaPlayers.begin(); iter != m_arenaInfo.vArenaPlayers.end(); ++iter ) {
		player = (*iter);
//		if( player == NULL ) continue;
		wstring aOpponentW = strtowstr(aOpponent);
		// Check if we have found the correct player
		if( wcscmp( player.GetPlayerName(), aOpponentW.c_str() ) == 0 ) {
			// Found the player, let's break and remove this player from the list
			bFoundPlayer = true;
			break;
		}
	}

	// We found a player in the list, now let's delete it
	if(bFoundPlayer) {
		m_arenaInfo.vArenaPlayers.erase(iter);
		//delete player;
	}

	ARENA_UNLOCK;		// Unlock
}

void ArenaManager::StartChatChannel(void)
{
	ARENA_LOCK;		// Lock

	// Clear our existing Chat Buffer
	m_szChatBuffer = L"";

	ARENA_UNLOCK;		//Unlock

	// Trigger our chat monitoring flag
	m_bChatActive = true;
}

void ArenaManager::StopChatChannel(void)
{// Trigger our chat monitoring flag
	m_bChatActive = false;

	ARENA_LOCK;		// Lock
	
	// Clear our existing Chat Buffer
	m_szChatBuffer = L"";

	ARENA_UNLOCK;		// Unlock

}

void ArenaManager::clearChatBuffer(void) {
	m_szChatBuffer = L"";
}

wstring& ArenaManager::getChatBuffer() {
	return m_szChatBuffer;
}

// Send a chat message to the kai client
void ArenaManager::sendChatMessage( wstring aMessage ) {
	KaiManager::getInstance().KaiChat( wstrtostr(aMessage) );
}

void ArenaManager::OnChat(string aVector, string aOpponent, string aMessage, bool bPrivate )
{

	// First, let's check if the chat is active- ( we don't want to build our buffer up if we're not viewing it )
	if( m_bChatActive == false ) return;
	
	// This class doesn't handle private messages, so let's bail if it is one.
	if( bPrivate == true) return;
	
	ARENA_LOCK;			// Lock

	m_szChatBuffer.append( strtowstr( aOpponent ) );
	m_szChatBuffer.append( L" : " );
	m_szChatBuffer.append( strtowstr( aMessage ) );
	m_szChatBuffer.append( L"\n"  );

	ARENA_UNLOCK;		// Unlock
}

HRESULT ArenaManager::DownloadArenaAvatar( string aVector )
{
	HRESULT retVal = S_FALSE;
	
	// Grab the urlPath for the requested avatar 
	string szImageUrlPath = FormatURL( aVector );
	// Grab the save path for the downloaded file
	string szImageSavePath = FileSavePath( aVector );

	// First, make sure the file doesn't already exist
	if( !FileExistsA( szImageSavePath ) ) {
		string szTag = sprintfaA("XlinkKaiArena|%s", aVector.c_str() );
		
		// Set up our http item object
		HttpItem * item = HTTPDownloader::getInstance().CreateHTTPItem( szImageUrlPath.c_str() );
		item->setTag(szTag.c_str());
		item->setRequester(this);
		item->SetPriority(HTTP_PRIORITY_HIGH);

		// Add the downloader to the queue
		HTTPDownloader::getInstance().AddToQueue(item);

		retVal = S_OK;
	}

	// Return
	return retVal;
}

HRESULT ArenaManager::DownloadPlayerAvatar( string aPlayerURL, string aPlayerName, bool isBuddy )
{
	// Create our tag
	string szTag = sprintfaA("XlinkKaiPlayer|%s|%d", aPlayerName.c_str(), isBuddy ? 1 : 0 );

	// Set up our HTTP Item Object
	HttpItem * item = HTTPDownloader::getInstance().CreateHTTPItem(aPlayerURL.c_str());
	item->setTag( szTag.c_str() );
	item->setRequester( this );
	item->SetPriority( HTTP_PRIORITY_NORMAL );

	// Add this item to the download queue
	HTTPDownloader::getInstance().AddToQueue(item);

	// Return Successfully
	return S_OK;
}

//Download arena images
void ArenaManager::DownloadCompleted(HttpItem* itm)
{
	// First let's check the response code, and return if it failed
	if( itm->getResponseCode() != 200 ) {
		DebugMsg("ArenaManager", "Download Failed:  Response Code %d", itm->getResponseCode());
		return;
	}

	// If the download doesn't have a save path, then it was downloaded to memory
	if( strcmp( itm->getSaveAsPath().c_str(), "") == 0 ) {

		// Let's first grab our tag and figure out what the heck just downloaded
		string szTag = itm->getTag();
		vector<string> vTagParts;
		StringSplit( szTag, "|", &vTagParts );

		if( vTagParts.size() > 1 ) {
			if( strcmp( vTagParts.at(0).c_str(), "XlinkKaiArena") == 0 ) {

				DebugMsg("ArenaManager", "Download Complete - %s", vTagParts.at(1).c_str());

				// Let's grab our save as path from our tag
				string szFileSavePath = getArenaAvatarSavePath( vTagParts.at(1) );
				szFileSavePath.append(".dds");

				// Retrieve the memory buffer from the downloaded item, and convert to DDS
				MemoryBuffer& buff = itm->GetMemoryBuffer();
				#ifdef _USING_DXT5
					ConvertImageInMemoryToDXT5(szFileSavePath, buff.GetData(), buff.GetDataLength());
				#else if
					ConvertImageInMemoryToDXT1(szFileSavePath, buff.GetData(), buff.GetDataLength());
				#endif
			}
			else if( strcmp( vTagParts.at(0).c_str(), "XlinkKaiPlayer") == 0) {

				DebugMsg("ArenaManager", "Download Complete - %s", vTagParts.at(1).c_str());

				// Let's grab our save as path from our tag
				bool isBuddy = atoi(vTagParts.at(2).c_str()) == 1 ? true : false;
				string szFileSavePath = getPlayerAvatarSavePath(vTagParts.at(1), isBuddy );
				szFileSavePath.append(".dds");

				// Retrieve the memory buffer from the downloaded item, and convert to DDS
				MemoryBuffer& buff = itm->GetMemoryBuffer();
				#ifdef _USING_DXT5
					ConvertImageInMemoryToDXT5(szFileSavePath, buff.GetData(), buff.GetDataLength());
				#else if
					ConvertImageInMemoryToDXT1(szFileSavePath, buff.GetData(), buff.GetDataLength());
				#endif
			}
			else {
				DebugMsg("ArenaManager", "Download Complete - Unknown" );
			}
		}
		else
		{
			DebugMsg("ArenaManager", "Download Error - Unknown Tag");
		}
	}
	else
	{
		DebugMsg("ArenaManager", "Invalid Download");
	}

	// Download Completed
}

string ArenaManager::FormatURL(string aVector) {
	// First let's make the vector path url-safe
	aVector = str_replaceallA(aVector, " ", "%20");
	// Now let's return our new url
	return sprintfaA("http://www.teamxlink.co.uk/media/avatars/%s.jpg", aVector.c_str());
}

// Grabs the save path for a specific arena avatar
string ArenaManager::getArenaAvatarSavePath(string aVector)
{
	// First, let's create our base folder, if it doesnt exist
	string szDataPath = sprintfaA("%s\\XlinkKaiData", SETTINGS::getInstance().getDataPath().c_str() );
	szDataPath = str_replaceallA( szDataPath, "\\\\", "\\" );
	if(!FileExistsA( szDataPath )) {
		_mkdir(szDataPath.c_str());
	}

	// Next, let's create our arena folder to hold the arena data
	string szArenaPath = sprintfaA("%s\\Arena", szDataPath.c_str() );
	szArenaPath = str_replaceallA( szArenaPath, "\\\\", "\\" );
	if(!FileExistsA( szArenaPath )) {
		_mkdir(szArenaPath.c_str());
	}

	// Last, we create our folder with the folder name being the vector path
	aVector = str_replaceallA(aVector,"/","\\");
	string szVectorPath = sprintfaA("%s\\%s", szArenaPath.c_str(), aVector.c_str());
	szVectorPath = str_replaceallA(szVectorPath,"\\\\","\\");
	if(!FileExistsA(szVectorPath)) {
		RecursiveMkdir(szVectorPath);
	}

	return szVectorPath;
}

string ArenaManager::getPlayerAvatarSavePath(string aPlayerName, bool isBuddy)
{
	// First, let's create our base folder, if it doesnt exist
	string szDataPath = sprintfaA("%s\\XlinkKaiData", SETTINGS::getInstance().getDataPath().c_str() );
	szDataPath = str_replaceallA( szDataPath, "\\\\", "\\" );
	if(!FileExistsA( szDataPath )) {
		_mkdir(szDataPath.c_str());
	}

	// Check for the player base path
	string szPlayerPath = sprintfaA("%s\\Player", szDataPath.c_str());
	szPlayerPath = str_replaceallA( szPlayerPath, "\\\\", "\\");
	if(!FileExistsA( szPlayerPath )) {
		_mkdir(szPlayerPath.c_str());
	}

	// If the player is a buddy, let's place him in a folder to be accessed later
	string szBuddyPath = "";
	if(isBuddy == true) {
		szBuddyPath = sprintfaA("%s\\%s", szPlayerPath.c_str(), aPlayerName.c_str());
		szBuddyPath = str_replaceallA(szBuddyPath, "\\\\", "\\");
	} else {
		// Otherwise, let's just place the image in a generic oppenent folder
		szBuddyPath = sprintfaA("%s\\Opponent", szPlayerPath.c_str() );
		szBuddyPath = str_replaceallA( szBuddyPath, "\\\\", "\\");
	}

	// If the folder exists already, just unlink- we're going to replace it
	if( FileExists(szBuddyPath) ) {
		_unlink(szBuddyPath.c_str());
	}

	// Return with the save path for the player avatar
	return szBuddyPath;
}

string ArenaManager::FileSavePath(string aVector) {
	string szDataPath = SETTINGS::getInstance().getDataPath();
	string szVectorPath = str_replaceallA( aVector, "/", "\\" );

	szDataPath = sprintfaA("%s\\XlinkKaiData\\%s.dds", szDataPath.c_str(), szVectorPath.c_str());
	szDataPath = str_replaceallA( szDataPath, "\\\\", "\\" );

	return szDataPath;
}

void ArenaManager::ClearArena()
{
	ARENA_LOCK;		// Lock

	// Clear our sub arena vector
	m_arenaInfo.vSubArenas.clear();

	// Iterate through each player in our player vector and delete 
//	vector<CKaiPlayer*>::iterator itr;
//	for(itr = m_arenaInfo.vArenaPlayers.begin(); itr != m_arenaInfo.vArenaPlayers.end(); ++itr)
//	{
//		delete *itr;
//		*itr = NULL;
//	}
	// Clear the vector list
	m_arenaInfo.vArenaPlayers.clear();

	ARENA_UNLOCK;	// Unlock
}