#pragma once

#include "../Settings/Settings.h"
#include "../HTTP/HttpItem.h"
#include "Sql.h"
#include "../SqliteWrapper/KompexSQLiteDatabase.h"
#include "../SqliteWrapper/KompexSQLiteStatement.h"
#include "../SqliteWrapper/KompexSQLiteException.h"

struct SQLTable
{
	string DataBaseName;
	string TableName;
	vector<string> ColumnHeaders;
	vector<map<string, string>> Rows;
	
	SQLTable()
	{
		Clear();
	};
	
	int GetColumnCount()
	{
		return ColumnHeaders.size();	
	};
	int GetRowCount()
	{
		return Rows.size();
	};
	void Clear()
	{
		DataBaseName = "";
		TableName = "";
		ColumnHeaders.clear();
		Rows.clear();
	};
};

struct TitleUpdatePath {
	TitleUpdatePath(ULONGLONG pathId, string deviceID, string dataPath, string cachePath, string contentPath, string root) : 
		pathId(pathId), deviceID(deviceID), dataPath(dataPath), cachePath(cachePath), contentPath(contentPath), root(root){};
	ULONGLONG pathId;
	string deviceID;
	string dataPath; 
	string cachePath; 
	string contentPath; 
	string root;
};

struct SQLTitleUpdate {
	SQLTitleUpdate(ULONGLONG titleUpdateId, wstring updateName, string path, int pathType, string filename, bool isBackup, ULONGLONG isBackupFor, bool isDisabled) : 
		titleUpdateId(titleUpdateId), updateName(updateName), path(path), pathType(pathType), filename(filename), isBackup(isBackup), isBackupFor(isBackupFor), isDisabled(isDisabled) {};
	ULONGLONG titleUpdateId;
	wstring updateName;
	string path;
	int pathType;
	string filename;
	bool isBackup;
	ULONGLONG isBackupFor;
	bool isDisabled;
};

struct SQLTab {
	SQLTab(ULONGLONG tabId, string tabName) : 
		tabId(tabId), tabName(tabName) {};
	ULONGLONG tabId;
	string tabName;
};

struct KaiUser {
	KaiUser(ULONGLONG id, string name, string password, const string& xuid) :
		id(id), name(name), password(password), xuid(xuid) {};
	ULONGLONG id;
	string name;
	string password;
	string xuid;
};

struct KaiContact {
	KaiContact(ULONGLONG contactId, string name, string iconPath) :
		contactId(contactId), name(name), iconPath(iconPath) {};
	ULONGLONG contactId;
	string name;
	string iconPath;
};


struct KaiMessage {
	KaiMessage(ULONGLONG messageId, string sender, string body, int flag, time_t time, ULONGLONG contactId, string contact, string contactIconPath) :
		messageId(messageId), sender(sender), body(body), 
		flag(flag), time(time), contact(contactId, contact, contactIconPath) {};
	ULONGLONG messageId;
	string sender;
	string body;
	int flag;
	time_t time;
	KaiContact contact;
};

typedef struct _HTTP_ITEM {
	DWORD QueueId;
	HTTP_TYPE TypeId;
	string Url;
	string OutputPath;
	BOOL OutputToMemory;
	string Tag;
	DWORD Tag2;
	HTTP_PRIORITY Priority;
	ULONGLONG TimeStamp;
	DWORD Expiration;
	string ContentHash;
} HTTP_ITEM;

typedef struct _CONTENT_ITEM_INFO {
	DWORD  FileType;
	DWORD  ContentType;
	DWORD  ItemTab;
	DWORD  ItemScanPathId;
	string ItemPath;
	string ItemDirectory;
	string ItemFileName;
	DWORD  TitleId;
	DWORD  TitleMediaId;
	int    TitleUserRating;
	BOOL   TitleIsFavorite;
	DWORD  TitleDiscNum;
	DWORD  TitleDiscsInSet;
	string TitleLastPlayedDate;
	ULONGLONG TitleLastPlayedOrder;
	string ContentHash;
} CONTENT_ITEM_INFO, *PCONTENT_ITEM_INFO;

typedef struct _CONTENT_ITEM_DISPLAY_INFO {
	wstring TitleName;
	wstring TitleDescription;
	wstring TitleDeveloper;
	wstring TitlePublisher;
	wstring TitleGenre;
	wstring TitleRating;
	wstring TitleRaters;
	wstring TitleReleaseDate;
} CONTENT_ITEM_DISPLAY_INFO, *PCONTENT_ITEM_DISPLAY_INFO;

typedef struct _CONTENT_ITEM_ASSET_INFO {
	DWORD   AssetFlags;
	DWORD   ScreenshotCount;
	BYTE*   ThumbnailData;
	DWORD   ThumbnailSize;
	string  AssetDirectory;
	string  BackgroundPath;
	string  BannerPath;
	string  BoxartPath;
	string  PreviewPath;
	string  NxeSlotPath;
	vector<string> ScreenshotPaths;
} CONTENT_ITEM_ASSET_INFO, *PCONTENT_ITEM_ASSET_INFO;

typedef struct _CONTENT_ITEM_HTTP_QUEUE {
	DWORD	httpItemCount;
	vector<HTTP_ITEM> httpItemQueue;
} CONTENT_ITEM_HTTP_QUEUE, *PCONTENT_ITEM_HTTP_QUEUE;

typedef struct {
	ULONGLONG contentId;
	wstring title;
	bool bIsMounted;
} RecentlyPlayed;

typedef struct _MULTI_DISC_INFO {
	DWORD		   DiscsInSet;
	DWORD		   DiscsFound;
	bool		   DiscIsCon[5];
	string		   DiscPaths[5];
} MULTI_DISC_INFO, *PMULTI_DISC_INFO;

typedef struct {
	string smbPath;
	string login;
	string encryptedPassword;
} SambaCredentialsEncrypted;

// Full struct for loading from DB and Saving new to DB
typedef struct _CONTENT_ITEM_LOAD_INFO {
	DWORD					  ContentId;
	CONTENT_ITEM_INFO         ItemInfo;
	CONTENT_ITEM_DISPLAY_INFO DisplayInfo;
	CONTENT_ITEM_ASSET_INFO   AssetInfo;
	CONTENT_ITEM_HTTP_QUEUE	  HttpQueue;
} CONTENT_ITEM_LOAD_INFO, *PCONTENT_ITEM_LOAD_INFO;
typedef CONTENT_ITEM_LOAD_INFO  CONTENT_ITEM_SAVE_INFO;
typedef CONTENT_ITEM_LOAD_INFO* PCONTENT_ITEM_SAVE_INFO;

// THESE ENUMS will eventually go away once Mattie writes the ContentManager and AssetManager..
// Instead, they should be loaded from the database and not hardcoded like this

typedef enum _ASSET_TYPE {
	ASSET_TYPE_ICON       = 0,
	ASSET_TYPE_BOXCOVER   = 1,
	ASSET_TYPE_BACKGROUND = 2,
	ASSET_TYPE_BANNER     = 3,
	ASSET_TYPE_SCREENSHOT = 4,
	ASSET_TYPE_VIDEO      = 5,
	ASSET_TYPE_NXE_SLOT   = 6
} ASSET_TYPE;

typedef enum _ASSET_FILE_TYPE {
	ASSET_FILE_TYPE_JPG = 0,
	ASSET_FILE_TYPE_PNG = 1,
	ASSET_FILE_TYPE_DDS = 2,
	ASSET_FILE_TYPE_ASX = 3,
	ASSET_FILE_TYPE_WMV = 4,
} ASSET_FILE_TYPE;

class FSDSql 
{
public:
	static FSDSql& getInstance()
	{
		static FSDSql singleton;
		return singleton;
	}

	bool GetSQLTable(string tableName, SQLTable* table);
	int GetSQLTableNames(vector<string>* tableNames);

//	bool initDatabase(string db);  // open the database, and make sure all the schema is there
	void closeDatabase();  // should figure out a way to call this when we are leaving the dash, but probably not required
	DWORD AddContentItem(CONTENT_ITEM_SAVE_INFO& SaveInfo);
	void LoadContentItemsFromDB(vector<CONTENT_ITEM_LOAD_INFO>& ItemVector);
	void LoadContentItemFromContentId(DWORD ContentId, CONTENT_ITEM_LOAD_INFO& ItemInfo);
	BOOL ItemPathExistsInDB(string ItemPath, ScanPath * ScanPath);
	BOOL ItemTitleIdExistsInDB(DWORD TitleId);
	void UpdateContentInfo(DWORD ContentId, CONTENT_ITEM_INFO& ItemInfo);
	void UpdateContentItemDisplayInfo(DWORD ContentId, CONTENT_ITEM_DISPLAY_INFO& DisplayInfo);
	void DeleteContentAsset(DWORD ContentId, ASSET_TYPE AssetType);
	void AddContentAsset(DWORD ContentId, ASSET_TYPE AssetType, ASSET_FILE_TYPE AssetFileType, VOID* AssetData, DWORD AssetDataLength = 0);
	void LoadContentAssets(DWORD ContentId, CONTENT_ITEM_ASSET_INFO& AssetInfo);
	void DeleteContentItemAssets(DWORD ContentId);
	void DeleteContentItem(DWORD ContentId);
	void LoadMultiDiscInfo(DWORD TitleId, DWORD DiscsInSet, MULTI_DISC_INFO& MultiDiscInfo);
	void cleanBadTUPaths();
	void addMountedDevice(const string& root, const string& deviceId);
	ULONGLONG addScanPath(const string& path, const string& deviceId, ULONGLONG retailTabId, ULONGLONG devkitTabId, int depth);
	
	ScanPath* getScanPath(const string& path, const string& deviceId);
	string getSerialByScanPathId(DWORD nScanPathId);
	void deleteScanPath(ULONGLONG scanPathId);
	void updateScanPath(const string& path, const string& deviceId, ULONGLONG scanPathId, ULONGLONG retailTabId, ULONGLONG devkitTabId, int depth);

	vector<ScanPath> getScanPaths();

	DWORD HttpQueueAddDownload( HttpItem * ref);
	DWORD HttpQueueRemoveDownload( HttpItem * ref );
	DWORD HttpQueueRestoreDownload( DWORD dwUrlID, string contentHash, vector<HTTP_ITEM>& items );
	DWORD LoadContentHttpQueue( string contentHash, CONTENT_ITEM_HTTP_QUEUE& httpQueue );

	ULONGLONG addKaiUser(const string& name, const string& password);
	ULONGLONG getKaiUserIdByName(const string& name);
	KaiUser* getKaiUserIdByXuid(const string& xuid);
	void setKaiUserPassword(ULONGLONG id, const string& password);
	vector<KaiUser*> getKaiUsers();
	void deleteKaiUser(ULONGLONG id);

	void addProfileXuid(const string& xuid, ULONGLONG optionalKaiUserId = 0);

	ULONGLONG addKaiContact(const string& name, const string& path);
	vector<KaiContact*> getKaiContactsForUserId(ULONGLONG userId);
	void deleteKaiContact(ULONGLONG id);

	void joinKaiUserToContact(ULONGLONG userId, ULONGLONG contactId);

	ULONGLONG addKaiMessage(const string& sender, const string& message, int flag, time_t time, ULONGLONG userId, ULONGLONG contactId);
	vector<KaiMessage*> getKaiMessagesForUserId(ULONGLONG userId);

	void deleteKaiMessage(ULONGLONG id);
	void updateKaiMessageFlag(ULONGLONG id, int flag);

	void updateFavorite(int playerId, ULONGLONG contentId, bool isFavorite);

	void updateRecentlyPlayed(int playerId, ULONGLONG contentId);
	bool getRecentlyPlayed(int playerId, ULONGLONG contentId, PULONGLONG order, SYSTEMTIME* st);  // return true if successful
	vector<RecentlyPlayed> getMostRecentlyPlayedIds(int playerId, int max_count);

	void updateUserRating(int playerId, ULONGLONG contentId, int rating);
	int getUserRating(int playerId, ULONGLONG contentId);
	
	void LoadContentFromDB();
	void cleanDVDContent();

	void addTitleUpdatePath(const string& deviceId, const string& dataPath, const string& cachePath, const string& contentPath);
	ULONGLONG addTitleUpdate(DWORD titleId, const wstring& name, const string& filename, const string& addPath, int isBackup, ULONGLONG isBackupFor, ULONGLONG titleUpdatePathId, int titleUpdatePathType);
	SQLTitleUpdate* getTitleUpdateByUpdateID( ULONGLONG titleUpdateId);
	void deleteTitleUpdate(ULONGLONG titleUpdateId);
	TitleUpdatePath* getTitleUpdatePathByUpdateID( ULONGLONG titleUpdateId);
	TitleUpdatePath* getTitleUpdatePathByPathID( ULONGLONG TitleUpdatePathId);
	ULONGLONG getTitleUpdateID(DWORD titleId, const wstring& name, const string& filename, const string& addPath, int isBackup, ULONGLONG isBackupFor, ULONGLONG titleUpdatePathId, int titleUpdatePathType);
	void updateTitleDisabled(int playerId, ULONGLONG TitleUpdateId, bool isDisabled);
	vector<TitleUpdatePath*> getTitleUpdatePaths();
	vector<SQLTitleUpdate*> getTitleUpdatesForTitleId(int playerId, DWORD titleId);

	vector<SQLTab*> getTabs();

	void writeGlobalSettings(const string& videoPath, const string& musicPath, const string& dvdCopy360Path,
		const string& dvdCopyOrigPath, const string& weatherLocation, int hOverscan, int vOverscan,
		int hDisplacement, int vDisplacement, int bLetterbox, int ftpPort,
		const string& ftpUser, const string& ftpPassword, int fanSpeed, int LEDformat,
		int timeFormat, int dateFormat, int bNTPOnStartup, int bUpdateXEXNotice,
		int bUpdateSkinNotice, int bUpdatePluginNotice, int bUpdateOnBeta, int installerState, 
		const string& fsdUsername, const string& fsdPassword, int fsdUseLogin, const string& locale,
		int bShowBackground, int nGameListVisual, int bShowTempStatus, int bShowDiskStatus, int bUseCelsius,
		int bShowIP, int bShowStatus, int bShowAvatar, int bShowScreenshots, 
		int bKaiAutoLogin, int bKaiLinkProfile, int bKaiNoticeOnline, int bKaiNoticeOffline,
		int bKaiNoticeInvite, int bKaiNoticeContactPM, int bKaiNoticeOpponentPM, const string& kaiServerIp,
		const string& kaiUsername, const string& kaiPassword, bool bFtpServerOn, bool bHttpServerOn, 
		int bShowStatusAtBoot, int downloadIcon, int downloadBoxart, int downloadBanner, int downloadBackground, int downloadVideo,
		int downloadScreenshots, int maxScreenshots, int httpDisableSecurity, const string& httpUsername, const string& httpPassword,
		int glTransition, bool bSambaClientOn, const string& weatherLocationName, int ResumeAssetDownload, int DisableAutoScan, int WeatherUnits,
		const string& smbClientWorkgroup, bool bSambaServerOn, const string& hostname, int FMAdvancedMode,
		int CoverPoll, const string& CoverPass, const string& CoverUser, bool CoverCheck, int CoverQuality );

	bool readGlobalSettings(string& videoPath, string& musicPath, string& dvdCopy360Path,
		string& dvdCopyOrigPath, string& weatherLocation, int& hOverscan, int& vOverscan,
		int& hDisplacement, int& vDisplacement, int& bLetterbox, int& ftpPort,
		string& ftpUser, string& ftpPassword, int& fanSpeed, int& LEDformat,
		int& timeFormat, int& dateFormat, int& bNTPOnStartup, int& bUpdateXEXNotice,
		int& bUpdateSkinNotice, int& bUpdatePluginNotice, int& bUpdateOnBeta, int& installerState,
		string& fsdUsername, string& fsdPassword, int& fsdUseLogin, string& locale,
		int& bShowBackground, int& nGameListVisual, int& bShowTempStatus, int& bShowDiskStatus, int& bUseCelsius,
		int& bShowIP, int& bShowStatus, int& bShowAvatar,int& bShowScreenshots,
		int& bKaiAutoLogin, int& bKaiLinkProfile, int& bKaiNoticeOnline, int& bKaiNoticeOffline,
		int& bKaiNoticeInvite, int& bKaiNoticeContactPM, int& bKaiNoticeOpponentPM, string& kaiServerIp,
		string& kaiUsername, string& kaiPassword, bool& bFtpServerOn, bool& bHttpServerOn,
		int& bShowStatusAtBoot, int& downloadIcon, int& downloadBoxart, int& downloadBanner, int& downloadBackground, int& downloadVideo,
		int& downloadScreenshots, int& maxScreenshots, int& httpDisableSecurity, string& httpUsername, string& httpPassword,
		int& glTransition, bool& bSambaClientOn, string& weatherLocationName, int& ResumeAssetDownload, int& DisableAutoScan, int& WeatherUnits,
		string& smbClientWorkgroup, bool& bSambaServerOn, string& hostname, int& FMAdvancedMode, int& CoverPoll, string& CoverPass, string& CoverUser, bool& CoverCheck, 
		int& CoverQuality ); 

	void writeUserSettings(const string& skin);
	bool readUserSettings(string& skin);

	//
	void getGameListSort(int tabId, int playerId, int& sortType, int& sortDirection, int& subtitle, bool& bOnlyFavorites);
	void setGameListSort(int tabId, int playerId, int sortType, int sortDirection, int subtitle, bool bOnlyFavorites);

	vector<SambaCredentialsEncrypted> getAllSambaClientCredentials(int playerId);
	SambaCredentialsEncrypted getSambaClientCredentials(int playerId, string path);
	void setSambaClientCredential(int playerId, string path, string login, string encryptedPassword);
	void deleteSambaClientCredential(int playerId, string path);

private:
	FSDSql();
	~FSDSql() {}
	FSDSql(const FSDSql&);                 // Prevent copy-construction
	FSDSql& operator=(const FSDSql&);      // Prevent assignment
	CRITICAL_SECTION lock;

	Kompex::SQLiteDatabase *getDatabase();

	string fixString(string& in);
	wstring fixStringW(wstring& in);
	map<DWORD, Kompex::SQLiteDatabase *> dbMap;
	bool bFirst;

	void execute(const char* stmt);
	bool getValueString(const char* stmt, string& value);
	

//	Kompex::SQLiteStatement* pStmtGetReleaseDate;

	void buildQueries();
	void createSchema(long currentSchemaVersion, long currentSettingsSchemaVersion);

//	string createSchemaVersion;
	string createAssetTypes;
	string createAssetFileTypes;
	string createTabs;
//	string createScanPahts;
	string createContentTypes;
	string createFavorites;
	string createRecentlyPlayedTitles;
	string createUserRatings;
	string createAssets;
	string createContentItems;

	string createContentItemsIndex;

	// version 2
	// DROP TABLE SchemaVersion
//	string createTitleUpdatePaths;
//	string createTitleUpdates;
//	string createKaiUsers;
	string createKaiMessages;
	string createKaiContacts;
	string createKaiUserToContacts;

	// version 3
	string createProfileXuids;
	string createKaiUsersV3;

	//version 4
//	string createTitleUpdatePathsV4;
	string createTitleUpdateDisables;

	//version 5
	// DROP TABLE ScanPaths
	string createScanPathsV5;

	//version 6
	string createContentItemsV6;

	//version 7
	string createTitleUpdatePathsV7;
	string createTitleUpdatesV7;

	//version 8
	string createHttpQueueTable;

	//Settings version 1
	string createGlobalSettings;

	//Settings version 2
	string createGlobalSettingsV2_temp;		// temp because it's just a column removal from a previous version, so needs the ALTER TABLE workaround
	string createUserSettings;

	//Settings version 3
	string createGameListSorts;
	string createGameListSortTypes;
	string createGameListSortDirections;

	//Settings version 4 was inlined (go see the code)

	//Settings version 5
	string alterGlobalSettings;

	//Settings version 6 was inlined (go see the code)

	//Settings version 7
	string alterGlobalSettingsV2;

	//Settings version 8 was inlined (go see the code)

	//Settings version 13
	string createSambaClientCredentials;

	void DumpTable(string tableName);
};