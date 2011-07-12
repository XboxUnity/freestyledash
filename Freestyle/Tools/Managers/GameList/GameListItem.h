#pragma once
#include "../../ContentList/ContentItemNew.h"
#include "../../Texture/TextureItem/TextureItem.h"

class GameListItem;

// Define some GameListItem Types for clarify and ease of use
typedef vector<GameListItem*>*			GameListContentPtr;
typedef vector<GameListItem*>			GameListContent;
typedef vector<GameListItem*>::iterator GameListIterator;

class GameListItem
{
public:
	ContentItemNew * ContentRef;

	struct {
		TextureItem * m_IconTexture;
		TextureItem * m_BoxartTexture;
		TextureItem * m_BannerTexture;
		TextureItem * m_BkgTexture;
		TextureItem * m_SS1Texture;
		TextureItem * m_SS2Texture;
	} Textures;

	struct {
		LPCWSTR m_szGenre;
		LPCWSTR m_szTitle;
		LPCWSTR m_szDeveloper;
		LPCWSTR m_szId;
		LPCWSTR m_szMid;
		LPCWSTR m_szDescription;
		LPWSTR m_szIcon;
		LPCWSTR m_szReleaseDate;
		LPCWSTR m_szRating;
		LPCWSTR m_szRaters;
		LPCWSTR m_szLastPlayedDate;
		LPCWSTR m_szExecutable;
		LPCWSTR m_szPath;
		LPCWSTR m_szDeviceCode;
		ULONGLONG m_nLastPlayedOrder;
		int m_nUserRating;
		vector<int> m_vUserRating;
	} Content;

	struct {
		bool m_bFavorite;
	} Flags;

	HRESULT RefreshContentText();
	HRESULT InitializeContentText();

	wstring m_szAltText;
	wstring m_szRatingPercent;

	GameListItem(ContentItemNew *item);
	~GameListItem();
};