#pragma once
#include "../ContentList/ContentManager.h"
#include "./ContentCallbacks/ContentCallback.h"
#include "./ContentCallbacks/LoaderCallback.h"

#include "../../Scenes/GameView/GameList/GamesList.h"
#include "../Texture/TextureItem/TextureItem.h"
#include "GameContentMessages.h"

class GameContentManager : public ContentCallback, public LoaderCallback
{
public:
	static GameContentManager& getInstance() {
		static GameContentManager singleton;
		return singleton;
	}

	// List State/Settings Retrieval
	GAMECONTENT_LIST_STATE getGameListState( bool bNullState = false ) { return bNullState ? m_sNullState : m_sListState; }
	GAMECONTENT_LIST_STATE getGameListSnapshot( bool bNullState = false ) { return bNullState ? m_sNullState : m_sListSnapshot; }
	CONTENT_ITEM_TAB getGameListTabId( void ) { return m_sTabId; }
	GAMECONTENT_LIST_SETTINGS getGameListSettings( void ) { return m_sListSettings; }
	bool getDisplayFavorites( void ) { return m_bDisplayFavorites; }

	// List State/Settings Setting
	HRESULT setGameListState( GAMECONTENT_LIST_STATE * listPacket ) { m_bAttachedList ? m_sListState = *listPacket : m_sListState = m_sNullState; return S_OK; }
	HRESULT setGameListSnapshot( void ) { m_bAttachedList ? m_sListSnapshot = m_sListState : m_sListSnapshot = m_sNullState; return S_OK; }
	HRESULT setGameListTabId( CONTENT_ITEM_TAB tabId ) { m_sTabId = tabId; return S_OK; }
	HRESULT setDisplayFavorites( bool bShowFavs ) { m_bDisplayFavorites = bShowFavs; return S_OK; }
	BOOL isDisplayFavorites() { return m_bDisplayFavorites == true ? TRUE : FALSE; }

	// List State/Setting Clearing
	HRESULT clearGameListState( void ) { m_sListState = m_sNullState; return S_OK; }
	HRESULT clearGameListSnapshot( void ) { m_sListSnapshot = m_sNullState; return S_OK; }
	HRESULT clearGameListTabId( void ) { m_sTabId = CONTENT_UNKNOWN; return S_OK; }
	HRESULT clearGameListAssets( void );

	// Toggle Functions for Sort and Subtitle Views
	BOOL ToggleDisplayFavorites( void );
	BOOL ToggleFavorite( GameListItem * pGameContent );
	GAMELIST_SUBTITLEVIEW ToggleSubTitleView( void );
	GAMELIST_SORTSTYLE ToggleSortStyle( void );
	GAMELIST_SORTDIRECTION ToggleSortDirection( void );
	HRESULT SortGameList( GAMELIST_SORTSTYLE nSortStyle, GAMELIST_SORTDIRECTION nSortDirection, GameListContentPtr pContentList);

	// Public GameContentTexture.cpp Function Prototypes
	HRESULT LoadStaticTextures();
	HXUIBRUSH getSourceImageTexture(int nImageType, int * nImageIndex, GameListContentPtr listContent);
	HXUIBRUSH getMissingIconTexture() { return m_sMissingIcon.GetTextureBrush(); }
	HXUIBRUSH getMissingBoxArtTexture() { return m_sMissingBoxArt.GetTextureBrush(); }
	HXUIBRUSH getMissingBannerTexture() { return m_sMissingBanner.GetTextureBrush(); }
	HXUIBRUSH getMissingScreenshotTexture() { return m_sMissingScreenshot.GetTextureBrush(); }
	HXUIBRUSH getLoadingIconTexture() { return m_sLoadingIcon.GetTextureBrush(); }
	HXUIBRUSH getLoadingBoxArtTexture() { return m_sLoadingBoxArt.GetTextureBrush(); }
	HXUIBRUSH getLoadingBannerTexture() { return m_sLoadingBanner.GetTextureBrush(); }
	HXUIBRUSH getLoadingScreenshotTexture() { return m_sLoadingScreenshot.GetTextureBrush(); }
	HRESULT getUserRatingVector(int nRatingIn, vector<int> * vRatingOut);
	HXUIBRUSH getUserRatingTexture(int nTexIndex) { return m_vUserRatingTextures.at(nTexIndex)->GetTextureBrush(); }
	HXUIBRUSH getFavoritesTexture() { return m_sFavoritesStar.GetTextureBrush(); }

	// Initialization Methods
	HRESULT InitializeGameList( CONTENT_ITEM_TAB sTabId );
	HRESULT AttachGameList( CGamesList& GameList, CONTENT_ITEM_TAB sTabId );
	HRESULT DetachGameList( void );
	bool getIsGameListAttached( void ) { return m_bAttachedList; }

	// GameList Modifying Methods
	HRESULT RefreshCurrentContent( GameListItem * pGameContent, int nCurrentIndex, int nGameListSize );
	HRESULT RefreshSortCaptions( int nSortStyle, int nSortDirection, bool bFavoritesOnly );
	GameListContentPtr getCurrentListContent(void) { return m_vDisplayListPointer;}
	HRESULT setListContent(CONTENT_ITEM_TAB sTabId, bool onlyFavorites, bool bRetainSelection = false);
	HRESULT UpdateListContent(GAMECONTENT_LIST_STATE listPacket);
	HRESULT getUserRating(GameListItem * pGameItem, int nNewRating);
	HRESULT setUserRating(GameListItem * pGameItem, int nNewRating);
	GAMELIST_SUBTITLEVIEW getGameListSubtitleView( void ) { return m_sListState.SubTitleView; }
	
	void SetControlsAreHidden(bool bDisplay) { m_bDisplayBackground = bDisplay; }
	bool GetControlsAreHidden( void ) { return m_bDisplayBackground; }

	HRESULT AdjustUserRating( GameListItem * pGameContent, int nMinRating, int nMaxRating, int nAmount );
	
	// Delete games from list
	void DeleteGameFromList(GameListItem* pGameContent);
	
	// Public Member Variables
	bool m_bListReady;
	CGamesList * m_cGameList;
	GameListItem * m_sPreviousSortGame;

	GameListContentPtr m_vActiveListPointer;
	GameListContentPtr m_vDisplayListPointer;

	HRESULT HandleCompleteAttach( void );

protected:
	virtual void OnContentAdded(ContentItemNew * pContent);
	virtual void OnContentMassAdd(vector<ContentItemNew *> items);
	virtual void OnContentUpdated(ContentItemNew * pContent, DWORD dwContentFlags);
	virtual void OnContentCacheLoaded( DWORD dwItemCount );
	virtual void OnContentDatabaseLoaded( DWORD dwItemCount );
	virtual void OnContentScanCompleted( DWORD dwItemCount );
	virtual void OnContentLoadAssets( GameListItem * pGameContent );

private:
	// Private GameContentTexture.cpp Function Prototypes
	HRESULT ManageContentTextures( void );
	HRESULT HandleScreenshotTextures( void );
	HRESULT HandleBackgroundTexture( void );

	// Private GameContentSettings.cpp Function Prototypes
	HRESULT ReadUserSettings( void );
	HRESULT ReadSkinSettings( void );

	// Texture Item Classes For Static Images
	TextureItemList m_vUserRatingTextures;
	TextureItem m_sFavoritesStar;
	TextureItem m_sMissingIcon;
	TextureItem m_sMissingBoxArt;
	TextureItem m_sMissingScreenshot;
	TextureItem m_sMissingBanner;
	TextureItem m_sLoadingIcon;
	TextureItem m_sLoadingBoxArt;
	TextureItem m_sLoadingScreenshot;
	TextureItem m_sLoadingBanner;
	
	// Member variables
	bool m_bAttachedList;

	bool m_bDisplayBackground;
	bool m_bDisplayScreenshots;
	bool m_bDisplayVideos;
	bool m_bDisplayFavorites;

	// Global GameContentManager Variables
	CONTENT_ITEM_TAB m_sTabId;
	GAMECONTENT_LIST_STATE m_sListState;
	GAMECONTENT_LIST_STATE m_sNullState;
	GAMECONTENT_LIST_STATE m_sListSnapshot;
	GAMECONTENT_LIST_SETTINGS m_sListSettings;
	
//	GAMELIST_SORTDIRECTION m_nSortDirection;
//	GAMELIST_SORTSTYLE m_nSortStyle;
//	GAMELIST_SUBTITLEVIEW m_nSubTitleView;

	// Static Class Management
	GameContentManager& operator=(const GameContentManager&);		    // Prevent assignment
	GameContentManager(const GameContentManager&);						// Prevent copy-construction
	~GameContentManager() {}
	GameContentManager();

};