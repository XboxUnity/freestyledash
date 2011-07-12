#pragma once
#include "../XEX/Xbox360Container.h"
#include "../XEX/XeXtractor.h"
#include "../XEX/XboxExecutable.h"
#include "../HTTP/HTTPDownloader.h"
#include "../HTTP/HttpItem.h"
#include "../ContentList/MarketplaceXML/MarketPlaceXML.h"
#include "../ContentList/MarketplaceXML/MarketPlaceOfferXML.h"
#include "ContentKaiVector.h"
#include "../Settings/Settings.h"
#include "../SQLite/FSDSql.h"

#define CONTENT_ASSET_THUMBNAIL		0x01
#define CONTENT_ASSET_BACKGROUND	0x02
#define CONTENT_ASSET_BANNER		0x04
#define CONTENT_ASSET_BOXART		0x08
#define CONTENT_ASSET_PREVIEW		0x10
#define CONTENT_ASSET_SCREENSHOT	0x20
#define CONTENT_ASSET_NXE_SLOT		0x40
#define CONTENT_ASSET_COMPLETE		0x80

#define SETMAXSS(flags, max_SS)		flags |= (max_SS << 24)
#define SETCURSS(flags, cur_SS)		flags |= (cur_SS << 16)

#define CONTENT_MIN_SCREENSHOTS		2
#define CONTENT_MAX_SCREENSHOTS	    20

#define ASSET_FLAGS_SUCCESS			0x00
#define ASSET_FLAGS_FAILED			0x01
#define ASSET_FLAGS_NEVER			0x02

typedef enum _ITEM_UPDATE_TYPE {
	ITEM_UPDATE_TYPE_INFO         = 0,
	ITEM_UPDATE_TYPE_DISPLAY_INFO = 1,
	ITEM_UPDATE_TYPE_RELEASEDATE  = 2,
	ITEM_UPDATE_TYPE_BANNER       = 3,
	ITEM_UPDATE_TYPE_BACKGROUND   = 4,
	ITEM_UPDATE_TYPE_BOXART       = 5,
	ITEM_UPDATE_TYPE_PREVIEW      = 6,
	ITEM_UPDATE_TYPE_SCREENSHOT   = 7,
	ITEM_UPDATE_TYPE_THUMBNAIL    = 8,
	ITEM_UPDATE_TYPE_NXE_SLOT     = 9
} ITEM_UPDATE_TYPE;

typedef enum _CONTENT_ITEM_TAB {
	
	// Unkown and none
	CONTENT_UNKNOWN  = 0,
	CONTENT_NONE	 = 0,
	
	// Our content tabs
	CONTENT_FIRST	 = 1,
	CONTENT_XBLA	 = 1,
	CONTENT_360		 = 2,
	CONTENT_XBOX1	 = 3,
	CONTENT_HOMEBREW = 4,
	CONTENT_EMULATOR = 5,
	CONTENT_LAST	 = 5,

	// Special tab
	CONTENT_FAVORITE = 6,
	CONTENT_XLINKKAI = 7
} CONTENT_ITEM_TAB;

typedef enum _CONTENT_FILE_TYPE {
	CONTENT_FILE_TYPE_UNKNOWN		= 0,
	CONTENT_FILE_TYPE_XEX			= 1,
	CONTENT_FILE_TYPE_XBE			= 2,
	CONTENT_FILE_TYPE_CONTAINER		= 3,
	CONTENT_FILE_TYPE_DVD			= 4,
	CONTENT_FILE_TYPE_XEX_PENDING	= 5
} CONTENT_FILE_TYPE;

typedef enum _CONTENT_SCREENSHOT_FLAG {
	CONTENT_SCREENSHOT_FLAG_REPLACE = 0,
	CONTENT_SCREENSHOT_FLAG_ADD		= 1
} CONTENT_SCREENSHOT_FLAG;

typedef struct _CONTENT_SCREENSHOT_INFO {
	int nScreenshotIndex;
	DWORD nScreenshotFlag;
} CONTENT_SCREENSHOT_INFO;

class ContentItemNew;
typedef vector<ContentItemNew*> ContentItemVector;
typedef vector<ContentItemNew*>* ContentItemVectorPtr;

class ContentItemObserver {
public:

	virtual void ItemUpdated(ContentItemNew* ContentItem, DWORD UpdateFlags) = 0;
	virtual void ItemDeleted(ContentItemNew* ContentItem) = 0;
};

class ContentItemNew: public iHttpItemRequester {

private:

	DWORD contentItemId; 

	// Item Desciption
	DWORD			  contentType;
	CONTENT_ITEM_TAB  itemTab;

	// Item Location Info
	DWORD     itemScanPathId;
	string	  itemRoot;
	string    itemPath;
	string	  itemDirectory;
	string	  itemFileName;

	// Item Execution Info
	DWORD	titleId;
	DWORD	titleMediaId;
	DWORD	titleDiscNum;
	DWORD	titleDiscsInSet;

	// Item Display Info
	wstring titleName;
	wstring titleDescription;
	wstring titleDeveloper;
	wstring titlePublisher;
	wstring titleGenre;
	wstring titleRating;
	wstring titleRaters;
	wstring titleReleaseDate;

	// Item Asset Info
	DWORD   assetFlags;
	DWORD   screenshotCount;
	BYTE*   thumbnailData;
	DWORD   thumbnailSize;
	string  assetDirectory;
	string  backgroundPath;
	string  bannerPath;
	string  boxartPath;
	string  previewPath;
	string  nxeSlotPath;
	vector<string> screenshotPaths;

	// Content Hash
	string contentHash;

	// Plugin Data File Location
	string  pluginDataPath;

	// Other Fields
	wstring				 titleMIdStr;
	wstring				 titleIdStr;
	ContentItemObserver* itemObserver;

	DWORD			   httpItemCount;
	vector<HTTP_ITEM> httpItemQueue;
	
	// User Fields
	int       titleUserRating;
	BOOL      titleIsFavorite;
	string    titleLastPlayedDate;
	ULONGLONG titleLastPlayed;

	void LoadBasePath();

	// Web info functions
	void BeginGetMarketPlaceInfo();
	void ParseMarketPlaceInfo(HttpItem* Item);
	void ParseMarketPlaceOfferInfo(HttpItem* Item);
	void ParseMarketPlaceASXInfo(HttpItem* Item);
	void DownloadCompleted(HttpItem* itm);

	// Database functions
	DWORD AddItemToDatabase();
	void  DeleteItemFromDatabase();
	void  UpdateContentInfo();
	void  UpdateDisplayInfo(BOOL UpdateDB = TRUE);

	void ItemUpdated(DWORD UpdateFlags);
	void ItemDeleted();

	void PrepareForMultiDiscLaunch();
	void ConsolidateTitleUpdates();

public:

	CONTENT_FILE_TYPE fileType;
	
	// Item Info Properties
	DWORD			  GetItemId();
	CONTENT_FILE_TYPE GetFileType(bool internalType = false);
	CONTENT_ITEM_TAB  GetItemTab();
	DWORD			  GetScanPathId();
	DWORD			  getTitleId();
	const wstring&	  getId();
	string&			  getRoot();
	string&			  getDirectory();
	string&		      getPath();
	string&		      getExecutable();
	DWORD			  getType();
	int				  getCurrentDisc();
	int				  getIdCounter();
	const wstring&	  getMId();
	
	// Item Display Info Properties
	const wstring&   getTitle();	
	const wstring&   getDescription();
	const wstring&   getGenre();
	const wstring&   getDeveloper();
	const wstring&   getReleaseDate();
	const wstring&   getRating();
	const wstring&   getRaters();
	void			 setTitle(const WCHAR* Title, BOOL UpdateDB = TRUE);
	void			 setDisplayInfo( string szTitle, string szDescription, string szGenre, string szDeveloper, BOOL UpdateDB = TRUE );
	
	// Item Asset Info Properties
	DWORD			 GetAssetFlags();
	string&			 getAssetDirectory();
	string&		     getBoxArtPath();
	string&		     getBannerPath();
	DWORD		     getIconData(LPBYTE * iconData);
	string		     getScreenshotPath(DWORD Index);
	string&		     getBackgroundPath();	
	string&			 getPreviewPath();
	DWORD			 getScreenshotCount();
	string&			 getSlotPath();
	void			 SetAssetFlag(DWORD AssetFlag);
	void			 SetIconData(BYTE* IconData, DWORD IconSize);
	void			 SetSlotPath(const CHAR* SlotPath);
	void			 SetBackgroundPath(const CHAR* BackgroundPath);

	void			 AddScreenshot( void ) { screenshotCount++; }
	string&			 getPluginDataDirectory();

	
	// HTTP Resuming related functions
	string&			 getContentHash();
	DWORD			 getHttpItemCount()	{ return httpItemCount; }
	void			 ResumeGetMarketplaceInfo();
	
	// User Properties
	DWORD		     getUserRating();
	BOOL   		     getIsFavorite();
	string			 getLastPlayedDate();
	LONGLONG		 getLastPlayedOrder();
	void			 setUserRating(int value, bool savetoDb);
	void			 setFavorite(BOOL isFav);
	void			 setFileType(CONTENT_FILE_TYPE type);
	
	// Other properties
	BOOL			     isKaiCompatible();
	ContentItemObserver* GetItemObserver();
	void				 SetItemObserver(ContentItemObserver* ItemObserver);
	
	// Constructors\Deconstructor
	ContentItemNew();
	ContentItemNew(CONTENT_ITEM_LOAD_INFO& ItemInfo, ContentItemObserver* ItemObserver = NULL);
	~ContentItemNew();

	// Special create functions
	static ContentItemNew* CreateItem(ScanPath* ScanPath, string ItemDirectory, string ItemName);

	void AddAsset(ITEM_UPDATE_TYPE AssetType, VOID* AssetData, DWORD AssetDataLength = 0, VOID * AssetInfo = NULL);
	void HideItem();
	void MoveItem(CONTENT_ITEM_TAB NewTab);
	void DeleteItem();
	void RefreshAssets();
	void LaunchGame();	
};