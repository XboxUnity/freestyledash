#include "stdafx.h"

#include "ContentItemNew.h"
#include "ContentExtractor.h"
#include "../Settings/Settings.h"
#include "./MarketplaceXML/MarketPlaceASX.h"
#include "../SQLite/FSDSql.h"
#include "../Managers/Drives/DrivesManager.h"
#include "../Plugin/PluginLoader.h"
#include "../Generic/DDS/DDSTools.h"
#include "../Managers/TitleUpdates/TitleUpdateManager.h"
	
// Item Info Properties
DWORD			  ContentItemNew::GetItemId()      { return contentItemId;  }
CONTENT_FILE_TYPE ContentItemNew::GetFileType(bool internalType )    { return internalType == true ? fileType : (fileType == CONTENT_FILE_TYPE_XEX_PENDING ? CONTENT_FILE_TYPE_XEX : fileType); }
CONTENT_ITEM_TAB  ContentItemNew::GetItemTab()     { return itemTab;        }
DWORD			  ContentItemNew::GetScanPathId()  { return itemScanPathId; }
DWORD			  ContentItemNew::getTitleId()     { return titleId;        }
const wstring&	  ContentItemNew::getId()          { return titleIdStr;     }
string&			  ContentItemNew::getRoot()		   { return itemRoot;       }
string&			  ContentItemNew::getDirectory()   { return itemDirectory;  }
string&		      ContentItemNew::getPath()        { return itemPath;       }
string&		      ContentItemNew::getExecutable()  { return itemPath;       }
DWORD			  ContentItemNew::getType()        { return contentType;    }
int				  ContentItemNew::getCurrentDisc() { return 0;              }
int				  ContentItemNew::getIdCounter()   { return 0;              }
const wstring&	  ContentItemNew::getMId()         { return titleMIdStr;    }
	
// Item Display Info Properties
const wstring&   ContentItemNew::getTitle()       { return titleName;        }
const wstring&   ContentItemNew::getDescription() { return titleDescription; }
const wstring&   ContentItemNew::getGenre()       { return titleGenre;       }
const wstring&   ContentItemNew::getDeveloper()   { return titleDeveloper;   }
const wstring&   ContentItemNew::getReleaseDate() { return titleReleaseDate; }
const wstring&   ContentItemNew::getRating()      { return titleRating;      }
const wstring&   ContentItemNew::getRaters()      { return titleRaters;      }
void			 ContentItemNew::setTitle(const WCHAR* Title, BOOL UpdateDB) {
	titleName = Title;
	UpdateDisplayInfo(UpdateDB);
}
	
// Item Asset Info Properties
DWORD			 ContentItemNew::GetAssetFlags()      { return assetFlags;      }
string&			 ContentItemNew::getAssetDirectory()  { return assetDirectory;  }
string&		     ContentItemNew::getBoxArtPath()      { return boxartPath;      }
string&		     ContentItemNew::getBannerPath()      { return bannerPath;      }
DWORD		     ContentItemNew::getIconData(LPBYTE * iconData) {
	*iconData = thumbnailData; 
	return thumbnailSize;

}
string&			 ContentItemNew::getPluginDataDirectory() { return pluginDataPath; }
string&			 ContentItemNew::getContentHash()		  { return contentHash; }

string		     ContentItemNew::getScreenshotPath(DWORD Index) {
	if(Index >= (int)screenshotPaths.size()) 
		return "";
	return screenshotPaths.at(Index);
}
string&		     ContentItemNew::getBackgroundPath()  { return backgroundPath;  }
string&			 ContentItemNew::getPreviewPath()	  { return previewPath;     }
DWORD			 ContentItemNew::getScreenshotCount() 
{	
	DWORD retCount = screenshotCount;
	if(retCount > (DWORD)SETTINGS::getInstance().getMaxScreenshots())
		retCount = SETTINGS::getInstance().getMaxScreenshots();
	
	return retCount;
}
string&			 ContentItemNew::getSlotPath()        { return nxeSlotPath;     }
void			 ContentItemNew::SetAssetFlag(DWORD AssetFlag) {
	assetFlags |= AssetFlag;
}
void			 ContentItemNew::SetIconData(BYTE* IconData, DWORD IconSize) {
	
	thumbnailData = IconData;
	thumbnailSize = IconSize;
	ItemUpdated(ITEM_UPDATE_TYPE_THUMBNAIL);
}
void			 ContentItemNew::SetSlotPath(const CHAR* SlotPath) {
	nxeSlotPath = SlotPath;
	ItemUpdated(ITEM_UPDATE_TYPE_NXE_SLOT);
}
void			 ContentItemNew::SetBackgroundPath(const CHAR* BackgroundPath) {
	backgroundPath = BackgroundPath;
	ItemUpdated(ITEM_UPDATE_TYPE_BACKGROUND);
}
	
// User Properties
DWORD		     ContentItemNew::getUserRating()      { return titleUserRating; }
BOOL   		     ContentItemNew::getIsFavorite()      { return titleIsFavorite; }
string			 ContentItemNew::getLastPlayedDate()  {	return titleLastPlayedDate; }
LONGLONG		 ContentItemNew::getLastPlayedOrder() {	return titleLastPlayed; }

void			 ContentItemNew::setUserRating(int value, bool savetoDb) {
	titleUserRating = value;
	if (savetoDb) {
		FSDSql::getInstance().updateUserRating(0, contentItemId, value);
	}
}

void			 ContentItemNew::setFavorite(BOOL isFav) { titleIsFavorite = isFav; }
	
// Other properties
BOOL			     ContentItemNew::isKaiCompatible() { return ContentKaiVector::getInstance().ContainsTitle(titleId); }
ContentItemObserver* ContentItemNew::GetItemObserver() { return itemObserver; }
void				 ContentItemNew::SetItemObserver(ContentItemObserver* ItemObserver) {
	itemObserver = ItemObserver;
}

void				ContentItemNew::setFileType(CONTENT_FILE_TYPE type) { 
	fileType = type;
	UpdateContentInfo();
}
	
// Functions
ContentItemNew::ContentItemNew() {

	// Initalize some data
	contentItemId  = 0;
	fileType	   = CONTENT_FILE_TYPE_UNKNOWN;
	contentType	   = 0;
	itemTab		   = CONTENT_UNKNOWN;
	itemScanPathId = 0;
	itemRoot	   = "";
	itemPath	   = "";
	itemDirectory  = "";
	itemFileName   = "";

	// Initalize our display text
	titleName		 = L"Not Available";
	titleIdStr		 = L"00000000";
	titleMIdStr		 = L"00000000";
	titleDeveloper	 = L"";
	titlePublisher	 = L"";
	titleGenre		 = L"Not Available";
	titleDescription = L"Not Available";
	titleRating		 = L"N/A";
	titleRaters		 = L"N/A";
	titleReleaseDate = L"N/A";
	titleIsFavorite	 = FALSE;
	titleLastPlayed	 = 0;

	// Initalize our execution info
	titleId			= 0;
	titleMediaId	= 0;
	titleUserRating	= 0;
	titleDiscNum	= 0;
	titleDiscsInSet	= 0;

	// Initalize our asset info
	assetFlags      = 0;
	screenshotCount = 0;
	thumbnailData   = NULL;
	thumbnailSize   = 0;
	assetDirectory	= "";
	backgroundPath	= "";
	bannerPath		= "";
	boxartPath		= "";
	previewPath		= "";
	nxeSlotPath     = "";
	screenshotPaths.clear();

	contentHash	   = "";
	httpItemCount	= 0;
	httpItemQueue.clear();

	// Initialize other
	itemObserver = NULL;
}

ContentItemNew::ContentItemNew(CONTENT_ITEM_LOAD_INFO& ItemInfo, ContentItemObserver* ItemObserver) {
		
	// Initalize some data
	contentItemId  = ItemInfo.ContentId;
	fileType	   = (CONTENT_FILE_TYPE)ItemInfo.ItemInfo.FileType;
	contentType	   = ItemInfo.ItemInfo.ContentType;
	itemTab		   = (CONTENT_ITEM_TAB)ItemInfo.ItemInfo.ItemTab;
	itemScanPathId = ItemInfo.ItemInfo.ItemScanPathId;

	itemRoot	   = DrivesManager::getInstance().getRootPathBySerialNumberStr(FSDSql::getInstance().getSerialByScanPathId(itemScanPathId));
	itemPath	   = ItemInfo.ItemInfo.ItemPath;
	itemDirectory  = ItemInfo.ItemInfo.ItemDirectory;
	itemFileName   = ItemInfo.ItemInfo.ItemFileName;

	contentHash	   = ItemInfo.ItemInfo.ContentHash;

	titleId			= ItemInfo.ItemInfo.TitleId;
	titleMediaId	= ItemInfo.ItemInfo.TitleMediaId;
	titleDiscNum	= ItemInfo.ItemInfo.TitleDiscNum;
	titleDiscsInSet	= ItemInfo.ItemInfo.TitleDiscsInSet;

	// User Fields
	titleUserRating	= ItemInfo.ItemInfo.TitleUserRating;
	titleIsFavorite = ItemInfo.ItemInfo.TitleIsFavorite;
	titleLastPlayedDate = ItemInfo.ItemInfo.TitleLastPlayedDate;
	titleLastPlayed = ItemInfo.ItemInfo.TitleLastPlayedOrder;

	// Initalize our display text
	titleName		 = ItemInfo.DisplayInfo.TitleName;
	titleDeveloper	 = ItemInfo.DisplayInfo.TitleDeveloper;
	titlePublisher	 = ItemInfo.DisplayInfo.TitlePublisher;
	titleGenre		 = ItemInfo.DisplayInfo.TitleGenre;
	titleDescription = ItemInfo.DisplayInfo.TitleDescription;
	titleRating		 = ItemInfo.DisplayInfo.TitleRating;
	titleRaters		 = ItemInfo.DisplayInfo.TitleRaters;
	titleReleaseDate = ItemInfo.DisplayInfo.TitleReleaseDate;
	
	// Initalize our asset info
	assetFlags      = ItemInfo.AssetInfo.AssetFlags;
	screenshotCount = ItemInfo.AssetInfo.ScreenshotCount;
	thumbnailData   = ItemInfo.AssetInfo.ThumbnailData;
	thumbnailSize   = ItemInfo.AssetInfo.ThumbnailSize;
	assetDirectory	= ItemInfo.AssetInfo.AssetDirectory;
	backgroundPath	= ItemInfo.AssetInfo.BackgroundPath;
	bannerPath		= ItemInfo.AssetInfo.BannerPath;
	boxartPath		= ItemInfo.AssetInfo.BoxartPath;
	previewPath		= ItemInfo.AssetInfo.PreviewPath;
	nxeSlotPath     = ItemInfo.AssetInfo.NxeSlotPath;
	screenshotPaths = ItemInfo.AssetInfo.ScreenshotPaths;

	titleIdStr		 = sprintfaW(L"%08x", titleId);
	titleMIdStr		 = sprintfaW(L"%08x", titleMediaId);
	itemObserver     = ItemObserver;

	httpItemQueue	 = ItemInfo.HttpQueue.httpItemQueue;
	httpItemCount	 = ItemInfo.HttpQueue.httpItemCount;

	LoadBasePath();

}

ContentItemNew::~ContentItemNew() {
	
	// Clear some data
	screenshotPaths.clear();
	if(thumbnailData != NULL)
		free(thumbnailData);
}

void ContentItemNew::ResumeGetMarketplaceInfo()
{
	DebugMsg("ContentItemNew", "ContentItem Has Found %d Outstanding HTTP Downloads", httpItemCount);

	for(DWORD nCount = 0; nCount < httpItemCount; nCount++)
	{
		// Create the HTTP Item
		HTTP_ITEM itemInfo = httpItemQueue.at(nCount);
		HttpItem * item = NULL;
		
		if( itemInfo.OutputToMemory == TRUE )
			item = HTTPDownloader::getInstance().CreateHTTPItem(itemInfo.Url.c_str());
		else
			item = HTTPDownloader::getInstance().CreateHTTPItem(itemInfo.Url.c_str(), itemInfo.OutputPath.c_str());
	
		// Let's recreate the HttpItem* from HTTP_ITEM info struct
		item->setQueueID(itemInfo.QueueId);		
		item->setTypeID(itemInfo.TypeId);
		item->setTag(itemInfo.Tag.c_str());
		item->setTag2(itemInfo.Tag2);
		item->SetPriority(itemInfo.Priority);
		item->setTimeStamp(itemInfo.TimeStamp);
		item->setExpiration(itemInfo.Expiration);
		item->setContentHash(itemInfo.ContentHash);
	
		// Fill in the remainder of information
		item->setRequester(this);
		item->setRetainQueue(true);

		// If the object has expired, don't add it to the queue
		if(!item->HasObjectExpired())
			HTTPDownloader::getInstance().AddToQueue(item);
	}

	httpItemQueue.clear();
	httpItemCount = 0;
}

ContentItemNew* ContentItemNew::CreateItem(ScanPath* ScanPath, string ItemDirectory, string ItemName) {

	// Get our full path
	string ItemRoot = ItemDirectory.substr(0, ItemDirectory.find_first_of(":") + 1);
	ItemDirectory = ItemDirectory.substr(ItemDirectory.find_first_of(":") + 1);
	string itemPath = ItemDirectory + ItemName;
	string itemFilePath = ItemRoot + itemPath;

	// Open our file and get the "magic" value
	FILE* fHandle; DWORD magic;
	fopen_s(&fHandle, itemFilePath.c_str(), "rb");
	if(fHandle == NULL) return NULL;
	fread(&magic, sizeof(DWORD), 1, fHandle);
	fclose(fHandle);

	// Get our file type
	CONTENT_FILE_TYPE fileType;
	string fileExt = FileExt(ItemName);
	if(magic == 0x58424548 && fileExt == "xbe")
		fileType = CONTENT_FILE_TYPE_XBE;
	else if(magic == IMAGE_XEX_HEADER_MAGIC && fileExt == "xex")
		fileType = CONTENT_FILE_TYPE_XEX_PENDING;
	else if (magic == CONSOLE_SIGNED || magic == LIVE_SIGNED || magic == PIRS_SIGNED) 
		fileType = CONTENT_FILE_TYPE_CONTAINER;
	else
		return NULL;

	// Create our content item
	ContentItemNew* item = new ContentItemNew();
	item->itemScanPathId = (DWORD)ScanPath->PathId;
	item->itemRoot = DrivesManager::getInstance().getRootPathBySerialNumberStr(FSDSql::getInstance().getSerialByScanPathId(item->itemScanPathId));
	item->itemPath = itemPath;
	item->itemDirectory = ItemDirectory;
	item->itemFileName = ItemName;
	item->fileType = fileType;

	item->contentHash = GenerateTimestampMd5();

	// If its a xex get what we can
	if(item->fileType == CONTENT_FILE_TYPE_XEX_PENDING) {
		
		// Read our xex info
		XeXtractor xex;
		if(xex.OpenXex(itemFilePath) != S_OK) {
			delete item;
			return NULL;
		}
		bool bExecId = true;
		// Get our execution id
		XEX_EXECUTION_ID executionId;
		if(xex.GetExecutionId(&executionId) != S_OK) {
			// Create a fake execution id for homebrew that doesn't have anything
			executionId.DiscNum = 1;
			executionId.DiscsInSet = 1;
			executionId.TitleID = 0;
			executionId.MediaID = 0;
		}

		// Pull some info from our execution id
		item->titleId		  = executionId.TitleID;		
		item->titleMediaId	  = executionId.MediaID;
		item->titleDiscNum	  = executionId.DiscNum;
		item->titleDiscsInSet = executionId.DiscsInSet;

		// Just so we have something to identify this
		item->contentType = XCONTENTTYPE_XBOX360TITLE;
		item->titleIdStr  = sprintfaW(L"%08x", item->titleId);
		item->itemTab     = (xex.IsRetailXex()) ? 
			(ScanPath->RetailTabId == 0 ? CONTENT_360 : (CONTENT_ITEM_TAB)ScanPath->RetailTabId) :
			(ScanPath->DevkitTabId == 0 ? CONTENT_HOMEBREW : (CONTENT_ITEM_TAB)ScanPath->DevkitTabId);

		// Set our xex name
		string titleName = ItemName.substr(0, ItemName.find_last_of("."));
		if(strcmp(titleName.c_str(), "default") == 0) {
			string pathWithoutSlash = ItemDirectory.substr(0, ItemDirectory.find_last_of("\\"));
			size_t lastSlashPos = pathWithoutSlash.find_last_of("\\");
			string parentFolderName = pathWithoutSlash.substr( lastSlashPos + 1 );
			titleName = parentFolderName;
		}
		
		WCHAR* multiByte = strtowchar(titleName);
		item->titleName = multiByte;
		delete multiByte;

		// Close our xex and clean up
		xex.CloseXex();

		// Now lets begin to get more info on this item
		ContentExtractor::getInstance().AddContentItem(item);
	}

	// If its a xbe lets get what we can
	if(item->fileType == CONTENT_FILE_TYPE_XBE) {

		// Read our xbe info
		XboxExecutable xbe;
		if(xbe.OpenExecutable(itemFilePath) != S_OK) {
			delete item;
			return NULL;
		}

		// Get the info we can
		item->titleName    = xbe.GetTitleName();
		item->titleId	   = xbe.GetTitleId();
		item->titleMediaId = xbe.GetMediaId();
		item->titleDiscNum = xbe.GetCurrentDisc();

		// Alittle extra info
		item->contentType = XCONTENTTYPE_XBOXTITLE;
		item->titleIdStr  = sprintfaW(L"%08x", item->titleId);
		item->itemTab	  = ScanPath->RetailTabId == 0 ? CONTENT_XBOX1 : (CONTENT_ITEM_TAB) ScanPath->RetailTabId;

		// Load our image and convert it to dxt
		BYTE * iconData; DWORD iconSize;
		HRESULT ret = xbe.GetGameThumbnail((void**)&iconData, (int*)&iconSize);
		if(ret == S_OK && iconSize > 0 && SETTINGS::getInstance().getDownloadIcon() == 1) {
			#ifdef _USING_DXT5
				ConvertImageInMemoryToDXT5Buffer(iconData, iconSize, &(item->thumbnailData),
					(long*)&(item->thumbnailSize));
			#else if
				ConvertImageInMemoryToDXT1Buffer(iconData, iconSize, &(item->thumbnailData),
					(long*)&(item->thumbnailSize));
			#endif
			item->assetFlags |= CONTENT_ASSET_THUMBNAIL;
			if(iconData != NULL) free(iconData);
		}
		

		// Close our xbe
		xbe.CloseExecutable();
	}

	// If its a container get what we can
	if(item->fileType == CONTENT_FILE_TYPE_CONTAINER) {

		// Read our Container info
		Xbox360Container container;
		if(container.OpenContainer(itemFilePath) != S_OK) {
			delete item;
			return NULL;
		}

		// Handle the different container types
		DWORD contentType = container.pContainerMetaData->ContentType;
		switch (contentType)
		{
		case XCONTENTTYPE_ARCADE:
			item->itemTab = ScanPath->RetailTabId == 0 ? CONTENT_XBLA : (CONTENT_ITEM_TAB) ScanPath->RetailTabId;
			break;
		case XCONTENTTYPE_XBOXTITLE:
			item->itemTab = ScanPath->RetailTabId == 0 ? CONTENT_XBOX1 : (CONTENT_ITEM_TAB) ScanPath->RetailTabId;
			break;
		case XCONTENTTYPE_XBOX360TITLE:
			item->itemTab = ScanPath->RetailTabId == 0 ? CONTENT_360 : (CONTENT_ITEM_TAB) ScanPath->RetailTabId;
			break;
		case XCONTENTTYPE_GAMEDEMO:
			item->itemTab = (container.IsRetailCon()) ?
				(ScanPath->RetailTabId == 0 ? CONTENT_360 : (CONTENT_ITEM_TAB) ScanPath->RetailTabId) :
				(ScanPath->DevkitTabId == 0 ? CONTENT_HOMEBREW : (CONTENT_ITEM_TAB) ScanPath->DevkitTabId);
			break;
/*
			// Check for NXE to convert to GOD
		case XCONTENTTYPE_INSTALLED_XBOX360TITLE:
			// TODO convert to GOD ONLY if in the settings
			if (true)
			{
				DebugMsg("ContentItemNew", "Converting NXE to GOD");
				if (container.ConvertNXE2GOD(ItemDirectory) == S_OK)
				{
					DebugMsg("ContentItemNew", "DONE: Converting NXE to GOD");
					container.CloseContainer();
					delete item;

					//make a new path
					
					// return the new contentitem
					return CreateItem(ScanPath, ItemDirectory, ItemName);

				}
			}
*/
		default:
			container.CloseContainer();
			delete item;
			return NULL;
		};

		// Get string values
		if(container.GetTitleName() != L"") 
			item->titleName = container.GetTitleName();
		else if(container.GetDisplayName() != L"")
			item->titleName = container.GetDisplayName();

		if(container.GetPublisher() != L"") 
			item->titlePublisher = container.GetPublisher();
		if(container.GetDescription() != L"") 
			item->titleDescription = container.GetDescription();


		// Handle the special case where the container is a QuickBoot Launcher
		if (item->titleName == L"QuickBoot Launcher") {
			item->titleName = container.GetDisplayName();
			wstring desc = L"QuickBoot Launcher\n\n";
			desc.append(item->titleDescription);
			item->titleDescription = desc;
		}

		// Load info from execution id
		item->titleId	      = container.pContainerMetaData->ExecutionId.TitleID;
		item->titleMediaId	  = container.pContainerMetaData->ExecutionId.MediaID;

		// ISO2GOD doesnt put the media id in the container, so we will get it from the xex if it's not there
		// and add it to the container for next time
		if (contentType = XCONTENTTYPE_XBOX360TITLE && item->titleMediaId == 0)
		{
			//get media id from the xex
			if (container.Mount() == S_OK)
			{
				XeXtractor xex;
				if(xex.OpenXex("con:\\default.xex") == S_OK)
				{
					// Get our execution id
					XEX_EXECUTION_ID executionId;
					if(xex.GetExecutionId(&executionId) == S_OK)
					{
						item->titleMediaId = executionId.MediaID;
					}
					else
					{
						executionId.MediaID = 0;
					}
				}
				xex.CloseXex();
				container.UnMount();
			}

			// add it to the container (IF we got it succesfully)
			if (item->titleMediaId != 0)
			{
				DebugMsg("ContentItemNew", "Adding media id 0x%08x to container", item->titleMediaId);
				if (container.WriteDWORDToOffset(item->titleMediaId, 0x354) == S_OK)
				{
					DebugMsg("ContentItemNew", "DONE: Adding media id to container");
					DebugMsg("ContentItemNew", "rehashing container");
					if (container.ReHash() == S_OK)
					{
						DebugMsg("ContentItemNew", "DONE: rehashing container");
					}
					else
					{
						DebugMsg("ContentItemNew", "FAILED: rehashing container");
					}
				}
				else
				{
					DebugMsg("ContentItemNew", "FAILED: Adding media id to container");
				}
			}
		}

		item->titleDiscNum    = container.pContainerMetaData->ExecutionId.DiscNum;
		item->titleDiscsInSet = container.pContainerMetaData->ExecutionId.DiscsInSet;

		// Load other info		
		item->contentType = container.pContainerMetaData->ContentType;
		item->titleIdStr  = sprintfaW(L"%08x", item->titleId);

		// Load our image and convert it to dxt
		if(container.pContainerMetaData->ThumbnailSize > 0 && SETTINGS::getInstance().getDownloadIcon() == 1) {
			BYTE * byte2 = ((container.pContainerMetaData)->Thumbnail);
			DWORD size = container.pContainerMetaData->ThumbnailSize;
			#ifdef _USING_DXT5
				ConvertImageInMemoryToDXT5Buffer(byte2, size, &(item->thumbnailData),
					(long*)&(item->thumbnailSize));
			#else if
				ConvertImageInMemoryToDXT1Buffer(byte2, size, &(item->thumbnailData),
					(long*)&(item->thumbnailSize));
			#endif
			item->assetFlags |= CONTENT_ASSET_THUMBNAIL;
		}

		// Close our container and clean up
		container.CloseContainer();		
	}

	// Add this to the database so we can get a id and stuff
	item->contentItemId = item->AddItemToDatabase();

	// Create and load our base path for data
	item->LoadBasePath();

	// Insert out thumbnail if we have one
	if(item->assetFlags & CONTENT_ASSET_THUMBNAIL) {
		FSDSql::getInstance().AddContentAsset(item->contentItemId, 
			ASSET_TYPE_ICON,  ASSET_FILE_TYPE_DDS, 
			item->thumbnailData, item->thumbnailSize);
	}

	// Now go ahead and start getting our marketplace info
	item->BeginGetMarketPlaceInfo();

	// Return our item
	return item;
}
void ContentItemNew::HideItem() {

	// Just change our tab to none
	MoveItem(CONTENT_NONE);
}
void ContentItemNew::MoveItem(CONTENT_ITEM_TAB NewTab) {

	// Set our item info
	itemTab = NewTab;

	// Now update within the DB
	UpdateContentInfo();
}
void ContentItemNew::LoadBasePath() {

	// Get our game data path
	string baseDataPath = SETTINGS::getInstance().getDataPath();
	string dataPath;
	dataPath = baseDataPath + "\\GameData";
	dataPath = str_replaceallA(dataPath, "\\\\", "\\");

	if(!FileExistsA(dataPath))
		_mkdir(dataPath.c_str());

	dataPath = dataPath + "\\" + sprintfa("%08X", contentItemId);
	dataPath = str_replaceallA(dataPath, "\\\\", "\\");

	if(!FileExistsA(dataPath))
		_mkdir(dataPath.c_str());

	assetDirectory = dataPath;	
	
	dataPath = dataPath + "\\PluginData";
	dataPath = str_replaceallA(dataPath, "\\\\", "\\");

	if(!FileExistsA(dataPath))
		_mkdir(dataPath.c_str());

	dataPath = dataPath.substr(baseDataPath.length());

	pluginDataPath = dataPath;
}
void ContentItemNew::RefreshAssets() {

	// Delete all assets for this current content id
	FSDSql::getInstance().DeleteContentItemAssets(contentItemId);

	// Clear our asset stuff
	assetFlags = 0;
	screenshotCount = 0;
	backgroundPath = "";
	bannerPath = "";
	boxartPath = "";
	previewPath = "";
	screenshotPaths.clear();

	string itemFilePath = getRoot() + getPath();
	
	// If this is an xbe or container, we need to re-extract the icon
	// If its a container let's get extract the icon
	if(fileType == CONTENT_FILE_TYPE_CONTAINER) {
		
		// Read our container info
		Xbox360Container container;
		if(container.OpenContainer(itemFilePath) != S_OK) {
			DebugMsg("ContentItem", "Refresh Assets Failed to open Xbox 360 Container");
			return;
		}
		// Load our image and convert it to dxt
		if(container.pContainerMetaData->ThumbnailSize > 0 && SETTINGS::getInstance().getDownloadIcon() == 1) {
			BYTE * byte2 = ((container.pContainerMetaData)->Thumbnail);
			DWORD size = container.pContainerMetaData->ThumbnailSize;
			#ifdef _USING_DXT5
				ConvertImageInMemoryToDXT5Buffer(byte2, size, &(thumbnailData),
					(long*)&(thumbnailSize));
			#else if
				ConvertImageInMemoryToDXT1Buffer(byte2, size, &(thumbnailData),
					(long*)&(thumbnailSize));
			#endif
			assetFlags |= CONTENT_ASSET_THUMBNAIL;
		}

		// Close our container and clean up
		container.CloseContainer();		
	}else if(fileType == CONTENT_FILE_TYPE_XBE) {

		// Read our xbe info
		XboxExecutable xbe;
		if(xbe.OpenExecutable(itemFilePath) != S_OK) {
			DebugMsg("ContentItem", "Refresh Assets Failed to open XboxExecutable");
			return;
		}

		// Load our image and convert it to dxt
		BYTE * iconData; DWORD iconSize;
		HRESULT ret = xbe.GetGameThumbnail((void**)&iconData, (int*)&iconSize);
		if(ret == S_OK && iconSize > 0 && SETTINGS::getInstance().getDownloadIcon() == 1) {
			#ifdef _USING_DXT5
				ConvertImageInMemoryToDXT5Buffer(iconData, iconSize, &(thumbnailData),
					(long*)&(thumbnailSize));
			#else if
				ConvertImageInMemoryToDXT1Buffer(iconData, iconSize, &(thumbnailData),
					(long*)&(thumbnailSize));
			#endif
			assetFlags |= CONTENT_ASSET_THUMBNAIL;
		}
		

		// Close our xbe
		xbe.CloseExecutable();
	}
	
	// Insert out thumbnail if we have one
	if(assetFlags & CONTENT_ASSET_THUMBNAIL) {
		FSDSql::getInstance().AddContentAsset(contentItemId, 
			ASSET_TYPE_ICON,  ASSET_FILE_TYPE_DDS, 
			thumbnailData, thumbnailSize);
	}

	// Now begin downloading the assests again
	BeginGetMarketPlaceInfo();

	if(fileType == CONTENT_FILE_TYPE_XEX || fileType == CONTENT_FILE_TYPE_XEX_PENDING) {
		setFileType(CONTENT_FILE_TYPE_XEX_PENDING);
		ContentExtractor::getInstance().AddContentItem(this);
	}

}

void ContentItemNew::setDisplayInfo( string szTitle, string szDescription, string szGenre, string szDeveloper, BOOL UpdateDB )
{
	titleName = FromUtf8(szTitle);
	titleDescription = FromUtf8(szDescription);
	titleGenre = FromUtf8(szGenre);
	//titlePublisher = strtowstr(szPublisher);
	titleDeveloper = FromUtf8(szDeveloper);

	UpdateDisplayInfo(UpdateDB);
}

void ContentItemNew::BeginGetMarketPlaceInfo() {

	// Get our locale
	string locale = SETTINGS::getInstance().getLocale();
	if(locale == "") locale = "en-US";

	// Setup our urls now
	string marketPlaceUrl = (string)"http://catalog.xboxlive.com/Catalog/Catalog.asmx/Query?" + 
		(string)"methodName=FindGames" + 
		(string)"&Names=Locale&Values=" + locale + 
		(string)"&Names=LegalLocale&Values=" + locale + 
		(string)"&Names=Store&Values=1&Names=PageSize&Values=100&Names=PageNum&Values=1" +
		(string)"&Names=DetailView&Values=5&Names=OfferFilterLevel&Values=1" + 
		(string)"&Names=MediaIds&Values=66acd000-77fe-1000-9115-d802" + sprintfa("%08x", titleId) + 
		(string)"&Names=UserTypes&Values=2&Names=MediaTypes&Values=1" + 
		(string)"&Names=MediaTypes&Values=21&Names=MediaTypes&Values=23" +
		(string)"&Names=MediaTypes&Values=37&Names=MediaTypes&Values=46";

	string marketplaceOfferUrl = (string)"http://catalog.xboxlive.com/Catalog/Catalog.asmx/Query?" +
		(string)"methodName=FindGameOffers" + 
		(string)"&Names=Locale&Values=" + locale + 
		(string)"&Names=LegalLocale&Values=" + locale + 
		(string)"&Names=Store&Values=1&Names=PageSize&Values=100&Names=PageNum&Values=1" + 
		(string)"&Names=DetailView&Values=5" + 
		(string)"&Names=MediaIds&Values=66acd000-77fe-1000-9115-d802" + sprintfa("%08x", titleId) + 
		(string)"&Names=UserTypes&Values=2&Names=MediaTypes&Values=1" + 
		(string)"&Names=MediaTypes&Values=21&Names=MediaTypes&Values=23" + 
		(string)"&Names=MediaTypes&Values=37&Names=MediaTypes&Values=46";

	// Create our items
	string szOutputPath = assetDirectory + "\\GameAssetInfo.bin";
	HttpItem* marketplaceItem = HTTPDownloader::getInstance().CreateHTTPItem(marketPlaceUrl.c_str(), szOutputPath.c_str());
	marketplaceItem->setTag("XBLMARKETPLACEDATA");
	marketplaceItem->SetPriority(HTTP_PRIORITY_ABOVENORMAL);
	marketplaceItem->setRequester(this);
	marketplaceItem->setQueueID(contentItemId);
	marketplaceItem->setRetainQueue(true);
	marketplaceItem->setContentHash(contentHash);
	marketplaceItem->setTypeID(HTTP_TYPE_CONTENT);

	szOutputPath = assetDirectory + "\\GameOfferInfo.bin";
	HttpItem* marketplaceOfferItem = HTTPDownloader::getInstance().CreateHTTPItem(marketplaceOfferUrl.c_str(), szOutputPath.c_str());
	marketplaceOfferItem->setTag("XBLMARKETPLACEOFFERDATA");
	marketplaceOfferItem->SetPriority(HTTP_PRIORITY_ABOVENORMAL);
	marketplaceOfferItem->setRequester(this);
	marketplaceOfferItem->setQueueID(contentItemId);
	marketplaceOfferItem->setRetainQueue(true);
	marketplaceOfferItem->setContentHash(contentHash);
	marketplaceOfferItem->setTypeID(HTTP_TYPE_CONTENT);

	// Add it to the queue now
	HTTPDownloader::getInstance().AddToQueue(marketplaceItem);
	HTTPDownloader::getInstance().AddToQueue(marketplaceOfferItem);
}

void ContentItemNew::DownloadCompleted(HttpItem* Item) {

	// Check our responce code first
	if(Item->getResponseCode() != 200) { 
		return;
	}

	// Figure out what kinda downloaded data we have and handle it
	string itemTag = Item->getTag();
	if(itemTag == "XBLMARKETPLACEDATA")
		ParseMarketPlaceInfo(Item);
	else if(itemTag == "XBLMARKETPLACEOFFERDATA")
		ParseMarketPlaceOfferInfo(Item);
	else if (itemTag == "XBLPREVIEWASX") {
		ParseMarketPlaceASXInfo(Item);
	}
	else if (itemTag == "XBLBANNER") {
		MemoryBuffer& buffer = Item->GetMemoryBuffer();
		AddAsset(ITEM_UPDATE_TYPE_BANNER, buffer.GetData(), buffer.GetDataLength());
	}
	else if (itemTag == "XBLBACKGROUND") {			
		if(assetFlags & CONTENT_ASSET_BACKGROUND)return;
		MemoryBuffer& buffer = Item->GetMemoryBuffer();
		AddAsset(ITEM_UPDATE_TYPE_BACKGROUND, buffer.GetData(), buffer.GetDataLength());
	}
	else if (itemTag == "XBLBOXART") {		
		MemoryBuffer& buffer = Item->GetMemoryBuffer();
		AddAsset(ITEM_UPDATE_TYPE_BOXART, buffer.GetData(), buffer.GetDataLength());
	}
	else if (itemTag == "XBLPREVIEWWMV") {		
		AddAsset(ITEM_UPDATE_TYPE_PREVIEW, NULL);
	}
	else if (itemTag == "XBLSCREENSHOT") {	
		MemoryBuffer& buffer = Item->GetMemoryBuffer();
		int nScreenshotNum = Item->getTag2();
		CONTENT_SCREENSHOT_INFO ssInfo;		
		screenshotCount++;
		ssInfo.nScreenshotIndex = nScreenshotNum;
		ssInfo.nScreenshotFlag = CONTENT_SCREENSHOT_FLAG_ADD;

		AddAsset(ITEM_UPDATE_TYPE_SCREENSHOT, buffer.GetData(), buffer.GetDataLength(), (void*)&ssInfo);
	}
	else 
		DebugBreak(); // Unhandled tag as of now
}
void ContentItemNew::ParseMarketPlaceASXInfo(HttpItem* Item) {

	MemoryBuffer& buffer = Item->GetMemoryBuffer();

	// We have our marketplace data now lets get our info from it
	MarketPlaceASX asx;
	ATG::XMLParser parser;
	parser.RegisterSAXCallbackInterface(&asx);
	parser.ParseXMLBuffer((CONST CHAR*)buffer.GetData(), (UINT)buffer.GetDataLength());

	if(asx.hasValidVersion == true) {
		if(asx.hasWMVUrl == true) {
			string szSaveAsPath = assetDirectory + "\\preview.wmv";
			HttpItem* newItem = HTTPDownloader::getInstance().CreateHTTPItem(asx.szWMVUrl.c_str(), szSaveAsPath.c_str());
			newItem->setTag("XBLPREVIEWWMV");
			newItem->setRequester(this);
			newItem->setQueueID(contentItemId);
			newItem->setRetainQueue(true);
			newItem->setTypeID(HTTP_TYPE_CONTENT);
			newItem->SetPriority(HTTP_PRIORITY_LOW);
			newItem->setContentHash(contentHash);
			HTTPDownloader::getInstance().AddToQueue(newItem);
		}
	}
}

void ContentItemNew::ParseMarketPlaceInfo(HttpItem* Item) {

	// We have our marketplace data now lets get our info from it
	MarketPlaceXML xml;
	ATG::XMLParser parser;
	parser.RegisterSAXCallbackInterface(&xml);
	//MemoryBuffer& buffer = Item->GetMemoryBuffer();
	parser.ParseXMLFile(Item->getSaveAsPath().c_str());
	//parser.ParseXMLBuffer((CHAR*)buffer.GetData(),	buffer.GetDataLength());

	// Fill in info that we have
	//if(xml.gotTitle)	   titleName = xml.m_Title;
	if(xml.gotDescription) titleDescription = xml.m_Description.c_str();
	if(xml.gotRating)      titleRating = xml.m_Rating.c_str();
	if(xml.gotRaters)      titleRaters = xml.m_Raters.c_str();
	if(xml.gotDeveloper)   titleDeveloper = xml.m_Developer.c_str();
	if(xml.gotPublisher)   titlePublisher = xml.m_Publisher.c_str();

	// Build Genre String
	if(xml.gotGenre) {
		string genreList = "";
		int nSize = (int)xml.m_GenreList.size();

		if(nSize == 1)
			genreList = xml.m_GenreList.at(0);
		else
		{
			for(int i = 0; i < nSize; i++)
			{
				if(i == 1)
					genreList = sprintfaA("%s", xml.m_GenreList.at(i).c_str());
				else
					genreList = sprintfaA("%s, %s", genreList.c_str(), xml.m_GenreList.at(i).c_str());
			}
		}
		titleGenre = FromUtf8(genreList);
	}
		
	if(xml.gotBanner && SETTINGS::getInstance().getDownloadBanner() == 1) {
		HttpItem* newItem = HTTPDownloader::getInstance().CreateHTTPItem(xml.m_BannerUrl.c_str());
		newItem->setTag("XBLBANNER");
		newItem->setRequester(this);
		newItem->setQueueID(contentItemId);
		newItem->setRetainQueue(true);
		newItem->setTypeID(HTTP_TYPE_CONTENT);
		newItem->SetPriority(HTTP_PRIORITY_NORMAL);
		newItem->setContentHash(contentHash);
		HTTPDownloader::getInstance().AddToQueue(newItem);
	}
	if(xml.gotBackground && SETTINGS::getInstance().getDownloadBackground() == 1) {
		HttpItem* newItem = HTTPDownloader::getInstance().CreateHTTPItem(xml.m_BackgroundUrl.c_str());
		newItem->setTag("XBLBACKGROUND");
		newItem->setRequester(this);
		newItem->setQueueID(contentItemId);
		newItem->setRetainQueue(true);
		newItem->setTypeID(HTTP_TYPE_CONTENT);
		newItem->setContentHash(contentHash);
		newItem->SetPriority(HTTP_PRIORITY_BELOWNORMAL);
		HTTPDownloader::getInstance().AddToQueue(newItem);
	}
	if(xml.gotBoxArt && SETTINGS::getInstance().getDownloadBoxart() == 1) {
		HttpItem* newItem = HTTPDownloader::getInstance().CreateHTTPItem(xml.m_BoxartUrl.c_str());
		newItem->setTag("XBLBOXART");
		newItem->setRequester(this);
		newItem->setQueueID(contentItemId);
		newItem->setRetainQueue(true);
		newItem->setTypeID(HTTP_TYPE_CONTENT);
		newItem->SetPriority(HTTP_PRIORITY_NORMAL);
		newItem->setContentHash(contentHash);
		HTTPDownloader::getInstance().AddToQueue(newItem);
	}
	if(xml.gotPreview && SETTINGS::getInstance().getDownloadVideo() == 1) {		
		HttpItem* newItem = HTTPDownloader::getInstance().CreateHTTPItem(xml.m_PreviewUrls.c_str());
		newItem->setTag("XBLPREVIEWASX");
		newItem->setRequester(this);
		newItem->setQueueID(contentItemId);
		newItem->setRetainQueue(true);
		newItem->setTypeID(HTTP_TYPE_CONTENT);
		newItem->setContentHash(contentHash);
		HTTPDownloader::getInstance().AddToQueue(newItem);
	}
	
	if( SETTINGS::getInstance().getDownloadScreenshots() == 1 )
	{
		DWORD shotCount = xml.m_ScreenshotsUrl.size();
		if(shotCount > (DWORD)SETTINGS::getInstance().getMaxScreenshots() ) shotCount = SETTINGS::getInstance().getMaxScreenshots();
		for(DWORD x = 0; x < shotCount;x++) {
			HttpItem* newItem = HTTPDownloader::getInstance().CreateHTTPItem(xml.m_ScreenshotsUrl.at(x).c_str());
			newItem->setTag("XBLSCREENSHOT");
			newItem->setTag2(x);
			newItem->setQueueID(contentItemId);
			newItem->setRetainQueue(true);
			newItem->setTypeID(HTTP_TYPE_CONTENT);
			newItem->setRequester(this);
			newItem->SetPriority(HTTP_PRIORITY_LOW);
			newItem->setContentHash(contentHash);
			HTTPDownloader::getInstance().AddToQueue(newItem);
		}
	}

	// Call our item updated
	UpdateDisplayInfo();
}
void ContentItemNew::ParseMarketPlaceOfferInfo(HttpItem* Item) {
	
	// We have our marketplace data now lets get our info from it
	MarketPlaceOfferXML xml;
	ATG::XMLParser parser;
	parser.RegisterSAXCallbackInterface(&xml);
	//MemoryBuffer& buffer = Item->GetMemoryBuffer();
	//parser.ParseXMLBuffer((CHAR*)buffer.GetData(),	buffer.GetDataLength());
	parser.ParseXMLFile(Item->getSaveAsPath().c_str());

	// Set our values
	if(xml.gotReleaseDate) titleReleaseDate = xml.m_ReleaseDate;

	// Call our item updated
	UpdateDisplayInfo();
}
void ContentItemNew::PrepareForMultiDiscLaunch() {

	// Make sure the plugin is even running
	if(!PluginLoader::getInstance().IsLoaded())
		return;

	// Check if theres more then 1
	if(titleDiscsInSet <= 1 || titleDiscsInSet > 5)
		return;

	// Now load our multidisc info
	MULTI_DISC_INFO multiDiscInfo; 
	ZeroMemory(&multiDiscInfo, sizeof(MULTI_DISC_INFO));
	FSDSql::getInstance().LoadMultiDiscInfo(titleId, titleDiscsInSet, 
		multiDiscInfo);

	// Make sure we loaded everything
	if(multiDiscInfo.DiscsFound != titleDiscsInSet) {
		DebugMsg("ContentItem", "Failed to load multidisc info for 0x%08X", titleId);
		//return;
	}

	// Now we gotta put into our plugin structure
	MULTIDISC_INFO multiInfo;
	ZeroMemory(&multiInfo, sizeof(MULTIDISC_INFO));
	multiInfo.TitleID = titleId;
	multiInfo.CurrentDiscNum = (BYTE)titleDiscNum;
	multiInfo.DiscsInSet = (BYTE)titleDiscsInSet;
	for(DWORD x = 0; x < titleDiscsInSet; x++) {
		strncpy_s(multiInfo.DiscPath[x], MAX_PATH,
			multiDiscInfo.DiscPaths[x].c_str(), 
			multiDiscInfo.DiscPaths[x].length());

		multiInfo.DiscIsCon[x] = multiDiscInfo.DiscIsCon[x];
		DebugMsg("ContentItem", "Registering Path:  [Disc %d] %s", x, multiDiscInfo.DiscPaths[x].c_str());
	}

	// Finally lets enable multidisc in our plugin
	if(PluginLoader::getInstance().IsLoaded() == TRUE) {
		if(PluginLoader::getInstance().EnableMultiDisc(&multiInfo) != S_OK) {
			DebugMsg("ContentItem", "Failed to enable multidisc with plugin");
		}
	}
	DebugMsg("ContentItem", "Multidisc Preparation Completed.");
}

void ContentItemNew::ConsolidateTitleUpdates() {
	TitleUpdateManager::getInstance().ConsolidateTitleUpdates(titleId);
	while (TitleUpdateManager::getInstance().isWorking())
	{
		Sleep(50);
	}
}



void ContentItemNew::LaunchGame() {

	// Make sure its not unknown first
	if(fileType == CONTENT_FILE_TYPE_UNKNOWN) return;

	// Update our last played time
//    SYSTEMTIME LocalSysTime;
//    GetLocalTime(&LocalSysTime);
//    SystemTimeToFileTime(&LocalSysTime, (FILETIME*)&titleLastPlayed);

	FSDSql::getInstance().updateRecentlyPlayed(0, GetItemId());

	if(PluginLoader::getInstance().IsLoaded() == TRUE) {
		if(PluginLoader::getInstance().SetScreenshotPath(pluginDataPath.c_str()) != S_OK) {
			DebugMsg("ContentItem", "Failed to set screenshot path with plugin");
		}
	}

	// If its a xex or xbe we can just launch
	if(fileType == CONTENT_FILE_TYPE_XEX ||	fileType == CONTENT_FILE_TYPE_XBE) {
		PrepareForMultiDiscLaunch();
		ConsolidateTitleUpdates();
		string szLaunchPath = itemRoot + itemPath;
		XLaunchNewImage(szLaunchPath.c_str(), 0);
	}

	// Check our content type
	if( contentType != XCONTENTTYPE_ARCADE &&
		contentType != XCONTENTTYPE_XBOXTITLE &&
		contentType != XCONTENTTYPE_XBOX360TITLE &&
		contentType != XCONTENTTYPE_INSTALLED_XBOX360TITLE &&
		contentType != XCONTENTTYPE_GAMEDEMO) {
		DebugMsg("ContentItemNew::LaunchGame", 
			"Invalid content type 0x%08X",
			contentType);
		return;
	}

	// Now we must open our container and launch it
	Xbox360Container container;
	if(container.OpenContainer(itemRoot + itemPath) != S_OK) return;
	container.CloseContainer();
	PrepareForMultiDiscLaunch();
	ConsolidateTitleUpdates();
	container.LaunchGame();
}
DWORD ContentItemNew::AddItemToDatabase() {

	// Basic item info
	CONTENT_ITEM_INFO itemInfo = {
		fileType, contentType, itemTab, itemScanPathId,
	    itemPath, itemDirectory, itemFileName, titleId,
		titleMediaId, titleUserRating, titleIsFavorite, titleDiscNum, titleDiscsInSet };

	itemInfo.ContentHash = contentHash;

	// Display info
	CONTENT_ITEM_DISPLAY_INFO displayInfo = {
		titleName, titleDescription, titleDeveloper,
		titlePublisher,	titleGenre,	titleRating,
		titleRaters, titleReleaseDate };

	// Asset info
	CONTENT_ITEM_ASSET_INFO assetInfo = {
		assetFlags, screenshotCount, thumbnailData, thumbnailSize,
		assetDirectory, backgroundPath, bannerPath, boxartPath,
		previewPath, nxeSlotPath, screenshotPaths };
	
	// Save info
	CONTENT_ITEM_SAVE_INFO saveInfo = {
		contentItemId, itemInfo, displayInfo, assetInfo };

	// Save our item to the database now
	return FSDSql::getInstance().AddContentItem(saveInfo);
}
void ContentItemNew::DeleteItemFromDatabase() {
	
	// First remove this item from the database
	FSDSql::getInstance().DeleteContentItemAssets(contentItemId);
	FSDSql::getInstance().DeleteContentItem(contentItemId);

	// Call item deleted
	ItemDeleted();
}
void ContentItemNew::AddAsset(ITEM_UPDATE_TYPE AssetType, VOID* AssetData, DWORD AssetDataLength, VOID * AssetInfo ) {

	// Figure out our asset type, handle the data and add to our database
	switch(AssetType) {
		case ITEM_UPDATE_TYPE_BANNER:
			FSDSql::getInstance().DeleteContentAsset(contentItemId, ASSET_TYPE_BANNER);
			bannerPath = assetDirectory + "\\banner.dds";
			#ifdef _USING_DXT5
				ConvertImageInMemoryToDXT5(bannerPath, (BYTE*)AssetData, AssetDataLength);
			#else if
				ConvertImageInMemoryToDXT1(bannerPath, (BYTE*)AssetData, AssetDataLength);
			#endif
			assetFlags |= CONTENT_ASSET_BANNER;
			FSDSql::getInstance().AddContentAsset(contentItemId, ASSET_TYPE_BANNER, 
				ASSET_FILE_TYPE_DDS, (VOID*)bannerPath.c_str());			
			break;

		case ITEM_UPDATE_TYPE_BACKGROUND:
			FSDSql::getInstance().DeleteContentAsset(contentItemId, ASSET_TYPE_BACKGROUND);
			backgroundPath = assetDirectory + "\\bg.dds";
			#ifdef _USING_DXT5
				ConvertImageInMemoryToDXT5(backgroundPath, (BYTE*)AssetData, AssetDataLength);
			#else if
				ConvertImageInMemoryToDXT1(backgroundPath, (BYTE*)AssetData, AssetDataLength);
			#endif
			assetFlags |= CONTENT_ASSET_BACKGROUND;
			FSDSql::getInstance().AddContentAsset(contentItemId, ASSET_TYPE_BACKGROUND, 
				ASSET_FILE_TYPE_DDS, (VOID*)backgroundPath.c_str());
			break;

		case ITEM_UPDATE_TYPE_BOXART:
			FSDSql::getInstance().DeleteContentAsset(contentItemId, ASSET_TYPE_BOXCOVER);
			boxartPath = assetDirectory + "\\box.dds";
			#ifdef _USING_DXT5
				ConvertImageInMemoryToDXT5(boxartPath, (BYTE*)AssetData, AssetDataLength);
			#else if
				ConvertImageInMemoryToDXT1(boxartPath, (BYTE*)AssetData, AssetDataLength);
			#endif
			assetFlags |= CONTENT_ASSET_BOXART;
			FSDSql::getInstance().AddContentAsset(contentItemId, ASSET_TYPE_BOXCOVER, 
				ASSET_FILE_TYPE_DDS, (VOID*)boxartPath.c_str());
			break;

		case ITEM_UPDATE_TYPE_PREVIEW:
			FSDSql::getInstance().DeleteContentAsset(contentItemId, ASSET_TYPE_VIDEO);
			assetFlags |= CONTENT_ASSET_PREVIEW;
			previewPath = assetDirectory + "\\preview.wmv";
			FSDSql::getInstance().AddContentAsset(contentItemId, ASSET_TYPE_VIDEO, 
				ASSET_FILE_TYPE_ASX, (VOID*)previewPath.c_str());
			break;

		case ITEM_UPDATE_TYPE_THUMBNAIL:
			FSDSql::getInstance().DeleteContentAsset(contentItemId, ASSET_TYPE_ICON);
			if(thumbnailData != NULL) free(thumbnailData);
			#ifdef _USING_DXT5
				ConvertImageInMemoryToDXT5Buffer((BYTE*)AssetData, AssetDataLength, 
					&thumbnailData, (long*)&thumbnailSize);
			#else if
				ConvertImageInMemoryToDXT1Buffer((BYTE*)AssetData, AssetDataLength, 
					&thumbnailData, (long*)&thumbnailSize);				
			#endif
			assetFlags |= CONTENT_ASSET_THUMBNAIL;
			FSDSql::getInstance().AddContentAsset(contentItemId, 
				ASSET_TYPE_ICON, ASSET_FILE_TYPE_DDS, thumbnailData, thumbnailSize);
			break;

		case ITEM_UPDATE_TYPE_SCREENSHOT:

			CONTENT_SCREENSHOT_INFO * nSSInfo = (CONTENT_SCREENSHOT_INFO*)AssetInfo;

			int nScreenshotIndex = nSSInfo->nScreenshotIndex;
//			int * nScreenshotCount = (int*)AssetInfo;
			// Get our screenshot path and add it
			string screenshotPath = sprintfaA("%s\\screenshot%d.dds", 
				assetDirectory.c_str(), nScreenshotIndex);

			if(nSSInfo->nScreenshotFlag == CONTENT_SCREENSHOT_FLAG_ADD)
				screenshotPaths.push_back(screenshotPath);

			// Now convert it
			#ifdef _USING_DXT5
				ConvertImageInMemoryToDXT5(screenshotPath, (BYTE*)AssetData, AssetDataLength);
			#else if
				ConvertImageInMemoryToDXT1(screenshotPath, (BYTE*)AssetData, AssetDataLength);
			#endif
			assetFlags |= CONTENT_ASSET_SCREENSHOT;

			// Trigger that we updated our item
			if(nSSInfo->nScreenshotFlag == CONTENT_SCREENSHOT_FLAG_ADD) {
				FSDSql::getInstance().AddContentAsset(contentItemId, ASSET_TYPE_SCREENSHOT, 
					ASSET_FILE_TYPE_DDS, (VOID*)screenshotPath.c_str());
			}

			ItemUpdated(ITEM_UPDATE_TYPE_SCREENSHOT);
			break;
	}

	// Call our item updated
	ItemUpdated(AssetType);
}
void ContentItemNew::UpdateContentInfo() {

	CONTENT_ITEM_INFO itemInfo = {
		fileType, contentType, itemTab, itemScanPathId,
	    itemPath, itemDirectory, itemFileName, titleId,
		titleMediaId, titleUserRating, titleDiscNum, titleDiscsInSet };

	FSDSql::getInstance().UpdateContentInfo(contentItemId, itemInfo);
}
void ContentItemNew::UpdateDisplayInfo(BOOL UpdateDB) {

	// Update our display info in the DB
	if(UpdateDB) {
		CONTENT_ITEM_DISPLAY_INFO displayInfo = {
			titleName, titleDescription, titleDeveloper,
			titlePublisher,	titleGenre,	titleRating,
			titleRaters, titleReleaseDate };
		FSDSql::getInstance().UpdateContentItemDisplayInfo(
			contentItemId, displayInfo);
	}

	// Trigger our item display info updated
	ItemUpdated(ITEM_UPDATE_TYPE_DISPLAY_INFO);
}
void ContentItemNew::DeleteItem() {
	DeleteItemFromDatabase();
}
void ContentItemNew::ItemUpdated(DWORD UpdateFlags) {
	if(itemObserver != NULL)
		itemObserver->ItemUpdated(this, UpdateFlags);
}
void ContentItemNew::ItemDeleted() {
	if(itemObserver != NULL)
		itemObserver->ItemDeleted(this);
}