#pragma once
#include "GameListItem.h"
#include "../../Generic/tools.h"
#include "../../ContentList/ContentManager.h"

class GameListManager
{
private :

	CRITICAL_SECTION lock;

	std::map<CONTENT_ITEM_TAB, GameListContent> m_GameLists;
	GameListManager(){ InitializeCriticalSection(&lock);}
	 ~GameListManager() {}
    GameListManager(const GameListManager&);                 // Prevent copy-construction
    GameListManager& operator=(const GameListManager&);      // Prevent assignment
public:
	
	static GameListManager& getInstance()
	{
		static GameListManager singleton;
		return singleton;
	}
	
	void SortListAlpha(CONTENT_ITEM_TAB listName);
	void SortListAlpha(GameListContentPtr vListContent);
	void SortListAlphaReverse(CONTENT_ITEM_TAB listName);
	void SortListAlphaReverse(GameListContentPtr vListContent);
	
	void SortListRating(CONTENT_ITEM_TAB listName);
	void SortListRating(GameListContentPtr vListContent);
	void SortListRatingReverse(CONTENT_ITEM_TAB listName);
	void SortListRatingReverse(GameListContentPtr vListContent);

	void SortListUserRating(CONTENT_ITEM_TAB listName);
	void SortListUserRating(GameListContentPtr vListContent);
	void SortListUserRatingReverse(CONTENT_ITEM_TAB listName);
	void SortListUserRatingReverse(GameListContentPtr vListContent);	

	void SortListRaters(CONTENT_ITEM_TAB listName);
	void SortListRaters(GameListContentPtr vListContent);
	void SortListRatersReverse(CONTENT_ITEM_TAB listName);
	void SortListRatersReverse(GameListContentPtr vListContent);

	void SortListReleaseDate(CONTENT_ITEM_TAB listName);
	void SortListReleaseDate(GameListContentPtr vListContent);
	void SortListReleaseDateReverse(CONTENT_ITEM_TAB listName);
	void SortListReleaseDateReverse(GameListContentPtr vListContent);

	void SortListLastPlayedDate(CONTENT_ITEM_TAB listName);
	void SortListLastPlayedDate(GameListContentPtr vListContent);
	void SortListLastPlayedDateReverse(CONTENT_ITEM_TAB listName);
	void SortListLastPlayedDateReverse(GameListContentPtr vListContent);

	void ClearList(CONTENT_ITEM_TAB listName);
	int GetListSize(CONTENT_ITEM_TAB listName);
	void AddToList(CONTENT_ITEM_TAB listName, GameListItem& toAdd);
	int FindGameIndex(CONTENT_ITEM_TAB listName, GameListItem * mWGame);
	GameListItem& GetGameAt(CONTENT_ITEM_TAB listName, int index);
	HRESULT DeleteGameAt(CONTENT_ITEM_TAB listName, int index);
	void ClearFavoritesList( void );

	GameListIterator FindGameIterator(CONTENT_ITEM_TAB listName, GameListItem * mWGame);
	GameListContentPtr GetGameList(CONTENT_ITEM_TAB listName);
	HRESULT GetGameList(CONTENT_ITEM_TAB szListName, GameListContentPtr vGameListPtr);
	GameListContentPtr GetFavoritesList(CONTENT_ITEM_TAB listName);
	
};

bool sortListItemAlpha(GameListItem* entr, GameListItem* entr2);
bool sortListItemAlphaReverse(GameListItem* entr, GameListItem* entr2);

bool sortListItemRating(GameListItem* entr, GameListItem* entr2);
bool sortListItemRatingReverse(GameListItem* entr, GameListItem* entr2);

bool sortListItemUserRating(GameListItem* entr, GameListItem* entr2);
bool sortListItemUserRatingReverse(GameListItem* entr, GameListItem* entr2);

bool sortListItemRaters(GameListItem* entr, GameListItem* entr2);
bool sortListItemRatersReverse(GameListItem* entr, GameListItem* entr2);

bool sortListItemReleaseDate(GameListItem* entr, GameListItem* entr2);
bool sortListItemReleaseDateReverse(GameListItem* entr, GameListItem* entr2);

bool sortListItemLastPlayedDate(GameListItem* entr, GameListItem* entr2);
bool sortListItemLastPlayedDateReverse(GameListItem* entr, GameListItem* entr2);

