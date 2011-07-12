#include "stdafx.h"

#include "GameListItem.h"
#include "../../Generic/tools.h"
#include "../../ContentList/ContentManager.h"
#include "GameListManager.h"

int GameListManager::GetListSize(CONTENT_ITEM_TAB listName)
{
	return m_GameLists[listName].size();
}

GameListContentPtr GameListManager::GetGameList(CONTENT_ITEM_TAB listName)
{
	GameListContentPtr test = &m_GameLists[listName];
	return test;
}

HRESULT GameListManager::GetGameList(CONTENT_ITEM_TAB szListName, GameListContentPtr vGameListPtr)
{
	int nListSize = (int)m_GameLists[szListName].size();

	for(int nIndex = 0; nIndex < nListSize; nIndex++)
	{
		(*vGameListPtr).push_back(m_GameLists[szListName].at(nIndex));
	}
	return S_OK;
}

GameListContentPtr GameListManager::GetFavoritesList(CONTENT_ITEM_TAB listName)
{
	int nListSize = (int)m_GameLists[listName].size();

	m_GameLists[CONTENT_FAVORITE].clear();
	for(int nIndex = 0; nIndex < nListSize; nIndex++)
	{
		GameListItem * pGameContent = m_GameLists[listName].at(nIndex);
		if(pGameContent->Flags.m_bFavorite) { 
			AddToList(CONTENT_FAVORITE, *pGameContent);
		}
	}
	if(m_GameLists[CONTENT_FAVORITE].size() > 0)
		return &m_GameLists[CONTENT_FAVORITE];
	else
		return NULL;
}

void GameListManager::ClearFavoritesList( void )
{
	m_GameLists[CONTENT_FAVORITE].clear();
}

void GameListManager::AddToList(CONTENT_ITEM_TAB listName, GameListItem& toAdd)
{
	EnterCriticalSection(&lock);
	m_GameLists[listName].push_back(&toAdd);
	LeaveCriticalSection(&lock);
}

GameListItem& GameListManager::GetGameAt(CONTENT_ITEM_TAB listName, int index)
{
	GameListItem * retVal = m_GameLists[listName].at(index);
	return *retVal;
}

HRESULT GameListManager::DeleteGameAt(CONTENT_ITEM_TAB listName, int index)
{
	if(index < (int)m_GameLists[listName].size()){
		GameListItem * mWGame = m_GameLists[listName].at(index);

		m_GameLists[listName].erase(FindGameIterator(listName, mWGame));

//		if(mWGame != NULL)
//			delete mWGame;
		return S_OK;
	}

	return S_FALSE;
}	

GameListIterator GameListManager::FindGameIterator(CONTENT_ITEM_TAB listName, GameListItem * mWGame)
{
	string path = mWGame->ContentRef->getRoot() + mWGame->ContentRef->getPath();
	string executable = mWGame->ContentRef->getExecutable();

	vector<GameListItem*>::iterator itr;
	int nIndexCounter = 0;

	for(itr = m_GameLists[listName].begin(); itr != m_GameLists[listName].end(); itr++)
	{
		string newPath = (*itr)->ContentRef->getRoot() + (*itr)->ContentRef->getPath();
		if(strcmp(path.c_str(), newPath.c_str()) == 0)
			break;

		nIndexCounter++;
	}

	return itr;
}
int GameListManager::FindGameIndex(CONTENT_ITEM_TAB listName, GameListItem * mWGame)
{
	string path = mWGame->ContentRef->getRoot() + mWGame->ContentRef->getPath();
	string executable = mWGame->ContentRef->getExecutable();

	vector<GameListItem*>::iterator itr;
	int nIndexCounter = 0;

	for(itr = m_GameLists[listName].begin(); itr != m_GameLists[listName].end(); itr++)
	{
		string newPath = (*itr)->ContentRef->getRoot() + (*itr)->ContentRef->getPath();
		if(strcmp(path.c_str(), newPath.c_str()) == 0)
			break;

		nIndexCounter++;
	}

	return nIndexCounter;
}

void GameListManager::ClearList(CONTENT_ITEM_TAB listName)
{
	vector<GameListItem*>::iterator itr;
	for(itr = m_GameLists[listName].begin(); itr != m_GameLists[listName].end(); itr++)
	{
		delete *itr;
		*itr = NULL;
	}
	m_GameLists[listName].clear();
}

void GameListManager::SortListAlpha(CONTENT_ITEM_TAB listName)
{
	EnterCriticalSection(&lock);
	sort(m_GameLists[listName].begin(),m_GameLists[listName].end(), sortListItemAlpha);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListAlpha(GameListContentPtr vListContent)
{

	EnterCriticalSection(&lock);
	sort(vListContent->begin(), vListContent->end(), sortListItemAlpha);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListAlphaReverse(CONTENT_ITEM_TAB listName)
{
	EnterCriticalSection(&lock);
	sort(m_GameLists[listName].begin(),m_GameLists[listName].end(),sortListItemAlphaReverse);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListAlphaReverse(GameListContentPtr vListContent)
{
	EnterCriticalSection(&lock);
	sort(vListContent->begin(),vListContent->end(),sortListItemAlphaReverse);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListRating(CONTENT_ITEM_TAB listName)
{
	EnterCriticalSection(&lock);
	sort(m_GameLists[listName].begin(),m_GameLists[listName].end(),sortListItemRating);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListRating(GameListContentPtr vListContent)
{
	EnterCriticalSection(&lock);
	sort(vListContent->begin(),vListContent->end(),sortListItemRating);
	LeaveCriticalSection(&lock);
}


void GameListManager::SortListRatingReverse(CONTENT_ITEM_TAB listName)
{
	EnterCriticalSection(&lock);
	sort(m_GameLists[listName].begin(),m_GameLists[listName].end(),sortListItemRatingReverse);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListUserRating(CONTENT_ITEM_TAB listName)
{
	EnterCriticalSection(&lock);
	sort(m_GameLists[listName].begin(),m_GameLists[listName].end(),sortListItemUserRating);
	LeaveCriticalSection(&lock);
}
void GameListManager::SortListUserRating(GameListContentPtr vListContent)
{
	EnterCriticalSection(&lock);
	sort(vListContent->begin(),vListContent->end(),sortListItemUserRating);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListUserRatingReverse(CONTENT_ITEM_TAB listName)
{
	EnterCriticalSection(&lock);
	sort(m_GameLists[listName].begin(),m_GameLists[listName].end(),sortListItemUserRatingReverse);
	LeaveCriticalSection(&lock);
}
void GameListManager::SortListUserRatingReverse(GameListContentPtr vListContent)
{
	EnterCriticalSection(&lock);
	sort(vListContent->begin(),vListContent->end(),sortListItemUserRatingReverse);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListRatingReverse(GameListContentPtr vListContent)
{
	EnterCriticalSection(&lock);
	sort(vListContent->begin(),vListContent->end(),sortListItemRatingReverse);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListRaters(CONTENT_ITEM_TAB listName)
{
	EnterCriticalSection(&lock);
	sort(m_GameLists[listName].begin(),m_GameLists[listName].end(),sortListItemRaters);
	LeaveCriticalSection(&lock);
}
void GameListManager::SortListRaters(GameListContentPtr vListContent)
{
	EnterCriticalSection(&lock);
	sort(vListContent->begin(),vListContent->end(),sortListItemRaters);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListRatersReverse(CONTENT_ITEM_TAB listName)
{
	EnterCriticalSection(&lock);
	sort(m_GameLists[listName].begin(),m_GameLists[listName].end(),sortListItemRatersReverse);
	LeaveCriticalSection(&lock);
}
void GameListManager::SortListRatersReverse(GameListContentPtr vListContent)
{
	EnterCriticalSection(&lock);
	sort(vListContent->begin(),vListContent->end(),sortListItemRatersReverse);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListReleaseDate(CONTENT_ITEM_TAB listName)
{
	EnterCriticalSection(&lock);
	sort(m_GameLists[listName].begin(),m_GameLists[listName].end(),sortListItemReleaseDate);
	LeaveCriticalSection(&lock);
}
void GameListManager::SortListReleaseDate(GameListContentPtr vListContent)
{
	EnterCriticalSection(&lock);
	sort(vListContent->begin(),vListContent->end(),sortListItemReleaseDate);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListReleaseDateReverse(CONTENT_ITEM_TAB listName)
{
	EnterCriticalSection(&lock);
	sort(m_GameLists[listName].begin(),m_GameLists[listName].end(),sortListItemReleaseDateReverse);
	LeaveCriticalSection(&lock);
}
void GameListManager::SortListReleaseDateReverse(GameListContentPtr vListContent)
{
	EnterCriticalSection(&lock);
	sort(vListContent->begin(),vListContent->end(),sortListItemReleaseDateReverse);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListLastPlayedDate(CONTENT_ITEM_TAB listName)
{
	EnterCriticalSection(&lock);
	sort(m_GameLists[listName].begin(),m_GameLists[listName].end(),sortListItemLastPlayedDate);
	LeaveCriticalSection(&lock);
}
void GameListManager::SortListLastPlayedDate(GameListContentPtr vListContent)
{
	EnterCriticalSection(&lock);
	sort(vListContent->begin(),vListContent->end(),sortListItemLastPlayedDate);
	LeaveCriticalSection(&lock);
}

void GameListManager::SortListLastPlayedDateReverse(CONTENT_ITEM_TAB listName)
{
	EnterCriticalSection(&lock);
	sort(m_GameLists[listName].begin(),m_GameLists[listName].end(),sortListItemLastPlayedDateReverse);
	LeaveCriticalSection(&lock);
}
void GameListManager::SortListLastPlayedDateReverse(GameListContentPtr vListContent)
{
	EnterCriticalSection(&lock);
	sort(vListContent->begin(),vListContent->end(),sortListItemLastPlayedDateReverse);
	LeaveCriticalSection(&lock);
}

bool sortListItemAlpha(GameListItem*left, GameListItem* right)
{
	int res = _wcsicmp(left->Content.m_szTitle, right->Content.m_szTitle);
	if (res < 0) return true;
	return false;
}

bool sortListItemAlphaReverse(GameListItem* left, GameListItem* right)
{
	int res = _wcsicmp(left->Content.m_szTitle, right->Content.m_szTitle);
	if (res > 0) return true;
	return false;
}

bool sortListItemUserRating(GameListItem* left, GameListItem* right)
{
	if (left->Content.m_nUserRating == right->Content.m_nUserRating) {
		int res = _wcsicmp(left->Content.m_szTitle, right->Content.m_szTitle);
		if (res < 0) return true;
		return false;
	}
	return (left->Content.m_nUserRating > right->Content.m_nUserRating);
}

bool sortListItemUserRatingReverse(GameListItem* left, GameListItem* right)
{
	if (left->Content.m_nUserRating == right->Content.m_nUserRating) {
		int res = _wcsicmp(left->Content.m_szTitle, right->Content.m_szTitle);
		if (res < 0) return true;
		return false;
	}	
	return (left->Content.m_nUserRating < right->Content.m_nUserRating);
}

bool sortListItemRating(GameListItem* left, GameListItem* right)
{
	int res = _wcsicmp(left->Content.m_szRating, right->Content.m_szRating);
	if (res > 0) return true;
	if (res == 0) {
		int res = _wcsicmp(left->Content.m_szTitle, right->Content.m_szTitle);
		if (res < 0) return true;
		return false;
	}
	return false;
}


bool sortListItemRatingReverse(GameListItem* left, GameListItem* right)
{
	int res = _wcsicmp(left->Content.m_szRating, right->Content.m_szRating);
	if (res < 0) return true;
	if (res == 0) {
		int res = _wcsicmp(left->Content.m_szTitle, right->Content.m_szTitle);
		if (res < 0) return true;
		return false;
	}
	return false;
}

bool sortListItemRaters(GameListItem* left, GameListItem* right)
{
	int l = _wtol(left->Content.m_szRaters);
	int r = _wtol(right->Content.m_szRaters);
	if (r < l) return true;
	if (r == l) {
		int res = _wcsicmp(left->Content.m_szTitle, right->Content.m_szTitle);
		if (res < 0) return true;
		return false;
	}
	return false;
}

bool sortListItemRatersReverse(GameListItem* left, GameListItem* right)
{
	int l = _wtol(left->Content.m_szRaters);
	int r = _wtol(right->Content.m_szRaters);
	if (r > l) return true;
	if (r == l) {
		int res = _wcsicmp(left->Content.m_szTitle, right->Content.m_szTitle);
		if (res < 0) return true;
		return false;
	}
	return false;
}

bool sortListItemReleaseDate(GameListItem* left, GameListItem* right)
{
	LPCWSTR l = left->Content.m_szReleaseDate;
	LPCWSTR r = right->Content.m_szReleaseDate;
	if (_wcsicmp(l, L"Not Available") == 0 || _wcsicmp(l, L"N/A") == 0)
		l = L"2001-01-01";
	if (_wcsicmp(r, L"Not Available") == 0 || _wcsicmp(r, L"N/A") == 0)
		r = L"2001-01-01";

	int res = _wcsicmp(l,r);
	if (res > 0) {
		return true;
	}
	if (res == 0) {
		int res = _wcsicmp(left->Content.m_szTitle, right->Content.m_szTitle);
		if (res < 0) return true;
		return false;
	}
	return false;
}

bool sortListItemReleaseDateReverse(GameListItem* left, GameListItem* right)
{
	LPCWSTR l = left->Content.m_szReleaseDate;
	LPCWSTR r = right->Content.m_szReleaseDate;
	if (_wcsicmp(l, L"Not Available") == 0 || _wcsicmp(l, L"N/A") == 0)
		l = L"2001-01-01";
	if (_wcsicmp(r, L"Not Available") == 0 || _wcsicmp(r, L"N/A") == 0)
		r = L"2001-01-01";

	int res = _wcsicmp(l,r);
	if (res < 0) {
		return true;
	}
	if (res == 0) {
		int res = _wcsicmp(left->Content.m_szTitle, right->Content.m_szTitle);
		if (res < 0) return true;
		return false;
	}
	return false;
}

bool sortListItemLastPlayedDate(GameListItem* left, GameListItem* right)
{
	if (left->Content.m_nLastPlayedOrder == right->Content.m_nLastPlayedOrder) {
		int res = _wcsicmp(left->Content.m_szTitle, right->Content.m_szTitle);
		if (res < 0) return true;
		return false;
	}
	return (left->Content.m_nLastPlayedOrder > right->Content.m_nLastPlayedOrder);
}

bool sortListItemLastPlayedDateReverse(GameListItem* left, GameListItem* right)
{
	if (left->Content.m_nLastPlayedOrder == right->Content.m_nLastPlayedOrder) {
		int res = _wcsicmp(left->Content.m_szTitle, right->Content.m_szTitle);
		if (res < 0) return true;
		return false;
	}
	return (left->Content.m_nLastPlayedOrder < right->Content.m_nLastPlayedOrder);
}
