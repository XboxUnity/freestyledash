#include "stdafx.h"
#include "FSDSql.h"
#include "../Debug/Debug.h"
#include "../Generic/xboxtools.h"
#include "../Managers/Drives/DrivesManager.h"
#include "../HTTP/HttpItem.h"

#include "../ContentList/ContentManager.h"

#pragma warning(disable:4652)		// C++ exception handling
#define FSD_LOCK  EnterCriticalSection(&lock)
#define FSD_UNLOCK LeaveCriticalSection(&lock)
//#define FSD_LOCK 
//#define FSD_UNLOCK 

FSDSql::FSDSql() //, pDatabase(NULL)
{
	DebugMsg("FSDSql", "In Constructor");
	InitializeCriticalSection(&lock);
	bFirst = true;
	buildQueries();
}

Kompex::SQLiteDatabase *FSDSql::getDatabase() {
	DWORD tid =	GetCurrentThreadId();
	
	EnterCriticalSection(&lock);  // DONT USE THE FSDLOCK MACRO HERE, we are protecting the map, not the db

	map<DWORD, Kompex::SQLiteDatabase *>::iterator i = dbMap.find(tid);
	if (i != dbMap.end()) {
		LeaveCriticalSection(&lock);
		return i->second;
	}

//	string dbPath = SETTINGS::getInstance().getDataPath();
	RecursiveMkdir("Game:\\Data\\Databases");

	string dbPath = "Game:\\Data\\Databases\\fsd2data.db";

	if (bFirst) {
		if(false){	// this is here so I can put a breakpoint here to delete the database
			unlink(dbPath.c_str());
		}
	}

	Kompex::SQLiteDatabase *pd = new Kompex::SQLiteDatabase(dbPath.c_str(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
	sqlite3_stmt*  sqlStmt = NULL;	
	int ret = sqlite3_prepare_v2(pd->GetDatabaseHandle(), "PRAGMA main.journal_mode = OFF;", -1, &sqlStmt, NULL); 
	if( ret == SQLITE_OK ) { 
		sqlite3_step(sqlStmt); 
		const unsigned char* colValue = sqlite3_column_text(sqlStmt, 0);  //<----- returns "off" 
		DebugMsg("FSDSql", "Journal is %s", colValue);
		sqlite3_finalize(sqlStmt); 
	} 

	Kompex::SQLiteStatement pStmt(pd);
	try {
		pStmt.SqlStatement("ATTACH 'Game:\\Data\\Databases\\fsd2settings.db' AS Settings");
		pStmt.SqlStatement("PRAGMA Settings.journal_mode = off;");
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't ATTACH", exception.GetString().c_str());
	} 


	dbMap[tid] = pd;

	if (bFirst) {
		long version = 0;
		int ret = sqlite3_prepare_v2(pd->GetDatabaseHandle(), "PRAGMA main.user_version;", -1, &sqlStmt, NULL); 
		if( ret == SQLITE_OK ) { 
			sqlite3_step(sqlStmt); 
			version = sqlite3_column_int(sqlStmt, 0); 
			DebugMsg("FSDSql", "version is %d", version);
			sqlite3_finalize(sqlStmt); 
		} 

		pStmt.Sql("PRAGMA Settings.user_version;");
		pStmt.FetchRow();
		long settingsVersion = pStmt.GetColumnInt(0);
		DebugMsg("FSDSql", "version of Settings schema is %d", settingsVersion);
		pStmt.FreeQuery();

		createSchema(version, settingsVersion);
		cleanDVDContent();
		bFirst = false;
	}

	LeaveCriticalSection(&lock);  // DONT USE THE FSDLOCK MACRO HERE, we are protecting the map, not the db

	return pd;
}

void FSDSql::cleanDVDContent() {
	FSD_LOCK;

	Kompex::SQLiteDatabase *pDatabase = getDatabase();

	Kompex::SQLiteStatement pStmt(pDatabase);
	ostringstream q;
	q << "DELETE FROM ContentItems WHERE ContentItemPath = 'dvd:\\'";

	try {
		pStmt.SqlStatement(q.str());
		DebugMsg("FSDSql", "DELETED CONTENT");
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't delete dvd content", exception.GetString().c_str());
	} 

	FSD_UNLOCK;
}


void FSDSql::closeDatabase() {
	FSD_LOCK;

	int i = 0;
	for(map<DWORD, Kompex::SQLiteDatabase *>::const_iterator it = dbMap.begin(); it != dbMap.end(); ++it) {
		DebugMsg("FSDSQL","closing %d", i++);
		it->second->Close();
		delete it->second;
	}
	dbMap.clear();

	DebugMsg("FSDSql", "Finished closeDatabase");
	FSD_UNLOCK;
}

// returns true if fetched a value, false if it was null
bool FSDSql::getValueString(const char* stmt, string& value) {
	bool r = false; 
	struct sqlite3_stmt *mStatement;
	sqlite3* db;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	db = pDatabase->GetDatabaseHandle();
//	DebugMsg("FSDSql", "db handle is [%X]", db);
//	DebugMsg("FSDSql", "about to prepare [%s]", stmt);
	if(sqlite3_prepare_v2(db, stmt, -1, &mStatement, 0) != SQLITE_OK) {
//		DebugMsg("FSDSql", "getValueString Couldn't prepare");
//		DebugMsg("FSDSql", "getValueString Couldn't prepare [%s]", sqlite3_errmsg(db));
	}
	else {
		if(!mStatement) {
			DebugMsg("FSDSql", "getValueString Prepare failed");
		}
		sqlite3_step(mStatement);

		std::stringstream ss;
		const unsigned char *chars = sqlite3_column_text(mStatement, 0);
		if (chars == NULL) {  
			value = "";
		}
		else {
			ss << chars;
			value = ss.str();
//			DebugMsg("FSDSql", "getValueString returning true with [%s]", value.c_str());
			r = true;
		}

//		DebugMsg("FSDSql", "Step returned [%d]", retval);
		sqlite3_finalize(mStatement);
	}
//	DebugMsg("FSDSql", "getValueString returning false");
	return r;
}

DWORD FSDSql::AddContentItem(CONTENT_ITEM_SAVE_INFO& SaveInfo) {
	
	// Lock
	FSD_LOCK;

	// Get our db and statement
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Get our many items we currently have
	ostringstream q; DWORD itemCount = 0;
	q << "SELECT MAX(ContentItemId) max FROM ContentItems";
	try {
		itemCount = (DWORD)pStmt.SqlAggregateFuncResult(q.str());
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't get MAX ContentId", 
			exception.GetString().c_str());
	}

	// Setup our new query
	q.str("");
	q << "INSERT INTO ContentItems (";
	q << "  ContentItemId,          ContentItemScanPathId,  ContentItemFileType,   ContentItemContentType,";
	q << "  ContentItemTab,         ContentItemPath,        ContentItemDirectory,  ContentItemFileName,";
	q << "  ContentItemTitleId,     ContentItemMediaId,     ContentItemDiscNum,    ContentItemDiscsInSet,";
	q << "  ContentItemName,        ContentItemDescription, ContentItemDeveloper,  ContentItemPublisher,";
	q << "  ContentItemGenre,       ContentItemRating,      ContentItemRaters,     ContentItemReleaseDate, ";
	q << "  ContentItemHash ";
	q << "  ) VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )";

	// Set our id for this item now that we have a count
	DWORD contentItemId = 0;

	try {
		DWORD tempId = contentItemId = itemCount + 1;
		pStmt.Sql(q.str());
		pStmt.BindInt(1, tempId);
		pStmt.BindInt(2, SaveInfo.ItemInfo.ItemScanPathId);
		pStmt.BindInt(3, SaveInfo.ItemInfo.FileType);
		pStmt.BindInt(4, SaveInfo.ItemInfo.ContentType);
		pStmt.BindInt(5, SaveInfo.ItemInfo.ItemTab);
		pStmt.BindString(6, SaveInfo.ItemInfo.ItemPath);
		pStmt.BindString(7, SaveInfo.ItemInfo.ItemDirectory);
		pStmt.BindString(8, SaveInfo.ItemInfo.ItemFileName);		
		pStmt.BindInt(9, SaveInfo.ItemInfo.TitleId);
		pStmt.BindInt(10, SaveInfo.ItemInfo.TitleMediaId);
		pStmt.BindInt(11, SaveInfo.ItemInfo.TitleDiscNum);
		pStmt.BindInt(12, SaveInfo.ItemInfo.TitleDiscsInSet);
		pStmt.BindString(13, ToUtf8(SaveInfo.DisplayInfo.TitleName));
		pStmt.BindString(14, ToUtf8(SaveInfo.DisplayInfo.TitleDescription));
		pStmt.BindString(15, ToUtf8(SaveInfo.DisplayInfo.TitleDeveloper));
		pStmt.BindString(16, ToUtf8(SaveInfo.DisplayInfo.TitlePublisher));
		pStmt.BindString(17, ToUtf8(SaveInfo.DisplayInfo.TitleGenre));
		pStmt.BindString(18, ToUtf8(SaveInfo.DisplayInfo.TitleRating));
		pStmt.BindString(19, ToUtf8(SaveInfo.DisplayInfo.TitleRaters));
		pStmt.BindString(20, ToUtf8(SaveInfo.DisplayInfo.TitleReleaseDate));
		pStmt.BindString(21, SaveInfo.ItemInfo.ContentHash);
		
		pStmt.ExecuteAndFree();
		contentItemId = tempId;
	}
	catch(Kompex::SQLiteException) {

		DebugMsg("FSDSql", "INSERT failed, content was probably there already");
	}

	// Unlock
	FSD_UNLOCK;

	// Return our Item ID
	return contentItemId;
}

void FSDSql::LoadContentItemFromContentId(DWORD ContentId, CONTENT_ITEM_LOAD_INFO& ItemInfo) {

	// Lock
	FSD_LOCK;

	// Get our DB
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	string gamerProfile = GetGamerId(0);

	// Setup our query stream
	ostringstream q;
	q << "SELECT ContentItemId,          ContentItemScanPathId,  ContentItemFileType,";
	q << "       ContentItemContentType, ContentItemTab,         ContentItemPath,";
	q << "       ContentItemDirectory,   ContentItemFileName,    ContentItemTitleId,"; 
	q << "       ContentItemMediaId,     ContentItemDiscNum,     ContentItemDiscsInSet,";
	q << "       ContentItemName,        ContentItemDescription, ContentItemDeveloper,";
	q << "       ContentItemPublisher,   ContentItemGenre,       ContentItemRating,";
	q << "       ContentItemRaters,      ContentItemReleaseDate,";
	q << "		 RecentlyPlayedTitleDateTime, RecentlyPlayedTitleOrder, "; 
	q << "		 UserRatingRating, ";
	q << "		 FavoriteId, "; //23
	q << "		 ScanPathDeviceId, "; //24
	q << "		 ContentItemHash ";

	q << " FROM   ContentItems, ScanPaths, MountedDevices";
	q << " LEFT OUTER JOIN RecentlyPlayedTitles ON ContentItemId = RecentlyPlayedTitleContentId AND RecentlyPlayedTitleGamerProfile = '" << gamerProfile << "'";
	q << " LEFT OUTER JOIN UserRatings ON ContentItemId = UserRatingContentId AND UserRatingGamerProfile = '" << gamerProfile << "'";
	q << " LEFT OUTER JOIN Favorites ON ContentItemId = FavoriteContentId AND FavoriteGamerProfile = '" << gamerProfile << "'";

	q << " WHERE ContentItemScanPathId = ScanPathId ";
	q << " AND ContentItemId = ?";


	try	
	{

		// Create our item
		CONTENT_ITEM_LOAD_INFO loadInfo;
		ZeroMemory(&loadInfo, sizeof(CONTENT_ITEM_LOAD_INFO));
		
		// Set our statement
		pStmt.Sql(q.str());
		pStmt.BindInt(1, ContentId);

		if(!pStmt.FetchRow())
			return;

		// Make sure this item exists first
		string szSerial = pStmt.GetColumnString(24);
		string szRootPath = DrivesManager::getInstance().getRootPathBySerialNumberStr(szSerial);
		string szFullPath = szRootPath + pStmt.GetColumnString(5);
	
		// Now load info into it
		loadInfo.ContentId     	          = pStmt.GetColumnInt(0);

		// Item Info
		loadInfo.ItemInfo.ItemScanPathId  = pStmt.GetColumnInt(1);
		loadInfo.ItemInfo.FileType		  = (CONTENT_FILE_TYPE)pStmt.GetColumnInt(2);
		loadInfo.ItemInfo.ContentType	  = pStmt.GetColumnInt(3);
		loadInfo.ItemInfo.ItemTab		  = (CONTENT_ITEM_TAB)pStmt.GetColumnInt(4);
		loadInfo.ItemInfo.ItemPath		  = pStmt.GetColumnString(5);
		loadInfo.ItemInfo.ItemDirectory	  = pStmt.GetColumnString(6);
		loadInfo.ItemInfo.ItemFileName	  = pStmt.GetColumnString(7);
		loadInfo.ItemInfo.TitleId		  = pStmt.GetColumnInt(8);
		loadInfo.ItemInfo.TitleMediaId	  = pStmt.GetColumnInt(9);
		loadInfo.ItemInfo.TitleDiscNum	  = pStmt.GetColumnInt(10);
		loadInfo.ItemInfo.TitleDiscsInSet = pStmt.GetColumnInt(11);
		loadInfo.ItemInfo.ContentHash	  = pStmt.GetColumnString(25);

		// Display Info
		loadInfo.DisplayInfo.TitleName		  = FromUtf8(pStmt.GetColumnString(12));
		loadInfo.DisplayInfo.TitleDescription = FromUtf8(pStmt.GetColumnString(13));
		loadInfo.DisplayInfo.TitleDeveloper	  = FromUtf8(pStmt.GetColumnString(14));
		loadInfo.DisplayInfo.TitlePublisher	  = FromUtf8(pStmt.GetColumnString(15));
		loadInfo.DisplayInfo.TitleGenre		  = FromUtf8(pStmt.GetColumnString(16));
		loadInfo.DisplayInfo.TitleRating	  = FromUtf8(pStmt.GetColumnString(17));
		loadInfo.DisplayInfo.TitleRaters	  = FromUtf8(pStmt.GetColumnString(18));
		loadInfo.DisplayInfo.TitleReleaseDate = FromUtf8(pStmt.GetColumnString(19));

		long long playedDate =  pStmt.GetColumnInt64(20);
		if (playedDate == 0) {
			loadInfo.ItemInfo.TitleLastPlayedDate = "Never";
		}
		else {
			SYSTEMTIME st;
			time_t time = playedDate;  //pStmt.GetColumnInt64(0);
			UnixTimeToSystemTime(&time, &st);
			loadInfo.ItemInfo.TitleLastPlayedDate = wstrtostr(SystemTimeToLocalString(st));
		}

		loadInfo.ItemInfo.TitleLastPlayedOrder = pStmt.GetColumnInt64(21);
		loadInfo.ItemInfo.TitleUserRating = pStmt.GetColumnInt(22);
		int favorite = pStmt.GetColumnInt(23);
		if (favorite) {
			loadInfo.ItemInfo.TitleIsFavorite = true;
		}
		else {
			loadInfo.ItemInfo.TitleIsFavorite = false;

		}

		// Now we can go ahead and add this item
		ItemInfo = loadInfo;

	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't fetch content: %s", 
			exception.GetString().c_str());
	}

	// Free our query
	pStmt.FreeQuery();

	// Unlock
	FSD_UNLOCK;

	// Load Assets and HTTP Queue
	LoadContentAssets(ContentId, ItemInfo.AssetInfo);
	LoadContentHttpQueue(ItemInfo.ItemInfo.ContentHash, ItemInfo.HttpQueue);
}

void FSDSql::LoadContentItemsFromDB(vector<CONTENT_ITEM_LOAD_INFO>& ItemVector) {

	// Lock
	FSD_LOCK;

	// Get our DB
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	string gamerProfile = GetGamerId(0);

	ItemVector.clear();

	// Setup our query stream
	ostringstream q;
	q << "SELECT ContentItemId,          ContentItemScanPathId,  ContentItemFileType,";
	q << "       ContentItemContentType, ContentItemTab,         ContentItemPath,";
	q << "       ContentItemDirectory,   ContentItemFileName,    ContentItemTitleId,"; 
	q << "       ContentItemMediaId,     ContentItemDiscNum,     ContentItemDiscsInSet,";
	q << "       ContentItemName,        ContentItemDescription, ContentItemDeveloper,";
	q << "       ContentItemPublisher,   ContentItemGenre,       ContentItemRating,";
	q << "       ContentItemRaters,      ContentItemReleaseDate, ContentItemHash, ";
	q << "		 RecentlyPlayedTitleDateTime, RecentlyPlayedTitleOrder, "; 
	q << "		 UserRatingRating, ";
	q << "		 FavoriteId, "; //24
	q << "		 ScanPathDeviceId "; //25

	q << " FROM   ContentItems, ScanPaths, MountedDevices";
	q << " LEFT OUTER JOIN RecentlyPlayedTitles ON ContentItemId = RecentlyPlayedTitleContentId AND RecentlyPlayedTitleGamerProfile = '" << gamerProfile << "'";
	q << " LEFT OUTER JOIN UserRatings ON ContentItemId = UserRatingContentId AND UserRatingGamerProfile = '" << gamerProfile << "'";
	q << " LEFT OUTER JOIN Favorites ON ContentItemId = FavoriteContentId AND FavoriteGamerProfile = '" << gamerProfile << "'";

	q << " WHERE ContentItemScanPathId = ScanPathId ";
	q << " AND ScanPathDeviceId = MountedDeviceDeviceId ";

	try	
	{
		// Set our statement
		pStmt.Sql(q.str());

		// Now loop and fetch each item		
		while (pStmt.FetchRow()) {

			// Make sure this item exists first
			string szSerial = pStmt.GetColumnString(25);
			string szRootPath = DrivesManager::getInstance().getRootPathBySerialNumberStr(szSerial);
			string szFullPath = szRootPath + pStmt.GetColumnString(5);
			if(!FileExists(szFullPath))
				continue;
//			if(!FileExists(szFullPath))
//				continue;
			
			// Create our item
			CONTENT_ITEM_LOAD_INFO loadInfo;
			ZeroMemory(&loadInfo, sizeof(CONTENT_ITEM_LOAD_INFO));

			// Now load info into it
			loadInfo.ContentId     	          = pStmt.GetColumnInt(0);

			// Item Info
			loadInfo.ItemInfo.ItemScanPathId  = pStmt.GetColumnInt(1);
			loadInfo.ItemInfo.FileType		  = (CONTENT_FILE_TYPE)pStmt.GetColumnInt(2);
			loadInfo.ItemInfo.ContentType	  = pStmt.GetColumnInt(3);
			loadInfo.ItemInfo.ItemTab		  = (CONTENT_ITEM_TAB)pStmt.GetColumnInt(4);
			loadInfo.ItemInfo.ItemPath		  = pStmt.GetColumnString(5);
			loadInfo.ItemInfo.ItemDirectory	  = pStmt.GetColumnString(6);
			loadInfo.ItemInfo.ItemFileName	  = pStmt.GetColumnString(7);
			loadInfo.ItemInfo.TitleId		  = pStmt.GetColumnInt(8);
			loadInfo.ItemInfo.TitleMediaId	  = pStmt.GetColumnInt(9);
			loadInfo.ItemInfo.TitleDiscNum	  = pStmt.GetColumnInt(10);
			loadInfo.ItemInfo.TitleDiscsInSet = pStmt.GetColumnInt(11);
			loadInfo.ItemInfo.ContentHash	  = pStmt.GetColumnString(20);

			// Display Info
			loadInfo.DisplayInfo.TitleName		  = FromUtf8(pStmt.GetColumnString(12));
			loadInfo.DisplayInfo.TitleDescription = FromUtf8(pStmt.GetColumnString(13));
			loadInfo.DisplayInfo.TitleDeveloper	  = FromUtf8(pStmt.GetColumnString(14));
			loadInfo.DisplayInfo.TitlePublisher	  = FromUtf8(pStmt.GetColumnString(15));
			loadInfo.DisplayInfo.TitleGenre		  = FromUtf8(pStmt.GetColumnString(16));
			loadInfo.DisplayInfo.TitleRating	  = FromUtf8(pStmt.GetColumnString(17));
			loadInfo.DisplayInfo.TitleRaters	  = FromUtf8(pStmt.GetColumnString(18));
			loadInfo.DisplayInfo.TitleReleaseDate = FromUtf8(pStmt.GetColumnString(19));

			long long playedDate =  pStmt.GetColumnInt64(21);
			if (playedDate == 0) {
				loadInfo.ItemInfo.TitleLastPlayedDate = "Never";
			}
			else {
				SYSTEMTIME st;
				time_t time = playedDate;  //pStmt.GetColumnInt64(0);
				UnixTimeToSystemTime(&time, &st);
				loadInfo.ItemInfo.TitleLastPlayedDate = wstrtostr(SystemTimeToLocalString(st));
			}

			loadInfo.ItemInfo.TitleLastPlayedOrder = pStmt.GetColumnInt64(22);
			loadInfo.ItemInfo.TitleUserRating = pStmt.GetColumnInt(23);
			int favorite = pStmt.GetColumnInt(24);
			if (favorite) {
				loadInfo.ItemInfo.TitleIsFavorite = true;
			}
			else {
				loadInfo.ItemInfo.TitleIsFavorite = false;
			}

			// Now we can go ahead and add this item
			ItemVector.push_back(loadInfo);
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't fetch content: %s", 
			exception.GetString().c_str());
	}

	// Free our query
	pStmt.FreeQuery();

	// Unlock
	FSD_UNLOCK;

	// Now load all our items
	for(DWORD x = 0; x < ItemVector.size(); x++) {

		// Get our item, load assets, and add to manager
		CONTENT_ITEM_LOAD_INFO& item = ItemVector.at(x);
		LoadContentAssets(item.ContentId, item.AssetInfo);
		// Only load up queue items if the status is enabled
		if(SETTINGS::getInstance().getResumeAssetDownload() == TRUE)
			LoadContentHttpQueue(item.ItemInfo.ContentHash, item.HttpQueue);
	}
}
BOOL FSDSql::ItemPathExistsInDB(string ItemPath, ScanPath * ScanPath) {
	
	// Lock
	FSD_LOCK;

	// Get our db and statement
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Try and get our item id
	ostringstream q; DWORD contentId = 0; DWORD scanpathId = 0;
	//string szSerial = "";
	q << "SELECT ContentItemId FROM ContentItems, ScanPaths ";
	q << "  WHERE ContentItemPath = ?";
	q << " AND ScanPathId = ?";

	try {
		pStmt.Sql(q.str());
		pStmt.BindString(1, ItemPath);
		pStmt.BindInt64(2, ScanPath->PathId);

		while(pStmt.FetchRow()) {
			contentId = pStmt.GetColumnInt(0);
		};
			
		pStmt.FreeQuery();
	}
	catch (Kompex::SQLiteException &exception) {
		DebugMsg("FSDSql", "Exception:  %s", exception.GetString().c_str());
		contentId = 0;
	}

	// Unlock
	FSD_UNLOCK;

	// Return if we have it
	return (contentId != 0);
}
BOOL FSDSql::ItemTitleIdExistsInDB(DWORD TitleId) {
	
	// Lock
	FSD_LOCK;

	// Get our db and statement
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Try and get our item id
	ostringstream q; DWORD contentId = 0;
	q << "SELECT ContentItemId FROM ContentItems ";
	q << "  WHERE ContentItemTitleId = ?";
	
	try {
		pStmt.Sql(q.str());
		pStmt.BindInt(1, TitleId);
		while(pStmt.FetchRow())
			contentId = pStmt.GetColumnInt(0);
		pStmt.FreeQuery();
	}
	catch (Kompex::SQLiteException &exception) {
		DebugMsg("FSDSql", "Exception:  %s", exception.GetString().c_str());
		contentId = 0;
	}

	// Unlock
	FSD_UNLOCK;

	// Return if we have it
	return (contentId != 0);
}
void FSDSql::UpdateContentInfo(DWORD ContentId, CONTENT_ITEM_INFO& ItemInfo) {

	// Lock
	FSD_LOCK;

	// Get our database
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Setup query
	ostringstream q;
	q << "UPDATE ContentItems SET ";
	q << "  ContentItemScanPathId = ?, ContentItemFileType = ?, ContentItemContentType = ?,";
	q << "  ContentItemTab = ?, ContentItemPath = ?, ContentItemDirectory = ?,";
	q << "  ContentItemFileName = ?";
	q << "  WHERE ContentItemId = ?";

	try {
		
		// Setup our query
		pStmt.Sql(q.str());

		// Bind our values to be updated
		pStmt.BindInt(1, ItemInfo.ItemScanPathId);
		pStmt.BindInt(2, ItemInfo.FileType);
		pStmt.BindInt(3, ItemInfo.ContentType);
		pStmt.BindInt(4, ItemInfo.ItemTab);
		pStmt.BindString(5, ItemInfo.ItemPath);
		pStmt.BindString(6, ItemInfo.ItemDirectory);
		pStmt.BindString(7, ItemInfo.ItemFileName);

		// Bind our ID finally
		pStmt.BindInt(8, ContentId);

		// Execute and free
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Exception:  %s", exception.GetString().c_str());
		DebugMsg("FSDSql", "Couldn't update item [%d]", ContentId);
	}

	// Unlock
	FSD_UNLOCK;
}
void FSDSql::UpdateContentItemDisplayInfo(DWORD ContentId, CONTENT_ITEM_DISPLAY_INFO& DisplayInfo) {

	// LOck
	FSD_LOCK;

	// Get our database
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Setup query
	ostringstream q;
	q << "UPDATE ContentItems SET ";
	q << "  ContentItemName = ?, ContentItemDescription = ?, ContentItemDeveloper = ?, ";
	q << "  ContentItemPublisher = ?, ContentItemGenre = ?, ContentItemRating = ?, ";
	q << "  ContentItemRaters = ?, ContentItemReleaseDate = ? ";
	q << "  WHERE ContentItemId = ?";

	try {
		
		// Setup our query
		pStmt.Sql(q.str());

		// Bind our values to be updated
		pStmt.BindString(1, ToUtf8(DisplayInfo.TitleName));
		pStmt.BindString(2, ToUtf8(DisplayInfo.TitleDescription));
		pStmt.BindString(3, ToUtf8(DisplayInfo.TitleDeveloper));
		pStmt.BindString(4, ToUtf8(DisplayInfo.TitlePublisher));
		pStmt.BindString(5, ToUtf8(DisplayInfo.TitleGenre));
		pStmt.BindString(6, ToUtf8(DisplayInfo.TitleRating));
		pStmt.BindString(7, ToUtf8(DisplayInfo.TitleRaters));
		pStmt.BindString(8, ToUtf8(DisplayInfo.TitleReleaseDate));

		// Bind our ID finally
		pStmt.BindInt(9, ContentId);

		// Execute and free
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Exception:  %s", exception.GetString().c_str());
		DebugMsg("FSDSql", "Couldn't update item [%d]", ContentId);
	}

	// Unlock
	FSD_UNLOCK;
}
void FSDSql::DeleteContentAsset(DWORD ContentId, ASSET_TYPE AssetType) {

	// Lock
	FSD_LOCK;

	// Get our DB
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Setup our query stream
	ostringstream q;
	q << "SELECT AssetAssetTypeId, AssetFilename";
	q << " FROM Assets WHERE AssetContentId = ? AND AssetAssetTypeId = ?";

	try	{

		// Set our statement and bind our ID
		pStmt.Sql(q.str());
		pStmt.BindInt(1, ContentId);
		pStmt.BindInt(2, AssetType);

		// Now loop and fetch each item		
		while (pStmt.FetchRow()) {
			// Get our asset type and delete if its a file
			ASSET_TYPE assetType = (ASSET_TYPE)pStmt.GetColumnInt(0);
			if(assetType != ASSET_TYPE_ICON) {
				string fileName = pStmt.GetColumnString(1);
				unlink(fileName.c_str());
			}
		}

		// Free our query
		pStmt.FreeQuery();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't delete asset for %d [%s]", 
			ContentId, exception.GetString().c_str());
	}

	// Setup our delete query to delete all thoes items we just handlded
	q.str("");
	q << "DELETE FROM Assets WHERE AssetContentId = ? AND AssetAssetTypeId = ?";

	try {

		// Bind our content id
		pStmt.Sql(q.str());
		pStmt.BindInt(1, ContentId);
		pStmt.BindInt(2, AssetType);

		// Now execute and free
		pStmt.ExecuteAndFree();
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't delete assets for %d [%s]", 
			ContentId, exception.GetString().c_str());
	}

	// Unlock
	FSD_UNLOCK;
}
void FSDSql::AddContentAsset(DWORD ContentId, ASSET_TYPE AssetType, ASSET_FILE_TYPE AssetFileType, VOID* AssetData, DWORD AssetDataLength) {
	
	// Lock
	FSD_LOCK;

	// Get our DB
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Setup our query
	ostringstream q;
	q << "INSERT INTO Assets (AssetAssetTypeId, AssetAssetFileTypeId, AssetContentId, ";
	q << ((AssetType == ASSET_TYPE_ICON) ? "AssetFileData" : "AssetFilename");
	q << ") VALUES (?, ?, ?, ?)";

	try {
		// Bind our values
		pStmt.Sql(q.str());
		pStmt.BindInt(1, AssetType);
		pStmt.BindInt(2, AssetFileType);
		pStmt.BindInt64(3, ContentId);

		// Bind proper data
		if(AssetType == ASSET_TYPE_ICON)
			pStmt.BindBlob(4, (VOID*)AssetData, AssetDataLength);
		else {
			long len = strlen((const char*)AssetData);
			char * szPath = (char*)malloc(len + 1);
			memset(szPath, 0, len + 1);
			strcpy_s(szPath, len+1, (const char*)AssetData);
			string assetPath = szPath;
			free(szPath);
			pStmt.BindString(4, assetPath);
		}

		// Execute and free
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't addAsset for %d [%s]", 
			ContentId, exception.GetString().c_str());
	}

	// Unlock
	FSD_UNLOCK;
}
void FSDSql::LoadContentAssets(DWORD ContentId, CONTENT_ITEM_ASSET_INFO& AssetInfo) {

	// Lock
	FSD_LOCK;

	// Get our DB
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Setup our query stream
	ostringstream q;
	q << "SELECT AssetAssetTypeId, AssetFilename, AssetFileData";
	q << " FROM Assets WHERE AssetContentId = ?";
	
	// Set our statement and bind our ID
	pStmt.Sql(q.str());
	pStmt.BindInt(1, ContentId);

	try	{

		ASSET_TYPE assetType;
		// Now loop and fetch each item		
		while (pStmt.FetchRow()) {

			// Get our asset type and handle it
			assetType = (ASSET_TYPE)pStmt.GetColumnInt(0);
			switch(assetType) {
				case ASSET_TYPE_ICON:
					AssetInfo.ThumbnailSize = pStmt.GetColumnBytes(2);
					AssetInfo.ThumbnailData = (BYTE*)malloc(AssetInfo.ThumbnailSize);
					memcpy(AssetInfo.ThumbnailData, pStmt.GetColumnBlob(2), 
						AssetInfo.ThumbnailSize);
					AssetInfo.AssetFlags |= CONTENT_ASSET_THUMBNAIL;
					break;
				case ASSET_TYPE_BOXCOVER:
					AssetInfo.BoxartPath = pStmt.GetColumnString(1);
					AssetInfo.AssetFlags |= CONTENT_ASSET_BOXART;
					break;
				case ASSET_TYPE_BACKGROUND:
					AssetInfo.BackgroundPath = pStmt.GetColumnString(1);
					AssetInfo.AssetFlags |= CONTENT_ASSET_BACKGROUND;
					break;
				case ASSET_TYPE_BANNER:
					AssetInfo.BannerPath = pStmt.GetColumnString(1);
					AssetInfo.AssetFlags |= CONTENT_ASSET_BANNER;
					break;
				case ASSET_TYPE_SCREENSHOT:
					AssetInfo.ScreenshotPaths.push_back(pStmt.GetColumnString(1));
					AssetInfo.ScreenshotCount++;
					AssetInfo.AssetFlags |= CONTENT_ASSET_SCREENSHOT;
					break;
				case ASSET_TYPE_VIDEO:
					AssetInfo.PreviewPath = pStmt.GetColumnString(1);
					AssetInfo.AssetFlags |= CONTENT_ASSET_PREVIEW;
					break;
				case ASSET_TYPE_NXE_SLOT:
					AssetInfo.NxeSlotPath = pStmt.GetColumnString(1);
					AssetInfo.AssetFlags |= CONTENT_ASSET_NXE_SLOT;
				default:
					DebugMsg("FSDSql", "Unknown asset type [%d]", assetType);
			}
		}

		// Free our query
		pStmt.FreeQuery();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't load asset for %d [%s]", 
			ContentId, exception.GetString().c_str());
	}

	// Unlock
	FSD_UNLOCK;
}
DWORD FSDSql::HttpQueueAddDownload( HttpItem * ref) {
	
	// Lock
	FSD_LOCK;

	// Get our db and statement
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Setup our new query
	ostringstream q;
	q.str("");
	q << "INSERT INTO HttpQueue (";
	q << "	HttpQueueQueueId,			HttpQueueTypeId,		HttpQueueUrl,		HttpQueueOutputPath,";
	q << "  HttpQueueOutputToMemory,	HttpQueueTag,			HttpQueueTag2,		HttpQueuePriority,";
	q << "  HttpQueueTimeStamp,			HttpQueueExpiration,	HttpQueueContentHash ";
	q << "  ) VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )";

	try {
		pStmt.Sql(q.str());

		pStmt.BindInt(1, ref->getQueueID());
		pStmt.BindInt(2, ref->getTypeID());
		pStmt.BindString(3, ref->getFullUrl());
		pStmt.BindString(4, ref->getSaveAsPath());
		pStmt.BindInt(5, ref->getOutputToMemory());
		pStmt.BindString(6, ref->getTag());
		pStmt.BindInt(7, ref->getTag2());
		pStmt.BindInt(8, ref->GetPriority());
		pStmt.BindInt64(9, ref->getTimeStamp());
		pStmt.BindInt(10, ref->getExpiration());
		pStmt.BindString(11, ref->getContentHash());
		
		pStmt.ExecuteAndFree();
	}
	catch(Kompex::SQLiteException) {

		DebugMsg("FSDSql", "INSERT failed, http queue item was probably there already");
	}

	// Unlock
	FSD_UNLOCK;

	return ref->getQueueID();
}

DWORD FSDSql::HttpQueueRemoveDownload( HttpItem * ref ) {

	
	// Lock
	FSD_LOCK;

	// Get our db and statement
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Setup our new query
	ostringstream q;
	q.str("");
	q << "DELETE FROM HttpQueue WHERE HttpQueueQueueId = ? AND HttpQueueUrl = ? AND HttpQueueContentHash = ?";

	try {
		pStmt.Sql(q.str());
		pStmt.BindInt(1, ref->getQueueID());
		pStmt.BindString(2, ref->getFullUrl());
		pStmt.BindString(3, ref->getContentHash());
		pStmt.ExecuteAndFree();
	}
	catch(Kompex::SQLiteException) {

		DebugMsg("FSDSql", "DELETE failed to process delete command");
	}

	// Unlock
	FSD_UNLOCK;

	return ref->getQueueID();
}

DWORD FSDSql::LoadContentHttpQueue( string contentHash, CONTENT_ITEM_HTTP_QUEUE& httpQueue )
{
	// Lock
	FSD_LOCK;

	// Get our DB
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Setup our query stream
	ostringstream q;
	q.str("");
	q << "SELECT HttpQueueQueueId, HttpQueueTypeId, HttpQueueUrl, HttpQueueOutputPath, HttpQueueOutputToMemory, ";
	q << "       HttpQueueTag, HttpQueueTag2, HttpQueuePriority, HttpQueueTimeStamp, HttpQueueExpiration  ";
	q << "FROM   HttpQueue WHERE HttpQueueContentHash = '" << contentHash.c_str() << "'  ";

	try {
		
		// Set our statement
		pStmt.Sql(q.str());

		// Now loop and fetch each item		
		while (pStmt.FetchRow()) {

			HTTP_ITEM httpItem;
			ZeroMemory(&httpItem, sizeof(HTTP_ITEM));
			
			// Fill in our HTTP_ITEM struct
			httpItem.QueueId = pStmt.GetColumnInt(0);
			httpItem.TypeId = (HTTP_TYPE)pStmt.GetColumnInt(1);
			httpItem.Url = pStmt.GetColumnString(2);
			httpItem.OutputPath = pStmt.GetColumnString(3);
			httpItem.OutputToMemory = pStmt.GetColumnInt(4);
			httpItem.Tag = pStmt.GetColumnString(5);
			httpItem.Tag2 = pStmt.GetColumnInt(6);
			httpItem.Priority = (HTTP_PRIORITY)pStmt.GetColumnInt(7);
			httpItem.TimeStamp = pStmt.GetColumnInt64(8);
			httpItem.Expiration = pStmt.GetColumnInt(9);
			httpItem.ContentHash = contentHash;

			// Add this data into our struct
			httpQueue.httpItemQueue.push_back(httpItem);
		}

		httpQueue.httpItemCount = httpQueue.httpItemQueue.size();

		pStmt.FreeQuery();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Failed to load the outstanding http items, %s", 
			exception.GetString().c_str());
	}

	// Now that these items are loaded, we need to remove them from the db
	q.str("");
	q << "DELETE FROM HttpQueue WHERE HttpQueueContentHash = ?";
	try {
		pStmt.Sql(q.str());
		pStmt.BindString(1, contentHash);
		pStmt.ExecuteAndFree();
	}
	catch(Kompex::SQLiteException) {

		DebugMsg("FSDSql", "DELETE failed to remove the http items");
	}

	// Unlock
	FSD_UNLOCK;

	return httpQueue.httpItemCount;
}

DWORD FSDSql::HttpQueueRestoreDownload( DWORD dwQueueID, string contentHash, vector<HTTP_ITEM>& items ) {

/*	// Lock
	FSD_LOCK;

	// Get our DB
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Get our many items we currently have
	ostringstream q; DWORD itemCount = 0;
	q << "SELECT COUNT(*) FROM HttpQueue WHERE HttpQueueQueueId = " << dwQueueID;

	try {
		itemCount = (DWORD)pStmt.SqlAggregateFuncResult(q.str());
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't get MAX ContentId", 
			exception.GetString().c_str());
	}

	// Setup our query stream
	q.str("");
	q << "SELECT HttpQueueTypeId, HttpQueueUrl, HttpQueueOutputPath, HttpQueueOutputToMemory, ";
	q << "       HttpQueueTag, HttpQueueTag2, HttpQueuePriority, HttpQueueTimeStamp, HttpQueueExpiration  ";
	q << "FROM   HttpQueue WHERE HttpQueueQueueId = " << dwQueueID << " AND HttpQueueContentHash = '" << contentHash.c_str() << "'  ";

	try {
		
		// Set our statement
		pStmt.Sql(q.str());

		// Now loop and fetch each item		
		DWORD loopCount = 0;
		while (pStmt.FetchRow()) {

			HTTP_ITEM item;
			ZeroMemory(&item, sizeof(HTTP_ITEM));
			
			item.queueId = dwQueueID;
			item.typeId = (HTTP_TYPE)pStmt.GetColumnInt(0);
			item.url = pStmt.GetColumnString(1);
			item.outputPath = pStmt.GetColumnString(2);
			item.outputToMemory = pStmt.GetColumnInt(3);
			item.tag = pStmt.GetColumnString(4);
			item.tag2 = pStmt.GetColumnInt(5);
			item.priority = (HTTP_PRIORITY)pStmt.GetColumnInt(6);
			item.timestamp = pStmt.GetColumnInt64(7);
			item.expiration = pStmt.GetColumnInt(8);
			item.contentHash = contentHash;

			items.push_back(item);
		}

		pStmt.FreeQuery();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't delete queue item for %d [%s]", 
			dwQueueID, exception.GetString().c_str());
	}

	// Setup our new query
	q.str("");
	q << "DELETE FROM HttpQueue WHERE HttpQueueContentHash = ?";
	try {
		pStmt.Sql(q.str());
		pStmt.BindString(1, contentHash);
		pStmt.ExecuteAndFree();
	}
	catch(Kompex::SQLiteException) {

		DebugMsg("FSDSql", "DELETE failed to process delete command");
	}
	
	FSD_UNLOCK;*/

	return 0;//itemCount;
}

void FSDSql::DeleteContentItemAssets(DWORD ContentId) {

	// Lock
	FSD_LOCK;

	// Get our DB
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Setup our query stream
	ostringstream q;
	q << "SELECT AssetAssetTypeId, AssetFilename";
	q << " FROM Assets WHERE AssetContentId = ?";

	try	{

		// Set our statement and bind our ID
		pStmt.Sql(q.str());
		pStmt.BindInt(1, ContentId);

		// Now loop and fetch each item		
		while (pStmt.FetchRow()) {
			// Get our asset type and delete if its a file
			ASSET_TYPE assetType = (ASSET_TYPE)pStmt.GetColumnInt(0);
			if(assetType != ASSET_TYPE_ICON) {
				string fileName = pStmt.GetColumnString(1);
				unlink(fileName.c_str());
			}
		}

		// Free our query
		pStmt.FreeQuery();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't delete asset for %d [%s]", 
			ContentId, exception.GetString().c_str());
	}

	// Setup our delete query to delete all thoes items we just handlded
	q.str("");
	q << "DELETE FROM Assets WHERE AssetContentId = ?";

	try {

		// Bind our content id
		pStmt.Sql(q.str());
		pStmt.BindInt(1, ContentId);

		// Now execute and free
		pStmt.ExecuteAndFree();
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't delete assets for %d [%s]", 
			ContentId, exception.GetString().c_str());
	}

	// Unlock
	FSD_UNLOCK;
}
void FSDSql::DeleteContentItem(DWORD ContentId) {

	// Lock
	FSD_LOCK;

	// Get our DB
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Setup our query stream
	ostringstream q;
	q << "DELETE FROM ContentItems WHERE ContentItemId = ?";

	try {

		// Bind our content id
		pStmt.Sql(q.str());
		pStmt.BindInt(1, ContentId);

		// Now execute and free
		pStmt.ExecuteAndFree();
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't delete assets for %d [%s]", 
			ContentId, exception.GetString().c_str());
	}

	// Unlock
	FSD_UNLOCK;
}
void FSDSql::LoadMultiDiscInfo(DWORD TitleId, DWORD DiscsInSet, MULTI_DISC_INFO& MultiDiscInfo) {
	
	// Lock
	FSD_LOCK;

	// Get our DB
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	// Setup our query stream
	ostringstream q;
	q << "SELECT ContentItemDiscNum, ContentItemPath, ContentItemDirectory, ContentItemFileType, ScanPathDeviceId";
	q << " FROM   ContentItems, ScanPaths, MountedDevices";
	q << " WHERE ContentItemTitleId = ? ";
	q << " AND ContentItemDiscsInSet = ? ";
	q << " AND ContentItemScanPathId = ScanPathId ";
	q << " AND ScanPathDeviceId = MountedDeviceDeviceId ";
	
	// Set our statement and bind our ID
	pStmt.Sql(q.str());
	pStmt.BindInt(1, TitleId);
	pStmt.BindInt(2, DiscsInSet);

	try	{

		// Set our discs in set
		MultiDiscInfo.DiscsInSet = DiscsInSet;

		// Now loop and fetch each item
		MultiDiscInfo.DiscsFound = 0;
		while (pStmt.FetchRow()) {

			// Get our info
			DWORD discNum = pStmt.GetColumnInt(0);

			// Make sure this is a valid disc num
			if(discNum > DiscsInSet || discNum > 5) {
				DebugMsg("FSDSql", "Disc out of range for 0x%08X", TitleId);
				break;
			}
			string szSerial = pStmt.GetColumnString(4);
			Drive * drv = DrivesManager::getInstance().getDriveBySerialNumberStr(szSerial);
			
			//string szRootPath = DrivesManager::getInstance().getRootPathBySerialNumberStr(szSerial);
			string szFullPath = drv->getSystemPath() + pStmt.GetColumnString(2);
			string szPathExec = drv->getSystemPath() + pStmt.GetColumnString(1);

			// Set our flag to indicate if this is a container or an executable
			MultiDiscInfo.DiscIsCon[discNum - 1] = pStmt.GetColumnInt(3) == CONTENT_FILE_TYPE_CONTAINER ? true : false;

			// Get our path and increment how many we loaded
//			if(MultiDiscInfo.DiscIsCon[discNum - 1] == true)
			MultiDiscInfo.DiscPaths[discNum - 1] = szPathExec;
//			else
//				MultiDiscInfo.DiscPaths[discNum - 1] = szFullPath;

			MultiDiscInfo.DiscsFound++;
		}

		// Free our query
		pStmt.FreeQuery();

		// Check to make sure we loaded what we need
		if(MultiDiscInfo.DiscsFound != DiscsInSet)
			DebugMsg("FSDSql", "Disc count mismatch for 0x%08X", TitleId);
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't load multi disc info for 0x%08X [%s]", 
			TitleId, exception.GetString().c_str());
	}

	// Unlock
	FSD_UNLOCK;
}
void FSDSql::buildQueries() {
	createAssetTypes = "CREATE TABLE IF NOT EXISTS AssetTypes (";
	createAssetTypes += "  AssetTypeId INTEGER PRIMARY KEY,";
	createAssetTypes += "  AssetTypeName TEXT,";
	createAssetTypes += "  AssetTypeFolder TEXT ";
	createAssetTypes += "  )";

	createAssetFileTypes = "CREATE TABLE IF NOT EXISTS AssetFileTypes (";
	createAssetFileTypes += " AssetFileTypeId INTEGER PRIMARY KEY,";
	createAssetFileTypes += " AssetFileTypeName TEXT";
	createAssetFileTypes += " )";

	createTabs = "CREATE TABLE IF NOT EXISTS Settings.Tabs (";
	createTabs += "  TabId INTEGER PRIMARY KEY,";
	createTabs += "  TabName TEXT ";
	createTabs += " )";

/*	createScanPahts = "CREATE TABLE IF NOT EXISTS ScanPaths (";
	createScanPahts += "  ScanPathId INTEGER PRIMARY KEY,";
	createScanPahts += "  ScanPathPath TEXT,";
	createScanPahts += "  ScanPathStorageDevice TEXT,";
	createScanPahts += "  ScanPathRetailTabId INTEGER,";
	createScanPahts += "  ScanPathDevkitTabId INTEGER,";
	createScanPahts += "  ScanPathDepth INTEGER";
	createScanPahts += " )";
*/
	createContentTypes = "CREATE TABLE IF NOT EXISTS ContentTypes (";
	createContentTypes += "  ContentTypeId INTEGER PRIMARY KEY,";
	createContentTypes += "  ContentTypeOfferId TEXT,";
	createContentTypes += "  ContentTypeDescription TEXT,";
	createContentTypes += "  ContentTypeDefaultTabId INTEGER";
	createContentTypes += " )";

	createFavorites = "CREATE TABLE IF NOT EXISTS Favorites (";
	createFavorites += "  FavoriteId INTEGER PRIMARY KEY, ";
	createFavorites += "  FavoriteGamerProfile TEXT, ";			// this can be "" for when no one is signed in
	createFavorites += "  FavoriteContentId INTEGER, ";
	createFavorites += "  UNIQUE (FavoriteGamerProfile, FavoriteContentId) ";
	createFavorites += ")";

	createRecentlyPlayedTitles = "CREATE TABLE IF NOT EXISTS RecentlyPlayedTitles (";
	createRecentlyPlayedTitles += "  RecentlyPlayedTitleId INTEGER PRIMARY KEY, ";			// this can be "" for when no one is signed in
	createRecentlyPlayedTitles += "  RecentlyPlayedTitleGamerProfile TEXT, ";			// this can be "" for when no one is signed in
	createRecentlyPlayedTitles += "  RecentlyPlayedTitleContentId INTEGER, ";
	createRecentlyPlayedTitles += "  RecentlyPlayedTitleDateTime INTEGER, ";
	createRecentlyPlayedTitles += "  RecentlyPlayedTitleOrder INTEGER, ";				// any time the table is updated this column gets the new highest value
	createRecentlyPlayedTitles += "  UNIQUE (RecentlyPlayedTitleGamerProfile, RecentlyPlayedTitleContentId) ";
	createRecentlyPlayedTitles += ")";

	createUserRatings = "CREATE TABLE IF NOT EXISTS UserRatings (";
	createUserRatings += "  UserRatingId INTEGER PRIMARY KEY, ";
	createUserRatings += "  UserRatingGamerProfile TEXT, ";			// this can be "" for when no one is signed in
	createUserRatings += "  UserRatingContentId INTEGER, ";
	createUserRatings += "  UserRatingRating INTEGER, ";		
	createUserRatings += "  UNIQUE (UserRatingGamerProfile, UserRatingContentId) ";
	createUserRatings += ")";

	createAssets = "CREATE TABLE IF NOT EXISTS Assets (";
	createAssets += "  AssetId INTEGER PRIMARY KEY, ";
	createAssets += "  AssetAssetTypeId INTEGER, ";
	createAssets += "  AssetAssetFileTypeId INTEGER, ";
	createAssets += "  AssetContentId INTEGER, ";
	createAssets += "  AssetFileName TEXT, ";
	createAssets += "  AssetFileData BLOB, ";
	createAssets += "  UNIQUE (AssetContentId, AssetAssetTypeId, AssetFileName) ";
	createAssets += "  )";

	/*
	createContentItems = "CREATE TABLE IF NOT EXISTS ContentItems (";
	createContentItems += "  ContentItemId INTEGER PRIMARY KEY, ";
	createContentItems += "  ContentItemTitleId TEXT,";
	createContentItems += "  ContentItemTitleIdCounter INTEGER,";  // almost always 1, except when we find duplicate titleIds
	createContentItems += "  ContentItemPath TEXT, ";
	createContentItems += "  ContentItemExecutable TEXT, ";
	createContentItems += "  ContentItemScanPathId INTEGER, ";
	createContentItems += "  ContentItemContentTypeId INTEGER check(typeof(ContentItemContentTypeId) = 'integer'), ";
	createContentItems += "  ContentItemTabId INTEGER, ";
	createContentItems += "  ContentItemMediaId TEXT, ";
	createContentItems += "  ContentItemTitleName TEXT,";
	createContentItems += "  ContentItemDiscNumber INTEGER, ";	
	createContentItems += "  ContentItemTotalDiscs INTEGER, ";	
	createContentItems += "  ContentItemReleaseDate TEXT, ";
	createContentItems += "  ContentItemGenre TEXT, ";
	createContentItems += "  ContentItemDeveloper TEXT, ";
	createContentItems += "  ContentItemPublisher TEXT, ";
	createContentItems += "  ContentItemDescription TEXT, ";  
	createContentItems += "  ContentItemRating TEXT, ";		// could be a REAL, but why bother?
	createContentItems += "  ContentItemTotalRaters INTEGER, ";
	createContentItems += "  UNIQUE (ContentItemPath, ContentItemExecutable, ContentItemScanPathId) ";
	createContentItems += ")";
	*/

	createContentItems = "CREATE TABLE IF NOT EXISTS ContentItems (";
	createContentItems += "  ContentItemId		     INTEGER PRIMARY KEY, ";
	createContentItems += "  ContentItemScanPathId   INTEGER, ";
	createContentItems += "  ContentItemFileType     INTEGER, ";
	createContentItems += "  ContentItemContentType  INTEGER, ";
	createContentItems += "  ContentItemTab          INTEGER, ";	
	createContentItems += "  ContentItemPath         TEXT,    ";
	createContentItems += "  ContentItemDirectory    TEXT,    ";
	createContentItems += "  ContentItemFileName     TEXT,    ";
	createContentItems += "  ContentItemTitleId      INTEGER, ";
	createContentItems += "  ContentItemMediaId      INTEGER, ";
	createContentItems += "  ContentItemDiscNum      INTEGER, ";
	createContentItems += "  ContentItemDiscsInSet   INTEGER, ";
	createContentItems += "  ContentItemName         TEXT,    ";
	createContentItems += "  ContentItemDescription  TEXT,    ";
	createContentItems += "  ContentItemDeveloper    TEXT,    ";	
	createContentItems += "  ContentItemPublisher    TEXT,    ";	
	createContentItems += "  ContentItemGenre        TEXT,    ";
	createContentItems += "  ContentItemRating       TEXT,    ";
	createContentItems += "  ContentItemRaters       TEXT,    ";
	createContentItems += "  ContentItemReleaseDate  TEXT,    ";
	createContentItems += "  UNIQUE (ContentItemPath) ";
	createContentItems += ")";

	createContentItemsV6 = "CREATE TABLE IF NOT EXISTS TempContentItems (";
	createContentItemsV6 += "  ContentItemId		   INTEGER PRIMARY KEY, ";
	createContentItemsV6 += "  ContentItemScanPathId   INTEGER, ";
	createContentItemsV6 += "  ContentItemFileType     INTEGER, ";
	createContentItemsV6 += "  ContentItemContentType  INTEGER, ";
	createContentItemsV6 += "  ContentItemTab          INTEGER, ";	
	createContentItemsV6 += "  ContentItemPath         TEXT,    ";
	createContentItemsV6 += "  ContentItemDirectory    TEXT,    ";
	createContentItemsV6 += "  ContentItemFileName     TEXT,    ";
	createContentItemsV6 += "  ContentItemTitleId      INTEGER, ";
	createContentItemsV6 += "  ContentItemMediaId      INTEGER, ";
	createContentItemsV6 += "  ContentItemDiscNum      INTEGER, ";
	createContentItemsV6 += "  ContentItemDiscsInSet   INTEGER, ";
	createContentItemsV6 += "  ContentItemName         TEXT,    ";
	createContentItemsV6 += "  ContentItemDescription  TEXT,    ";
	createContentItemsV6 += "  ContentItemDeveloper    TEXT,    ";	
	createContentItemsV6 += "  ContentItemPublisher    TEXT,    ";	
	createContentItemsV6 += "  ContentItemGenre        TEXT,    ";
	createContentItemsV6 += "  ContentItemRating       TEXT,    ";
	createContentItemsV6 += "  ContentItemRaters       TEXT,    ";
	createContentItemsV6 += "  ContentItemReleaseDate  TEXT,    ";
	createContentItemsV6 += "  UNIQUE (ContentItemPath, ContentItemFileName, ContentItemScanPathId) ";
	createContentItemsV6 += ")";

	createContentItemsIndex = "CREATE INDEX IF NOT EXISTS IDX_ContentItems_By_Id ON ContentItems(ContentItemId ASC)";

	// version 2 schema starts here
/*	createTitleUpdatePaths = "CREATE TABLE IF NOT EXISTS TitleUpdatePaths (";
	createTitleUpdatePaths += "  TitleUpdatePathId INTEGER PRIMARY KEY,  ";
	createTitleUpdatePaths += "  TitleUpdatePathRoot TEXT,  ";
	createTitleUpdatePaths += "  TitleUpdatePathDeviceId TEXT,  ";
	createTitleUpdatePaths += "  TitleUpdatePathPath TEXT,  ";
	createTitleUpdatePaths += "  TitleUpdatePathIsBackup INTEGER,  ";
	createTitleUpdatePaths += "  TitleUpdatePathBackupForTitleUpdatePathId INTEGER,  ";
	createTitleUpdatePaths += "  UNIQUE (TitleUpdatePathDeviceId, TitleUpdatePathRoot, TitleUpdatePathPath) ";
	createTitleUpdatePaths += ")";
*/
/*	createTitleUpdates = "CREATE TABLE IF NOT EXISTS TitleUpdates (";
	createTitleUpdates += "  TitleUpdateId INTEGER PRIMARY KEY, ";
	createTitleUpdates += "  TitleUpdateTitleId TEXT, ";
	createTitleUpdates += "  TitleUpdateUpdateName TEXT, ";
	createTitleUpdates += "  TitleUpdateFileName TEXT, ";
	createTitleUpdates += "  TitleUpdateTitleUpdatePathId INTEGER, ";
	createTitleUpdates += "  UNIQUE (TitleUpdateFileName, TitleUpdateTitleUpdatePathId) ";
	createTitleUpdates += ")";
*/
/*	createKaiUsers = "CREATE TABLE IF NOT EXISTS KaiUsers (";
	createKaiUsers += "  KaiUserId INTEGER PRIMARY KEY,  ";
	createKaiUsers += "  KaiUserName TEXT,  ";
	createKaiUsers += "  KaiUserPassword TEXT,  ";
	createKaiUsers += "  KaiUserLinkedXUID TEXT,  ";
	createKaiUsers += "  UNIQUE (KaiUserName, KaiUserLinkedXUID)  ";
	createKaiUsers += ")";
*/
	createKaiMessages = "CREATE TABLE IF NOT EXISTS Settings.KaiMessages (";
	createKaiMessages += "  KaiMessageId INTEGER PRIMARY KEY,  ";
	createKaiMessages += "  KaiMessageSender TEXT,  ";
	createKaiMessages += "  KaiMessageBody TEXT,  ";
	createKaiMessages += "  KaiMessageFlag INTEGER ,  ";
	createKaiMessages += "  KaiMessageDateTime INTEGER,  ";
	createKaiMessages += "  KaiMessageKaiUserId INTEGER,  ";
	createKaiMessages += "  KaiMessageKaiContactId INTEGER  ";
	createKaiMessages += ")";

	createKaiContacts = "CREATE TABLE IF NOT EXISTS Settings.KaiContacts (";
	createKaiContacts += "  KaiContactId INTEGER PRIMARY KEY,  ";
	createKaiContacts += "  KaiContactName TEXT,  ";
	createKaiContacts += "  KaiContactIconPath TEXT,  ";
	createKaiContacts += "  UNIQUE (KaiContactName)  ";
	createKaiContacts += ")";

	createKaiUserToContacts = "CREATE TABLE IF NOT EXISTS Settings.KaiUserToContacts (";
	createKaiUserToContacts += "  KaiUserToContactsKaiUserId INTEGER, ";
	createKaiUserToContacts += "  KaiUserToContactsKaiContactId INTEGER, ";
	createKaiUserToContacts += "  UNIQUE (KaiUserToContactsKaiUserId, KaiUserToContactsKaiContactId)  ";
	createKaiUserToContacts += ")";

	// version 3
	createKaiUsersV3 = "CREATE TABLE IF NOT EXISTS Settings.KaiUsers (";
	createKaiUsersV3 += "  KaiUserId INTEGER PRIMARY KEY,  ";
	createKaiUsersV3 += "  KaiUserName TEXT,  ";
	createKaiUsersV3 += "  KaiUserPassword TEXT,  ";
	createKaiUsersV3 += "  UNIQUE (KaiUserName)  ";
	createKaiUsersV3 += ")";

	createProfileXuids = "CREATE TABLE IF NOT EXISTS Settings.ProfileXuids (";
	createProfileXuids += "  ProfileXuidId INTEGER PRIMARY KEY,  ";
	createProfileXuids += "  ProfileXuidXuid TEXT,  ";
	createProfileXuids += "  ProfileXuidKaiUserId INTEGER,  ";
	createProfileXuids += "  UNIQUE (ProfileXuidXuid) ";
	createProfileXuids += ")";

	// version 4
/*	createTitleUpdatePathsV4 = "CREATE TABLE IF NOT EXISTS Settings.TitleUpdatePaths (";
	createTitleUpdatePathsV4 += "  TitleUpdatePathId INTEGER PRIMARY KEY,  ";
	createTitleUpdatePathsV4 += "  TitleUpdatePathDeviceId TEXT,  ";
	createTitleUpdatePathsV4 += "  TitleUpdatePathPath TEXT,  ";
	createTitleUpdatePathsV4 += "  TitleUpdatePathIsBackup INTEGER,  ";
	createTitleUpdatePathsV4 += "  TitleUpdatePathBackupForTitleUpdatePathId INTEGER,  ";
	createTitleUpdatePathsV4 += "  UNIQUE (TitleUpdatePathDeviceId, TitleUpdatePathPath) ";
	createTitleUpdatePathsV4 += ")";
*/
	createTitleUpdateDisables = "CREATE TABLE IF NOT EXISTS Settings.TitleUpdateDisables (";
	createTitleUpdateDisables += " TitleUpdateDisableId INTEGER PRIMARY KEY,  ";
	createTitleUpdateDisables += " TitleUpdateDisableProfileXuid TEXT,  ";
	createTitleUpdateDisables += " TitleUpdateDisableTitleUpdateId TEXT,  ";
	createTitleUpdateDisables += " UNIQUE (TitleUpdateDisableProfileXuid, TitleUpdateDisableTitleUpdateId) ";
	createTitleUpdateDisables += ")";

	// version 5
	createScanPathsV5 = "CREATE TABLE IF NOT EXISTS Settings.ScanPaths (";
	createScanPathsV5 += "  ScanPathId INTEGER PRIMARY KEY,";
	createScanPathsV5 += "  ScanPathDeviceId TEXT,";
	createScanPathsV5 += "  ScanPathPath TEXT,";
	createScanPathsV5 += "  ScanPathRetailTabId INTEGER,";
	createScanPathsV5 += "  ScanPathDevkitTabId INTEGER,";
	createScanPathsV5 += "  ScanPathDepth INTEGER, ";
	createScanPathsV5 += "  UNIQUE (ScanPathDeviceId, ScanPathPath) ";
	createScanPathsV5 += " )";

	//version 7
	createTitleUpdatePathsV7 = "CREATE TABLE IF NOT EXISTS Settings.TitleUpdatePaths (";
	createTitleUpdatePathsV7 += "  TitleUpdatePathId INTEGER PRIMARY KEY,  ";
	createTitleUpdatePathsV7 += "  TitleUpdatePathDeviceId TEXT,  ";
	createTitleUpdatePathsV7 += "  TitleUpdatePathContentPath TEXT,  ";
	createTitleUpdatePathsV7 += "  TitleUpdatePathCachePath TEXT, ";
	createTitleUpdatePathsV7 += "  TitleUpdatePathDataPath TEXT, ";
	createTitleUpdatePathsV7 += "  UNIQUE (TitleUpdatePathDeviceId ) ";
	createTitleUpdatePathsV7 += ")";

	createTitleUpdatesV7 = "CREATE TABLE IF NOT EXISTS TitleUpdates (";
	createTitleUpdatesV7 += "  TitleUpdateId INTEGER PRIMARY KEY, ";
	createTitleUpdatesV7 += "  TitleUpdateTitleId INTEGER, ";
	createTitleUpdatesV7 += "  TitleUpdateUpdateName TEXT, ";
	createTitleUpdatesV7 += "  TitleUpdateTitleUpdatePathAdd TEXT, ";
	createTitleUpdatesV7 += "  TitleUpdateFileName TEXT, ";
	createTitleUpdatesV7 += "  TitleUpdateTitleUpdatePathId INTEGER, ";
	createTitleUpdatesV7 += "  TitleUpdateTitleUpdatePathType INTEGER, ";
	createTitleUpdatesV7 += "  TitleUpdateIsBackup INTEGER, ";
	createTitleUpdatesV7 += "  TitleUpdateIsBackupForTitleUpdateId INTEGER ";
	createTitleUpdatesV7 += ")";

	//version 9
	createHttpQueueTable = "CREATE TABLE IF NOT EXISTS HttpQueue (";
	createHttpQueueTable += "  HttpQueueQueueId INTEGER,  ";
	createHttpQueueTable += "  HttpQueueTypeId INTEGER,  ";
	createHttpQueueTable += "  HttpQueueUrl TEXT,  ";
	createHttpQueueTable += "  HttpQueueOutputPath TEXT, ";
	createHttpQueueTable += "  HttpQueueOutputToMemory INTEGER, ";
	createHttpQueueTable += "  HttpQueueTag TEXT,  ";
	createHttpQueueTable += "  HttpQueueTag2 INTEGER,  ";
	createHttpQueueTable += "  HttpQueuePriority INTEGER,  ";
	createHttpQueueTable += "  HttpQueueTimeStamp INTEGER, ";
	createHttpQueueTable += "  HttpQueueExpiration INTEGER, ";
	createHttpQueueTable += "  HttpQueueContentHash INTEGER ";
	createHttpQueueTable += ")";

	// Settings version 1
	createGlobalSettings = "CREATE TABLE IF NOT EXISTS Settings.GlobalSettings (";
	createGlobalSettings += "  GlobalSettingSkin TEXT, ";
	createGlobalSettings += "  GlobalSettingVideoPath TEXT, ";
	createGlobalSettings += "  GlobalSettingMusicPath TEXT, ";
	createGlobalSettings += "  GlobalSettingDVDCopy360Path TEXT, ";
	createGlobalSettings += "  GlobalSettingDVDCopyOrigPath TEXT, ";
	createGlobalSettings += "  GlobalSettingWeatherLocation TEXT, ";
	createGlobalSettings += "  GlobalSettingHOverscan INTEGER, ";
	createGlobalSettings += "  GlobalSettingVOverscan INTEGER, ";
	createGlobalSettings += "  GlobalSettingHDisplacement INTEGER, ";
	createGlobalSettings += "  GlobalSettingVDisplacement INTEGER, ";
	createGlobalSettings += "  GlobalSettingLetterbox INTEGER, ";
	createGlobalSettings += "  GlobalSettingFtpPort INTEGER, ";
	createGlobalSettings += "  GlobalSettingFtpUser TEXT, ";
	createGlobalSettings += "  GlobalSettingFtpPassword TEXT, ";
	createGlobalSettings += "  GlobalSettingFanSpeed INTEGER, ";
	createGlobalSettings += "  GlobalSettingLEDFormat INTEGER, ";
	createGlobalSettings += "  GlobalSettingTimeFormat INTEGER, ";
	createGlobalSettings += "  GlobalSettingDateFormat INTEGER, ";
	createGlobalSettings += "  GlobalSettingNTPOnStartup INTEGER, ";
	createGlobalSettings += "  GlobalSettingUpdateXEXNotice INTEGER, ";
	createGlobalSettings += "  GlobalSettingUpdateSkinNotice INTEGER, ";
	createGlobalSettings += "  GlobalSettingUpdatePluginNotice INTEGER, ";
	createGlobalSettings += "  GlobalSettingUpdateOnBeta INTEGER, ";
	createGlobalSettings += "  GlobalSettingInstallerState INTEGER ";
	createGlobalSettings += " )";

	//Settings version 2
	createGlobalSettingsV2_temp = "CREATE TABLE IF NOT EXISTS Settings.GlobalSettings_TEMP (";
	createGlobalSettingsV2_temp += "  GlobalSettingVideoPath TEXT, ";
	createGlobalSettingsV2_temp += "  GlobalSettingMusicPath TEXT, ";
	createGlobalSettingsV2_temp += "  GlobalSettingDVDCopy360Path TEXT, ";
	createGlobalSettingsV2_temp += "  GlobalSettingDVDCopyOrigPath TEXT, ";
	createGlobalSettingsV2_temp += "  GlobalSettingWeatherLocation TEXT, ";
	createGlobalSettingsV2_temp += "  GlobalSettingHOverscan INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingVOverscan INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingHDisplacement INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingVDisplacement INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingLetterbox INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingFtpPort INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingFtpUser TEXT, ";
	createGlobalSettingsV2_temp += "  GlobalSettingFtpPassword TEXT, ";
	createGlobalSettingsV2_temp += "  GlobalSettingFanSpeed INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingLEDFormat INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingTimeFormat INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingDateFormat INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingNTPOnStartup INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingUpdateXEXNotice INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingUpdateSkinNotice INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingUpdatePluginNotice INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingUpdateOnBeta INTEGER, ";
	createGlobalSettingsV2_temp += "  GlobalSettingInstallerState INTEGER ";
	createGlobalSettingsV2_temp += " )";

	createUserSettings = "CREATE TABLE IF NOT EXISTS Settings.UserSettings (";
	createUserSettings += " UserSettingGamerProfile TEXT, ";
	createUserSettings += "  UserSettingSkin TEXT, ";
	createUserSettings += "  UNIQUE(UserSettingGamerProfile) ";
	createUserSettings += ")";

	//Settings version 3
	createGameListSorts = "CREATE TABLE IF NOT EXISTS Settings.GameListSorts (";
	createGameListSorts += " GameListSortId INTEGER PRIMARY KEY,";
	createGameListSorts += " GameListSortGamerProfile TEXT, ";
	createGameListSorts += " GameListSortTabId INTEGER, ";
	createGameListSorts += " GameListSortTypeId INTEGER, ";
	createGameListSorts += " GameListSortDirectionId INTEGER, ";
	createGameListSorts += " GameListSortSubtitleId INTEGER, ";
	createGameListSorts += " UNIQUE (GameListSortGamerProfile, GameListSortTabId) ";
	createGameListSorts += ")";

	createGameListSortTypes = "CREATE TABLE IF NOT EXISTS Settings.GameListSortTypes (";
	createGameListSortTypes += " GameListSortTypeId INTEGER PRIMARY KEY,";
	createGameListSortTypes += " GameListSortType TEXT ";
	createGameListSortTypes += ")";

	createGameListSortDirections = "CREATE TABLE IF NOT EXISTS Settings.GameListSortDirections (";
	createGameListSortDirections += " GameListSortDirectionId INTEGER PRIMARY KEY, ";
	createGameListSortDirections += " GameListSortDirectionType TEXT ";
	createGameListSortDirections += ")";

	//Settings version 4 was inline

	//Settings version 5
	alterGlobalSettings = "ALTER TABLE Settings.GlobalSettings ";
	alterGlobalSettings += " ADD COLUMN GlobalSettingLocale TEXT";

	//Settings version 6 was inline

	//Settings version 7
	alterGlobalSettingsV2 = "ALTER TABLE Settings.GlobalSettings ";
	alterGlobalSettingsV2 += " ADD COLUMN GlobalSettingGameListScreenshots INTEGER";

	//Settings version 8 was inline

	//Settings version 11 was inline

	//Settings version 13
	createSambaClientCredentials = "CREATE TABLE IF NOT EXISTS Settings.SambaClientCredentials (";
	createSambaClientCredentials += " SambaClientCredentialId INTEGER PRIMARY KEY, ";
	createSambaClientCredentials += " SambaClientCredentialGamerProfile TEXT, ";
	createSambaClientCredentials += " SambaClientCredentialPath TEXT, ";
	createSambaClientCredentials += " SambaClientCredentialLogin TEXT, ";
	createSambaClientCredentials += " SambaClientCredentialEncryptedPassword TEXT, ";
	createSambaClientCredentials += " UNIQUE (SambaClientCredentialGamerProfile, SambaClientCredentialPath) ";
	createSambaClientCredentials += ")";

}

void FSDSql::addMountedDevice(const string& root, const string& deviceId) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);
	
	try {
		pStmt.Sql("INSERT INTO MountedDevices VALUES (?, ?)");
		pStmt.BindString(1, deviceId);
		pStmt.BindString(2, root);
		pStmt.ExecuteAndFree();

	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "addMountedDevice", exception.GetString().c_str());
	}

	FSD_UNLOCK;
}

vector<SQLTab*> FSDSql::getTabs() {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	vector<SQLTab*> retVec;

	try {
		ostringstream os;
		os << "SELECT TabId, TabName from Tabs "; 
		pStmt.Sql(os.str());
		while (pStmt.FetchRow()) {
			int col = 0;
			ULONGLONG id = pStmt.GetColumnInt64(col++);
			string tabName = pStmt.GetColumnString(col++);

			SQLTab* tab = new SQLTab(id, tabName);
			retVec.push_back(tab);
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getTabs", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return retVec;
}


void FSDSql::getGameListSort(const int tabId, const int playerId, int& sortType, int& sortDirection, int& subtitle, bool& bOnlyFavorites) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	vector<SQLTitleUpdate*> retVec;

	string gamerProfile = GetGamerId(playerId);
	sortType = 0;
	sortDirection = 0;
	subtitle = 0;

	try {
		ostringstream os;
		os << "SELECT GameListSortTypeId, GameListSortDirectionId, GameListSortSubtitleId, GameListSortFavorites FROM Settings.GameListSorts WHERE ";
		os << " GameListSortGamerProfile = ? AND ";
		os << " GameListSortTabId = ?";
		pStmt.Sql(os.str());
		pStmt.BindString(1, gamerProfile);
		pStmt.BindInt(2, tabId);
		if (pStmt.FetchRow()) {
			sortType = pStmt.GetColumnInt(0);
			sortDirection = pStmt.GetColumnInt(1);
			subtitle = pStmt.GetColumnInt(2);
			bOnlyFavorites = pStmt.GetColumnInt(3) ? true: false;
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getGameListSort", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return;
}

void FSDSql::setGameListSort(const int tabId, const int playerId, const int sortType, const int sortDirection, const int subtitle, const bool bOnlyFavorites) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();

	Kompex::SQLiteStatement pStmt(pDatabase);

	string gamerProfileId = GetGamerId(playerId);
	ostringstream q;

	try {
		q << "INSERT OR REPLACE INTO Settings.GameListSorts (GameListSortGamerProfile, GameListSortTabId, GameListSortTypeId, GameListSortDirectionId, GameListSortSubtitleId, GameListSortFavorites) ";
		q << " VALUES ('" << gamerProfileId << "', " << tabId << ", " << sortType << ", " << sortDirection << ", " << subtitle << ", " << (bOnlyFavorites ? 1 : 0) << ")";
		pStmt.SqlStatement(q.str());	
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "setGameListSort ", exception.GetString().c_str());
	}

	FSD_UNLOCK;
}
void FSDSql::cleanBadTUPaths()
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	vector<ULONGLONG> retVal;

	try {
		ostringstream os;
		os << "SELECT TitleUpdatePathId ";
		os << "  FROM TitleUpdatePaths";
		os << "  WHERE TitleUpdatePathDeviceId = '0000000000000000000000000000000000000000'";
		pStmt.Sql(os.str());
		while (pStmt.FetchRow()) {
			int col = 0;
			ULONGLONG id = pStmt.GetColumnInt64(col++);
									
			retVal.push_back(id);
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "cleanBadTUPaths exception 1: %s", exception.GetString().c_str());
	}

	for (unsigned int x = 0; x < retVal.size(); x++)
	{
		Kompex::SQLiteDatabase *pDatabase = getDatabase();
		Kompex::SQLiteStatement pStmt(pDatabase);

		try {
			ostringstream os;
			os << "DELETE FROM TitleUpdatePaths WHERE TitleUpdatePathId = ?";
			pStmt.Sql(os.str());
			pStmt.BindInt64(1, retVal.at(x));
			pStmt.ExecuteAndFree();
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "cleanBadTUPaths exception 2: %s", exception.GetString().c_str());
		}
	}
	FSD_UNLOCK;
}

TitleUpdatePath* FSDSql::getTitleUpdatePathByUpdateID( ULONGLONG titleUpdateId)
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	TitleUpdatePath* retVal;

	try {
		ostringstream os;
		os << "SELECT TitleUpdatePathId, TitleUpdatePathDeviceId, MountedDeviceMountPoint, TitleUpdatePathDataPath, TitleUpdatePathCachePath, TitleUpdatePathContentPath ";
		os << "  FROM TitleUpdates, TitleUpdatePaths, MountedDevices ";
		os << "  WHERE TitleUpdatePathDeviceId = MountedDeviceDeviceId";
		os << "  AND TitleUpdatePathId = TitleUpdateTitleUpdatePathId";
		os << "  AND TitleUpdateId = ?";
		pStmt.Sql(os.str());
		pStmt.BindInt64(1, titleUpdateId);
		while (pStmt.FetchRow()) {
			int col = 0;
			ULONGLONG id = pStmt.GetColumnInt64(col++);
			string deviceID = pStmt.GetColumnString(col++);
			string root = pStmt.GetColumnString(col++);
			string dataPath = pStmt.GetColumnString(col++);
			string cachePath = pStmt.GetColumnString(col++);
			string contentPath = pStmt.GetColumnString(col++);
			if (root.compare("USBMUCache0:") == 0 ) {
				contentPath = sprintfaA("USBMU0:%s", contentPath.c_str());
			} else if (	root.compare("USBMUCache1:") == 0 ) {
				contentPath = sprintfaA("USBMU1:%s", contentPath.c_str());
			} else if (	root.compare("USBMUCache2:") == 0) {
				contentPath = sprintfaA("USBMU2:%s", contentPath.c_str());
			} else {
				contentPath = root + contentPath;
			}
			cachePath = root + cachePath;
						
			retVal = new TitleUpdatePath(id, deviceID, dataPath, cachePath, contentPath, root);
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getTitleUpdatePathByUpdateID: %s", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return retVal;
}

TitleUpdatePath* FSDSql::getTitleUpdatePathByPathID( ULONGLONG TitleUpdatePathId)
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	TitleUpdatePath* retVal;

	try {
		ostringstream os;
		os << "SELECT TitleUpdatePathId, TitleUpdatePathDeviceId, MountedDeviceMountPoint, TitleUpdatePathDataPath, TitleUpdatePathCachePath, TitleUpdatePathContentPath ";
		os << "  FROM TitleUpdatePaths, MountedDevices ";
		os << "  WHERE TitleUpdatePathDeviceId = MountedDeviceDeviceId";
		os << "  AND TitleUpdatePathId = ?";
		pStmt.Sql(os.str());
		pStmt.BindInt64(1, TitleUpdatePathId);
		while (pStmt.FetchRow()) {
			int col = 0;
			ULONGLONG id = pStmt.GetColumnInt64(col++);
			string deviceID = pStmt.GetColumnString(col++);
			string root = pStmt.GetColumnString(col++);
			string dataPath = pStmt.GetColumnString(col++);
			string cachePath = pStmt.GetColumnString(col++);
			string contentPath = pStmt.GetColumnString(col++);
			if (root.compare("USBMUCache0:") == 0 ) {
				contentPath = sprintfaA("USBMU0:%s", contentPath.c_str());
			} else if (	root.compare("USBMUCache1:") == 0 ) {
				contentPath = sprintfaA("USBMU1:%s", contentPath.c_str());
			} else if (	root.compare("USBMUCache2:") == 0) {
				contentPath = sprintfaA("USBMU2:%s", contentPath.c_str());
			} else {
				contentPath = root + contentPath;
			}
			cachePath = root + cachePath;
						
			retVal = new TitleUpdatePath(id, deviceID, dataPath, cachePath, contentPath, root);
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getTitleUpdatePathByPathID: %s", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return retVal;
}

SQLTitleUpdate* FSDSql::getTitleUpdateByUpdateID( ULONGLONG titleUpdateId)
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);
	SQLTitleUpdate* retVal;
	try {
		ostringstream os;

		os.str("");
		os << "SELECT TitleUpdateId, TitleUpdateTitleId, TitleUpdateUpdateName, TitleUpdateFileName, MountedDeviceMountPoint, TitleUpdatePathCachePath, TitleUpdatePathContentPath, TitleUpdatePathDataPath, TitleUpdateTitleUpdatePathAdd, TitleUpdateIsBackup, TitleUpdateIsBackupForTitleUpdateId, TitleUpdateTitleUpdatePathType, TitleUpdateDisableId";
		os << "  FROM TitleUpdates, TitleUpdatePaths, MountedDevices  ";
		os << "  LEFT OUTER JOIN TitleUpdateDisables ON TitleUpdateId = TitleUpdateDisableTitleUpdateId AND TitleUpdateDisableProfileXuid = ?";
		os << "  WHERE TitleUpdatePathDeviceId = MountedDeviceDeviceId ";
		os << "  AND TitleUpdatePathId = TitleUpdateTitleUpdatePathId ";
		os << "  AND TitleUpdateId = ? ";

		pStmt.Sql(os.str());
		pStmt.BindInt64(1, titleUpdateId);
		while (pStmt.FetchRow()) {
			int col = 0;
			int titleId = pStmt.GetColumnInt(col++);
			ULONGLONG id = pStmt.GetColumnInt64(col++);
			wstring updateName = FromUtf8(pStmt.GetColumnString(col++));
			string filename = pStmt.GetColumnString(col++);
			string root = pStmt.GetColumnString(col++);
			string cachePath = pStmt.GetColumnString(col++);
			string contentPath = pStmt.GetColumnString(col++);
			string dataPath = pStmt.GetColumnString(col++);
			string addPath = pStmt.GetColumnString(col++);
			int isBackup = pStmt.GetColumnInt(col++);
			ULONGLONG isBackupFor = pStmt.GetColumnInt64(col++);
			int pathType = pStmt.GetColumnInt(col++);
			ULONGLONG disabled = pStmt.GetColumnInt64(col++);
			string fullpath;
			if (pathType == 0)
			{
				fullpath = dataPath;
				if (addPath != "")
				{
					fullpath = fullpath + "\\" + addPath;
				}
			} else if (pathType == 1)
			{
				fullpath = root + cachePath;
			} else if (pathType == 2)
			{
				if (root.compare("USBMUCache0:") == 0 ) {
					fullpath = sprintfaA("USBMU0:%s", contentPath.c_str());
				} else if (	root.compare("USBMUCache1:") == 0 ) {
					fullpath = sprintfaA("USBMU1:%ss", contentPath.c_str());
				} else if (	root.compare("USBMUCache2:") == 0) {
					fullpath = sprintfaA("USBMU2:%s", contentPath.c_str());
				} else {
					fullpath = root + contentPath;
				}
				fullpath = sprintfaA("%s\\%08X\\000B0000", fullpath.c_str(), titleId);
			}
			retVal = new SQLTitleUpdate(id, updateName, fullpath, pathType, filename, isBackup ? true : false, isBackupFor, disabled ? true : false);
		}
	}
	catch(Kompex::SQLiteException) // &exception)
	{
		// this title update exists alread.
	}
	FSD_UNLOCK;
	return retVal;
}
vector<SQLTitleUpdate*> FSDSql::getTitleUpdatesForTitleId(int playerId, DWORD titleId) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	vector<SQLTitleUpdate*> retVec;
	vector<SQLTitleUpdate*> delVec;

	string gamerProfile = GetGamerId(playerId);

	try {
		ostringstream os;
		os << "SELECT TitleUpdateId, TitleUpdateUpdateName, TitleUpdateFileName, MountedDeviceMountPoint, TitleUpdatePathCachePath, TitleUpdatePathContentPath, TitleUpdatePathDataPath, TitleUpdateTitleUpdatePathAdd, TitleUpdateIsBackup, TitleUpdateIsBackupForTitleUpdateId, TitleUpdateTitleUpdatePathType, TitleUpdateDisableId";
		os << "  FROM TitleUpdates, TitleUpdatePaths, MountedDevices  ";
		os << "  LEFT OUTER JOIN TitleUpdateDisables ON TitleUpdateId = TitleUpdateDisableTitleUpdateId AND TitleUpdateDisableProfileXuid = ?";
		os << "  WHERE TitleUpdatePathDeviceId = MountedDeviceDeviceId ";
		os << "  AND TitleUpdatePathId = TitleUpdateTitleUpdatePathId ";
		os << "  AND TitleUpdateTitleId = ? ";
		pStmt.Sql(os.str());
		pStmt.BindString(1, gamerProfile);
		pStmt.BindInt(2, titleId);
		while (pStmt.FetchRow()) {
			int col = 0;
			ULONGLONG id = pStmt.GetColumnInt64(col++);
			wstring updateName = FromUtf8(pStmt.GetColumnString(col++));
			string filename = pStmt.GetColumnString(col++);
			string root = pStmt.GetColumnString(col++);
			string cachePath = pStmt.GetColumnString(col++);
			string contentPath = pStmt.GetColumnString(col++);
			string dataPath = pStmt.GetColumnString(col++);
			string addPath = pStmt.GetColumnString(col++);
			int isBackup = pStmt.GetColumnInt(col++);
			ULONGLONG isBackupFor = pStmt.GetColumnInt64(col++);
			int pathType = pStmt.GetColumnInt(col++);
			ULONGLONG disabled = pStmt.GetColumnInt64(col++);
			
			string fullpath;
			if (pathType == 0)
			{
				fullpath = dataPath;
				if (addPath != "")
				{
					fullpath = fullpath + "\\" + addPath;
				}
			} else if (pathType == 1)
			{
				fullpath = root + cachePath;
			} else if (pathType == 2)
			{
				if (root.compare("USBMUCache0:") == 0 ) {
					fullpath = sprintfaA("USBMU0:%s", contentPath.c_str());
				} else if (	root.compare("USBMUCache1:") == 0 ) {
					fullpath = sprintfaA("USBMU1:%ss", contentPath.c_str());
				} else if (	root.compare("USBMUCache2:") == 0) {
					fullpath = sprintfaA("USBMU2:%s", contentPath.c_str());
				} else {
					fullpath = root + contentPath;
				}
				fullpath = sprintfaA("%s\\%08X\\000B0000", fullpath.c_str(), titleId);
			}
			SQLTitleUpdate* tu = new SQLTitleUpdate(id, updateName, fullpath, pathType, filename, isBackup ? true : false, isBackupFor, disabled ? true : false);
			if (FileExists(fullpath + "\\" + filename)) {
				retVec.push_back(tu);
			}
			else {
				delVec.push_back(tu);				
			}
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getTitleUpdatesForTitleId: %s", exception.GetString().c_str());
	}
	for (unsigned int x = 0; x < delVec.size(); x++)
	{
		deleteTitleUpdate(delVec.at(x)->titleUpdateId);
	}
	FSD_UNLOCK;
	return retVec;
}


vector<TitleUpdatePath*> FSDSql::getTitleUpdatePaths() {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	vector<TitleUpdatePath*> retVec;

	try {
		ostringstream os;
		os << "SELECT TitleUpdatePathId, TitleUpdatePathDeviceId, MountedDeviceMountPoint, TitleUpdatePathDataPath, TitleUpdatePathCachePath, TitleUpdatePathContentPath ";
		os << "  FROM TitleUpdatePaths, MountedDevices ";
		os << "  WHERE TitleUpdatePathDeviceId = MountedDeviceDeviceId";
		pStmt.Sql(os.str());
		while (pStmt.FetchRow()) {
			int col = 0;
			ULONGLONG id = pStmt.GetColumnInt64(col++);
			string deviceID = pStmt.GetColumnString(col++);
			string root = pStmt.GetColumnString(col++);
			string dataPath = pStmt.GetColumnString(col++);
			string cachePath = pStmt.GetColumnString(col++);
			string contentPath = pStmt.GetColumnString(col++);
			if (root.compare("USBMUCache0:") == 0 ) {
				contentPath = sprintfaA("USBMU0:%s", contentPath.c_str());
			} else if (	root.compare("USBMUCache1:") == 0 ) {
				contentPath = sprintfaA("USBMU1:%s", contentPath.c_str());
			} else if (	root.compare("USBMUCache2:") == 0) {
				contentPath = sprintfaA("USBMU2:%s", contentPath.c_str());
			} else {
				contentPath = root + contentPath;
			}
			cachePath = root + cachePath;
						
			TitleUpdatePath* tu = new TitleUpdatePath(id, deviceID, dataPath, cachePath, contentPath, root);
			retVec.push_back(tu);
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getTitleUpdatePaths: %s", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return retVec;
}

ULONGLONG FSDSql::addTitleUpdate(DWORD titleId, const wstring& name, const string& filename, const string& addPath, int isBackup, ULONGLONG isBackupFor, ULONGLONG titleUpdatePathId, int titleUpdatePathType)
{
	
	ULONGLONG id = getTitleUpdateID(titleId, name, filename, addPath, isBackup, isBackupFor, titleUpdatePathId, titleUpdatePathType);
	if (id == -1) {
		FSD_LOCK;
		Kompex::SQLiteDatabase *pDatabase = getDatabase();
		Kompex::SQLiteStatement pStmt(pDatabase);
		
		try {
			ostringstream os;

			os.str("");
			os << "INSERT INTO TitleUpdates (TitleUpdateTitleId, TitleUpdateUpdateName, TitleUpdateFileName, TitleUpdateTitleUpdatePathAdd, TitleUpdateIsBackup, TitleUpdateIsBackupForTitleUpdateId, TitleUpdateTitleUpdatePathId, TitleUpdateTitleUpdatePathType) ";
			os << " VALUES (?,?,?,?,?,?,?,?) ";

			pStmt.Sql(os.str());
			pStmt.BindInt(1, titleId);
			pStmt.BindString(2, ToUtf8(name));
			pStmt.BindString(3, filename);
			pStmt.BindString(4, addPath);
			pStmt.BindInt(5, isBackup);
			pStmt.BindInt64(6, isBackupFor);
			pStmt.BindInt64(7, titleUpdatePathId);
			pStmt.BindInt(8, titleUpdatePathType);
			pStmt.ExecuteAndFree();

			id = pDatabase->GetLastInsertRowId();
		}
		catch(Kompex::SQLiteException) // &exception)
		{
			// this title update exists already
		}

		FSD_UNLOCK;
	}
	return id;
}

ULONGLONG FSDSql::getTitleUpdateID(DWORD titleId, const wstring& name, const string& filename, const string& addPath, int isBackup, ULONGLONG isBackupFor, ULONGLONG titleUpdatePathId, int titleUpdatePathType)
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);
	ULONGLONG id = -1;
	try {
		ostringstream os;

		os.str("");
		os << "SELECT TitleUpdateId FROM TitleUpdates ";
		os << " WHERE TitleUpdateTitleId = ?";
		os << " AND TitleUpdateUpdateName = ?";
		os << " AND TitleUpdateFileName = ?";
		os << " AND TitleUpdateTitleUpdatePathAdd = ?";
		os << " AND TitleUpdateTitleUpdatePathId = ?";
		os << " AND TitleUpdateTitleUpdatePathType = ?";
		os << " AND TitleUpdateIsBackup = ?";
		os << " AND TitleUpdateIsBackupForTitleUpdateId = ?";

		pStmt.Sql(os.str());
		pStmt.BindInt(1, titleId);
		pStmt.BindString(2, ToUtf8(name));
		pStmt.BindString(3, filename);
		pStmt.BindString(4, addPath);
		pStmt.BindInt64(5, titleUpdatePathId);
		pStmt.BindInt(6, titleUpdatePathType);
		pStmt.BindInt(7, isBackup);
		pStmt.BindInt64(8, isBackupFor);
		while (pStmt.FetchRow()) {
			int col = 0;
			id = pStmt.GetColumnInt64(col++);
		}
	}
	catch(Kompex::SQLiteException) // &exception)
	{
		// this title update exists alread.
	}
	FSD_UNLOCK;
	return id;	
}

void FSDSql::updateTitleDisabled(int playerId, ULONGLONG TitleUpdateId, bool isDisabled) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();

	Kompex::SQLiteStatement pStmt(pDatabase);

	string gamerProfileId = GetGamerId(playerId);
	ostringstream q;

	try {
		if (isDisabled) 
			q << "INSERT OR REPLACE INTO TitleUpdateDisables (TitleUpdateDisableTitleUpdateId, TitleUpdateDisableProfileXuid) VALUES (" << TitleUpdateId << ", '" << gamerProfileId << "')";
		else
			q << "DELETE FROM TitleUpdateDisables WHERE TitleUpdateDisableTitleUpdateId = " << TitleUpdateId << " AND TitleUpdateDisableProfileXuid = '" << gamerProfileId << "'";
	}
	catch(Kompex::SQLiteException &exception)
	{
		// this device is already in the table...
		DebugMsg("FSDSql", "updateTitleDisabled", exception.GetString().c_str());
	}

	pStmt.SqlStatement(q.str());	

	FSD_UNLOCK;
}


void FSDSql::addTitleUpdatePath(const string& deviceId, const string& dataPath, const string& cachePath, const string& contentPath) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream os;

		os.str("");
		os << "INSERT INTO TitleUpdatePaths (TitleUpdatePathDeviceId, TitleUpdatePathCachePath, TitleUpdatePathContentPath, TitleUpdatePathDataPath) ";
		os << " VALUES (?,?,?,?) ";

		pStmt.Sql(os.str());
		pStmt.BindString(1, deviceId);
		pStmt.BindString(2, cachePath);
		pStmt.BindString(3, contentPath);
		pStmt.BindString(4, dataPath);
		pStmt.ExecuteAndFree();

	}
	catch(Kompex::SQLiteException) // &exception)
	{
		// this device is already in the table...
		//DebugMsg("FSDSql", "addTitleUpdatePath", exception.GetString().c_str());
	}
	

	FSD_UNLOCK;
}

void FSDSql::createSchema(long currentSchemaVersion, long currentSettingsSchemaVersion) {
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);
	
	bool db_is_empty = false;

	if (currentSchemaVersion == 0) { // either we are dealing with an empty DB, or a version 1 schema when the version was in a table
		try	
		{
			pStmt.Sql("SELECT version FROM schemaVersion");
			if(pStmt.FetchRow()) {
				currentSchemaVersion = 1;
				int v = pStmt.GetColumnInt(0);			// the value doesn't matter...this must be a version 1 schema
				DebugMsg("FSDSql","Read a version of %d", v);
				pStmt.FreeQuery();
			}
		}
		catch(Kompex::SQLiteException &exception)
		{
			db_is_empty = true;
			DebugMsg("FSDSql", "schemaVersion table not there", exception.GetString().c_str());
		}
	}

	if (db_is_empty) { 
		try	
		{
			pStmt.SqlStatement( createAssetTypes);
			pStmt.SqlStatement( createAssetFileTypes);
			pStmt.SqlStatement( createTabs);
//			pStmt.SqlStatement( createScanPahts);
			pStmt.SqlStatement( createContentTypes);
			pStmt.SqlStatement( createFavorites);
			pStmt.SqlStatement( createRecentlyPlayedTitles);
			pStmt.SqlStatement( createUserRatings);
			pStmt.SqlStatement( createAssets);
			pStmt.SqlStatement( createContentItems);
			pStmt.SqlStatement( createContentItemsIndex);

			// NOW POPULATE tables that require it.

			pStmt.BeginTransaction();
			pStmt.SqlStatement("DELETE FROM AssetTypes");
			pStmt.SqlStatement("INSERT INTO AssetTypes (AssetTypeId, AssetTypeName, AssetTypeFolder) VALUES (0, 'Icon', 'Icons')");
			pStmt.SqlStatement("INSERT INTO AssetTypes (AssetTypeId, AssetTypeName, AssetTypeFolder) VALUES (1, 'BoxCover', 'BoxCovers')");
			pStmt.SqlStatement("INSERT INTO AssetTypes (AssetTypeId, AssetTypeName, AssetTypeFolder) VALUES (2, 'Background', 'Backgrounds')");
			pStmt.SqlStatement("INSERT INTO AssetTypes (AssetTypeId, AssetTypeName, AssetTypeFolder) VALUES (3, 'Banner', 'Banners')");
			pStmt.SqlStatement("INSERT INTO AssetTypes (AssetTypeId, AssetTypeName, AssetTypeFolder) VALUES (4, 'ScreentShot', 'Screenshots')");
			pStmt.SqlStatement("INSERT INTO AssetTypes (AssetTypeId, AssetTypeName, AssetTypeFolder) VALUES (5, 'Video', 'PreviewVideos')");
			
			pStmt.SqlStatement("DELETE FROM AssetFileTypes");
			pStmt.SqlStatement("INSERT INTO AssetFileTypes (AssetFileTypeId, AssetFileTypeName) VALUES (0, 'jpg')");
			pStmt.SqlStatement("INSERT INTO AssetFileTypes (AssetFileTypeId, AssetFileTypeName) VALUES (1, 'png')");
			pStmt.SqlStatement("INSERT INTO AssetFileTypes (AssetFileTypeId, AssetFileTypeName) VALUES (2, 'dds')");
			pStmt.SqlStatement("INSERT INTO AssetFileTypes (AssetFileTypeId, AssetFileTypeName) VALUES (3, 'asx')");
			pStmt.SqlStatement("INSERT INTO AssetFileTypes (AssetFileTypeId, AssetFileTypeName) VALUES (4, 'wmv')");

			pStmt.SqlStatement("DELETE FROM Tabs");	

			pStmt.SqlStatement("INSERT INTO Tabs (TabId, TabName) VALUES (1, 'XBLA')");
			pStmt.SqlStatement("INSERT INTO Tabs (TabId, TabName) VALUES (2, 'Xbox 360')");
			pStmt.SqlStatement("INSERT INTO Tabs (TabId, TabName) VALUES (3, 'Xbox Classic')");
			pStmt.SqlStatement("INSERT INTO Tabs (TabId, TabName) VALUES (4, 'Homebrew')");
			pStmt.SqlStatement("INSERT INTO Tabs (TabId, TabName) VALUES (5, 'Emulator')");

			pStmt.CommitTransaction();
			currentSchemaVersion = 1;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR: %s", exception.GetString().c_str());
			return;
		}
	}
	if(currentSchemaVersion == 1) {
		// do the work here to change a version 1 schema into a version 2 schema
		try	
		{
			pStmt.SqlStatement("DROP TABLE IF EXISTS schemaVersion");

//			pStmt.SqlStatement( createTitleUpdatePaths);
//			pStmt.SqlStatement( createTitleUpdates);
//			pStmt.SqlStatement( createKaiUsers);
			pStmt.SqlStatement( createKaiMessages);
			pStmt.SqlStatement( createKaiContacts);
			pStmt.SqlStatement( createKaiUserToContacts);

			currentSchemaVersion = 2;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading schema verson 1 to version 2: %s", exception.GetString().c_str());
			return;
		}
	}
	if(currentSchemaVersion == 2) {
		// do the work here to change a version 2 schema into a version 3 schema
		try	
		{
			pStmt.SqlStatement("DROP TABLE IF EXISTS main.KaiUsers");

			pStmt.SqlStatement( createKaiUsersV3);
			pStmt.SqlStatement( createProfileXuids);

			currentSchemaVersion = 3;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading schema verson 2 to version 3: %s", exception.GetString().c_str());
			return;
		}		
	}
	if(currentSchemaVersion == 3) {
		// do the work here to change a version 3 schema into a version 4 schema
		try	
		{
			//pStmt.SqlStatement("DROP TABLE IF EXISTS main.TitleUpdatePaths");

			//pStmt.SqlStatement( createTitleUpdatePathsV4 );
			pStmt.SqlStatement( createTitleUpdateDisables);

			currentSchemaVersion = 4;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading schema verson 3 to version 4: %s", exception.GetString().c_str());
			return;
		}		
	}
	if(currentSchemaVersion == 4) {
		try	
		{
			pStmt.SqlStatement("DROP TABLE IF EXISTS main.ScanPaths");
			pStmt.SqlStatement( createScanPathsV5 );
			currentSchemaVersion = 5;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading schema verson 4 to version 5: %s", exception.GetString().c_str());
			return;
		}		

	}
	if(currentSchemaVersion == 5) {
		try {
			pStmt.SqlStatement("INSERT INTO Tabs (TabId, TabName) VALUES (0, 'Automatic')");			
			currentSchemaVersion = 6;
		} 
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading schema verson 5 to version 6: %s", exception.GetString().c_str());
			return;
		}
	}

	if(currentSchemaVersion == 6) {
		try {
			
			// First Let's Create the new ContentItems table with the new Constraints
			pStmt.SqlStatement(createContentItemsV6);
			
			ostringstream q;
			q << "SELECT ContentItemId,          ContentItemScanPathId,  ContentItemFileType,";		//2
			q << "       ContentItemContentType, ContentItemTab,         ContentItemPath,";			//5
			q << "       ContentItemDirectory,   ContentItemFileName,    ContentItemTitleId,";		//8
			q << "       ContentItemMediaId,     ContentItemDiscNum,     ContentItemDiscsInSet,";	//11
			q << "       ContentItemName,        ContentItemDescription, ContentItemDeveloper,";	//14
			q << "       ContentItemPublisher,   ContentItemGenre,       ContentItemRating,";		//17
			q << "       ContentItemRaters,      ContentItemReleaseDate ";							//19
			q << "  FROM ContentItems ";

			// Set our statement
			pStmt.Sql(q.str());
			vector<CONTENT_ITEM_LOAD_INFO> contentItemsInfo;
			while(pStmt.FetchRow()) {
				// Create our item
				CONTENT_ITEM_LOAD_INFO loadInfo;
				ZeroMemory(&loadInfo, sizeof(CONTENT_ITEM_LOAD_INFO));

				// Now let's fill our structs 
				loadInfo.ContentId     	          = pStmt.GetColumnInt(0);
				loadInfo.ItemInfo.ItemScanPathId  = pStmt.GetColumnInt(1);
				loadInfo.ItemInfo.FileType		  = (CONTENT_FILE_TYPE)pStmt.GetColumnInt(2);
				loadInfo.ItemInfo.ContentType	  = pStmt.GetColumnInt(3);
				loadInfo.ItemInfo.ItemTab         = (CONTENT_ITEM_TAB)pStmt.GetColumnInt(4);
				loadInfo.ItemInfo.ItemPath		  = pStmt.GetColumnString(5);
				loadInfo.ItemInfo.ItemDirectory   = pStmt.GetColumnString(6);
				loadInfo.ItemInfo.ItemFileName	  = pStmt.GetColumnString(7);
				loadInfo.ItemInfo.TitleId		  = pStmt.GetColumnInt(8);
				loadInfo.ItemInfo.TitleMediaId	  = pStmt.GetColumnInt(9);
				loadInfo.ItemInfo.TitleDiscNum	  = pStmt.GetColumnInt(10);
				loadInfo.ItemInfo.TitleDiscsInSet = pStmt.GetColumnInt(11);
				
				// Display Info
				loadInfo.DisplayInfo.TitleName		  = FromUtf8(pStmt.GetColumnString(12));
				loadInfo.DisplayInfo.TitleDescription = FromUtf8(pStmt.GetColumnString(13));
				loadInfo.DisplayInfo.TitleDeveloper	  = FromUtf8(pStmt.GetColumnString(14));
				loadInfo.DisplayInfo.TitlePublisher	  = FromUtf8(pStmt.GetColumnString(15));
				loadInfo.DisplayInfo.TitleGenre		  = FromUtf8(pStmt.GetColumnString(16));
				loadInfo.DisplayInfo.TitleRating	  = FromUtf8(pStmt.GetColumnString(17));
				loadInfo.DisplayInfo.TitleRaters	  = FromUtf8(pStmt.GetColumnString(18));
				loadInfo.DisplayInfo.TitleReleaseDate = FromUtf8(pStmt.GetColumnString(19));

				// Information is Loaded, now lets modify our values
				string szOriginalPath = loadInfo.ItemInfo.ItemPath;
				if(szOriginalPath.find_first_of(":") != string::npos) 
				{
					string szModifiedPath = szOriginalPath.substr(szOriginalPath.find_first_of(":") + 1);
					loadInfo.ItemInfo.ItemPath = szModifiedPath;
				}
				string szOriginalDirectory = loadInfo.ItemInfo.ItemDirectory;
				if(szOriginalDirectory.find_first_of(":") != string::npos)
				{
					string szModifiedDirectory = szOriginalDirectory.substr(szOriginalDirectory.find_first_of(":") + 1);
					loadInfo.ItemInfo.ItemDirectory = szModifiedDirectory;
				}


				contentItemsInfo.push_back(loadInfo);
			};
			unsigned int nContentCount = contentItemsInfo.size();
			pStmt.FreeQuery();

			// Now that we have a list of all of our content, we are going to create the fill the new table
			
			for( unsigned int x = 1; x < nContentCount + 1; x++ )
			{
				q.str("");
				q << "INSERT INTO TempContentItems (";
				q << "  ContentItemId,          ContentItemScanPathId,  ContentItemFileType,   ContentItemContentType,";
				q << "  ContentItemTab,         ContentItemPath,        ContentItemDirectory,  ContentItemFileName,";
				q << "  ContentItemTitleId,     ContentItemMediaId,     ContentItemDiscNum,    ContentItemDiscsInSet,";
				q << "  ContentItemName,        ContentItemDescription, ContentItemDeveloper,  ContentItemPublisher,";
				q << "  ContentItemGenre,       ContentItemRating,      ContentItemRaters,     ContentItemReleaseDate";
				q << "  ) VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )";

				// Grab the current item to process
				CONTENT_ITEM_SAVE_INFO SaveInfo = contentItemsInfo.at(x - 1);

				pStmt.Sql(q.str());
				pStmt.BindInt(1, x);
				pStmt.BindInt(2, SaveInfo.ItemInfo.ItemScanPathId);
				pStmt.BindInt(3, SaveInfo.ItemInfo.FileType);
				pStmt.BindInt(4, SaveInfo.ItemInfo.ContentType);
				pStmt.BindInt(5, SaveInfo.ItemInfo.ItemTab);
				pStmt.BindString(6, SaveInfo.ItemInfo.ItemPath);
				pStmt.BindString(7, SaveInfo.ItemInfo.ItemDirectory);
				pStmt.BindString(8, SaveInfo.ItemInfo.ItemFileName);		
				pStmt.BindInt(9, SaveInfo.ItemInfo.TitleId);
				pStmt.BindInt(10, SaveInfo.ItemInfo.TitleMediaId);
				pStmt.BindInt(11, SaveInfo.ItemInfo.TitleDiscNum);
				pStmt.BindInt(12, SaveInfo.ItemInfo.TitleDiscsInSet);
				pStmt.BindString(13, ToUtf8(SaveInfo.DisplayInfo.TitleName));
				pStmt.BindString(14, ToUtf8(SaveInfo.DisplayInfo.TitleDescription));
				pStmt.BindString(15, ToUtf8(SaveInfo.DisplayInfo.TitleDeveloper));
				pStmt.BindString(16, ToUtf8(SaveInfo.DisplayInfo.TitlePublisher));
				pStmt.BindString(17, ToUtf8(SaveInfo.DisplayInfo.TitleGenre));
				pStmt.BindString(18, ToUtf8(SaveInfo.DisplayInfo.TitleRating));
				pStmt.BindString(19, ToUtf8(SaveInfo.DisplayInfo.TitleRaters));
				pStmt.BindString(20, ToUtf8(SaveInfo.DisplayInfo.TitleReleaseDate));
				pStmt.ExecuteAndFree();
			}

			// Drop the previous table, and renmae the temp table for a complete replacement
			pStmt.SqlStatement(" DROP TABLE IF EXISTS ContentItems; ");
			pStmt.SqlStatement(" ALTER TABLE TempContentItems RENAME TO ContentItems; ");
			
			currentSchemaVersion = 7;
		} 
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading schema verson 6 to version 7: %s", exception.GetString().c_str());
			return;
		}
	}

	if(currentSchemaVersion == 7) {
		// do the work here to change a version 3 schema into a version 4 schema
		try	
		{
			pStmt.SqlStatement("DROP TABLE IF EXISTS Settings.TitleUpdatePaths");
			pStmt.SqlStatement("DROP TABLE IF EXISTS main.TitleUpdates");

			pStmt.SqlStatement( createTitleUpdatePathsV7 );
			pStmt.SqlStatement( createTitleUpdatesV7);

			currentSchemaVersion = 8;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading schema verson 7 to version 8: %s", exception.GetString().c_str());
			return;
		}		
	}

	if(currentSchemaVersion == 8) {
		// do the work here to change a version 3 schema into a version 4 schema
		try	
		{
			
			// First let's add a new column to all the currently existing content items, and set it default value to 0
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE ContentItems ADD ContentItemHash TEXT NOT NULL DEFAULT('0')");
			pStmt.CommitTransaction();
			
			// Next let's create our http queue table
			pStmt.SqlStatement(createHttpQueueTable);

			currentSchemaVersion = 9;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading schema verson 8 to version 9: %s", exception.GetString().c_str());
			return;
		}		
	}

	//if(currentSchemaVersion == 8) {
		// Do work to update database to schemaversion 9
		// currentSchemaVersion = 9;
	//}

//Now deal with the Settings table
	
	bool bResetSettingsToDefault = false;  // there is only one situation where this is acceptable

	if(currentSettingsSchemaVersion == 0) {
		// do the work here to change a version 0 schema into a version 1 schema
		try	
		{
			pStmt.SqlStatement( createGlobalSettings );
			currentSettingsSchemaVersion = 1;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 0 to version 1: %s", exception.GetString().c_str());
			return;
		}		
	}

	if(currentSettingsSchemaVersion == 1) {
		try {
			// sqlite doesn't have ALTER TABLE DROP COLUMN, so we have to do all this to drop the Skin column that used to be here
			
			pStmt.BeginTransaction();
			ostringstream os;
			pStmt.SqlStatement(createGlobalSettingsV2_temp);
			os.str("");
			os << "INSERT INTO Settings.GlobalSettings_TEMP SELECT ";
			os << " GlobalSettingVideoPath, GlobalSettingMusicPath, GlobalSettingDVDCopy360Path, GlobalSettingDVDCopyOrigPath, ";
			os << " GlobalSettingWeatherLocation,GlobalSettingHOverscan,GlobalSettingVOverscan,GlobalSettingHDisplacement, ";
			os << " GlobalSettingVDisplacement,GlobalSettingLetterbox,GlobalSettingFtpPort,GlobalSettingFtpUser,GlobalSettingFtpPassword, ";
			os << " GlobalSettingFanSpeed,GlobalSettingLEDFormat,GlobalSettingTimeFormat,GlobalSettingDateFormat,GlobalSettingNTPOnStartup, ";
			os << " GlobalSettingUpdateXEXNotice,GlobalSettingUpdateSkinNotice,GlobalSettingUpdatePluginNotice,GlobalSettingUpdateOnBeta, ";
			os << " GlobalSettingInstallerState ";
			os << " FROM Settings.GlobalSettings;";
			pStmt.SqlStatement(os.str());

			pStmt.SqlStatement("DROP TABLE Settings.GlobalSettings;");
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings_TEMP RENAME TO GlobalSettings;");
			
			pStmt.SqlStatement(createUserSettings);
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 2;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 1 to version 2: %s", exception.GetString().c_str());
			return;
		}			
	}

	if(currentSettingsSchemaVersion == 2) {
		try {
			pStmt.BeginTransaction();
			pStmt.SqlStatement( createGameListSorts );
			pStmt.SqlStatement( createGameListSortTypes );
			pStmt.SqlStatement( createGameListSortDirections );

			pStmt.SqlStatement("DELETE FROM GameListSortTypes");
			pStmt.SqlStatement("INSERT INTO GameListSortTypes (GameListSortTypeId, GameListSortType) VALUES (0, 'Title')");
			pStmt.SqlStatement("INSERT INTO GameListSortTypes (GameListSortTypeId, GameListSortType) VALUES (1, 'Rating')");
			pStmt.SqlStatement("INSERT INTO GameListSortTypes (GameListSortTypeId, GameListSortType) VALUES (2, '# of Ratings')");
			pStmt.SqlStatement("INSERT INTO GameListSortTypes (GameListSortTypeId, GameListSortType) VALUES (3, 'Release Date')");
			pStmt.SqlStatement("INSERT INTO GameListSortTypes (GameListSortTypeId, GameListSortType) VALUES (4, 'Last Played')");
			pStmt.SqlStatement("INSERT INTO GameListSortTypes (GameListSortTypeId, GameListSortType) VALUES (5, 'User Rating')");

			pStmt.SqlStatement("DELETE FROM GameListSortDirections");	
			pStmt.SqlStatement("INSERT INTO GameListSortDirections (GameListSortDirectionId, GameListSortDirectionType) VALUES (0, 'Forward')");
			pStmt.SqlStatement("INSERT INTO GameListSortDirections (GameListSortDirectionId, GameListSortDirectionType) VALUES (1, 'Backward')");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 3;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 2 to version 3: %s", exception.GetString().c_str());
			return;
		}	
	}

	if(currentSettingsSchemaVersion == 3) {
		try {
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingFsdUsername TEXT");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingFsdPassword TEXT");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingFsdUseLogin INTEGER");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 4;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 3 to version 4: %s", exception.GetString().c_str());
			return;
		}	
	}


	if(currentSettingsSchemaVersion == 4) {
		try {

			pStmt.BeginTransaction();
			pStmt.SqlStatement( alterGlobalSettings );
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 5;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 4 to version 5: %s", exception.GetString().c_str());
			return;
		}	
	}

	if(currentSettingsSchemaVersion == 5) {
		try {
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingGameListBackground INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingGameListVisual INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingMainTempStatus INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingMainDiskStatus INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingMainUseCelsius INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingMainShowIp INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingMainShowStatus INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingMainShowAvatar INTEGER");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 6;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 5 to version 6: %s", exception.GetString().c_str());
			return;
		}	
	}
	if(currentSettingsSchemaVersion == 6) {
		try {

			pStmt.BeginTransaction();
			pStmt.SqlStatement( alterGlobalSettingsV2);
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 7;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 6 to version 7: %s", exception.GetString().c_str());
			return;
		}	
	}

	if(currentSettingsSchemaVersion == 7) {
		try {
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingKaiAutoLogin INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingKaiLinkProfile INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingKaiNoticeOnline INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingKaiNoticeOffline INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingKaiNoticeInvite INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingKaiNoticeContactPM INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingKaiNoticeOpponentPM INTEGER");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingKaiServerIp TEXT");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingKaiUsername TEXT");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingKaiPassword TEXT");
			pStmt.CommitTransaction();

			// Write default settings into db, but this is the last time we can do it this way, since we have to protect user's settings
			bResetSettingsToDefault = true; 
			
			currentSettingsSchemaVersion = 8;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 7 to version 8: %s", exception.GetString().c_str());
			return;
		}	
	}

	if(currentSettingsSchemaVersion == 8) {
		try {
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GameListSorts ADD GameListSortFavorites INTEGER");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 9;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 8 to version 9: %s", exception.GetString().c_str());
			return;
		}	
	}

	if(currentSettingsSchemaVersion == 9) {
		try {
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingFtpServerOn INTEGER NOT NULL DEFAULT(1)");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingHttpServerOn INTEGER NOT NULL DEFAULT(1)");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 10;
		}
		catch(Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 9 to version 10: %s", exception.GetString().c_str());
			return;
		}	
	}

	if(currentSettingsSchemaVersion == 10)
	{
		try 
		{
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingShowStatusAtBoot INTEGER NOT NULL DEFAULT(0)");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 11;
		} 
		catch( Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 10 to version 11: %s", exception.GetString().c_str());
			return;
		}
	}

	if(currentSettingsSchemaVersion == 11)
	{
		try 
		{
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingDownloadIcon INTEGER NOT NULL DEFAULT(1)");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingDownloadBoxart INTEGER NOT NULL DEFAULT(1)");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingDownloadBanner INTEGER NOT NULL DEFAULT(1)");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingDownloadBackground INTEGER NOT NULL DEFAULT(1)");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingDownloadVideo INTEGER NOT NULL DEFAULT(0)");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingDownloadScreenshots INTEGER NOT NULL DEFAULT(1)");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingMaxScreenshots INTEGER NOT NULL DEFAULT(15)");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingHTTPDisableSecurity INTEGER NOT NULL DEFAULT(0)");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingHttpUsername TEXT NOT NULL DEFAULT('fsdhttp')");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingHttpPassword TEXT NOT NULL DEFAULT('fsdhttp')");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 12;
		} 
		catch( Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 11 to version 12: %s", exception.GetString().c_str());
			return;
		}
	}

	if(currentSettingsSchemaVersion == 12)
	{
		try 
		{
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingGameListTransition INTEGER NOT NULL DEFAULT(0)");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 13;
		} 
		catch( Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 12 to version 13: %s", exception.GetString().c_str());
			return;
		}
	}
	if(currentSettingsSchemaVersion == 13)
	{
		try 
		{
			pStmt.BeginTransaction();
			pStmt.SqlStatement(createSambaClientCredentials);
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingSambaClientOn INTEGER NOT NULL DEFAULT(0)");
			pStmt.CommitTransaction();


			currentSettingsSchemaVersion = 14;
		} 
		catch( Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 13 to version 14: %s", exception.GetString().c_str());
			return;
		}
	}
	if(currentSettingsSchemaVersion == 14)
	{
		try 
		{
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingWeatherLocName TEXT NOT NULL DEFAULT('undefined')");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingResumeAssetDownload INTEGER NOT NULL DEFAULT(1)");
			pStmt.CommitTransaction();

			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingDisableScanAtBoot INTEGER NOT NULL DEFAULT(0)");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 15;
		}
		catch( Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 14 to version 15:  %s", exception.GetString().c_str());
			return;
		}
	}
	if(currentSettingsSchemaVersion == 15)
	{
		try 
		{
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingWeatherUnits INTEGER NOT NULL DEFAULT(0)");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 16;
		}
		catch( Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 15 to version 16:  %s", exception.GetString().c_str());
			return;
		}
	}
	if(currentSettingsSchemaVersion == 16)
	{
		try 
		{
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingSambaClientWorkgroup TEXT NOT NULL DEFAULT('WORKGROUP')");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 17;
		}
		catch( Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 16 to version 17:  %s", exception.GetString().c_str());
			return;
		}
	}
	if(currentSettingsSchemaVersion == 17)
	{
		try 
		{
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingSambaServerOn INTEGER NOT NULL DEFAULT(0)");
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingHostname TEXT NOT NULL DEFAULT('XBOX360')");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 18;
		} 
		catch( Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 17 to version 18: %s", exception.GetString().c_str());
			return;
		}
	}

	if(currentSettingsSchemaVersion == 18)
	{
		try 
		{
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingAdvancedModeOn INTEGER NOT NULL DEFAULT(0)");
			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 19;
		} 
		catch( Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 18 to version 19: %s", exception.GetString().c_str());
			return;
		}
	}

	if(currentSettingsSchemaVersion == 19)
	{
		try 
		{
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingCoverPollInterval INTEGER NOT NULL DEFAULT(0)");
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingCoverUserName TEXT NOT NULL DEFAULT('')");
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingCoverPassword TEXT NOT NULL DEFAULT('')");
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingCoverCheck INT NOT NULL DEFAULT(1)");

			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 20;
		} 
		catch( Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 19 to version 20: %s", exception.GetString().c_str());
			return;
		}
	}
	if(currentSettingsSchemaVersion == 20)
	{
		try 
		{
			pStmt.BeginTransaction();
			pStmt.SqlStatement("ALTER TABLE Settings.GlobalSettings ADD GlobalSettingCoverQuality INTEGER NOT NULL DEFAULT(0)");

			pStmt.CommitTransaction();

			currentSettingsSchemaVersion = 21;
		} 
		catch( Kompex::SQLiteException &exception)
		{
			DebugMsg("FSDSql", "ERROR while upgrading Settings schema version 20 to version 21: %s", exception.GetString().c_str());
			return;
		}
	}
	if (bResetSettingsToDefault)
		SETTINGS::getInstance().ResetToDefaultSettings();


// create temporary tables
	try	
	{
		pStmt.SqlStatement("DROP TABLE IF EXISTS MountedDevices");
		ostringstream os;
		os << "CREATE TABLE MountedDevices ( ";
		os << "  MountedDeviceDeviceId TEXT, ";
		os << "  MountedDeviceMountPoint TEXT ";
		os << ")";
		pStmt.SqlStatement(os.str());
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "ERROR while creating temp tables: %s", exception.GetString().c_str());
		return;
	}

	ostringstream os;
	os << "PRAGMA user_version=" << currentSchemaVersion;
	sqlite3_stmt*  sqlStmt = NULL;	
	int ret = sqlite3_prepare_v2(pDatabase->GetDatabaseHandle(), os.str().c_str(), -1, &sqlStmt, NULL); 
	if( ret == SQLITE_OK ) { 
		sqlite3_step(sqlStmt); 
		sqlite3_finalize(sqlStmt); 
	} 

	os.str("");
	os << "PRAGMA Settings.user_version=" << currentSettingsSchemaVersion;
	sqlStmt = NULL;	
	ret = sqlite3_prepare_v2(pDatabase->GetDatabaseHandle(), os.str().c_str(), -1, &sqlStmt, NULL); 
	if( ret == SQLITE_OK ) { 
		sqlite3_step(sqlStmt); 
		sqlite3_finalize(sqlStmt); 
	} 
}

void FSDSql::DumpTable(string tableName) {
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	ostringstream q;
	q << "SELECT * FROM " << tableName;

	pStmt.Sql(q.str());
	bool bFirst = true;
	q.str("");
	while (pStmt.FetchRow()) {
		if (bFirst) {
			for (int j = 0; j < pStmt.GetColumnCount(); j++) {
				q << setw(10) << pStmt.GetColumnName(j) << " ! ";
			}
			DebugMsg("FSDSql1",q.str().c_str());
			bFirst = false;
		}
		q.str("");
		for (int j = 0; j < pStmt.GetColumnCount(); j++) {
			bool isNull;
			q << setw(10) << pStmt.GetColumnString(j, isNull) << " ! ";
		}
		DebugMsg("FSDSql2",q.str().c_str());

	}
	pStmt.FreeQuery();
}

string FSDSql::fixString(string& in) {
	return str_replaceallA(in, "'", "''");
}

wstring FSDSql::fixStringW(wstring& in) {
	return str_replaceallW(in, L"'", L"''");
}


void FSDSql::updateRecentlyPlayed(int playerId, ULONGLONG contentId) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();

	Kompex::SQLiteStatement pStmt(pDatabase);

	ULONGLONG playedOrder = 1;
	string gamerProfileId = GetGamerId(playerId);

	SYSTEMTIME SysTime;
	GetSystemTime( &SysTime );

	time_t time;
	SystemTimeToUnixTime(&SysTime, &time); 

	DebugMsg("FsdSQL", "Asked to update recently played for %d", contentId);
//	DumpTable("lastPlayed");

	// first, go find the last most recently used by playedOrder
	ostringstream q;
	q << "SELECT RecentlyPlayedTitleOrder, RecentlyPlayedTitleContentId from RecentlyPlayedTitles WHERE ";
	q << "  RecentlyPlayedTitleOrder = (SELECT MAX(RecentlyPlayedTitleOrder) FROM RecentlyPlayedTitles WHERE RecentlyPlayedTitleGamerProfile = '" << gamerProfileId << "') ";
	q << "  AND RecentlyPlayedTitleGamerProfile = '" << gamerProfileId << "'";

	pStmt.Sql(q.str());
	if (pStmt.FetchRow()) {
		playedOrder = pStmt.GetColumnInt64(0);
		ULONGLONG id = pStmt.GetColumnInt64(1);
		if (id != contentId) {  // this contentId wasn't most recently played, so make it so
			playedOrder += 1;
		}
		else if (playedOrder == 0) {
			playedOrder = 1;		// old bug where the first one was assigned 0
		}
	}
	pStmt.FreeQuery();

	char newPlayedOrder[17];
	sprintf_s(newPlayedOrder, 17, "%016I64d", playedOrder);

	q.str("");
	q << "INSERT OR REPLACE INTO RecentlyPlayedTitles (RecentlyPlayedTitleContentId, RecentlyPlayedTitleGamerProfile, RecentlyPlayedTitleDateTime, RecentlyPlayedTitleOrder) VALUES ";
	q << "('" << contentId << "', '" << gamerProfileId << "', '" << time << "', '" << newPlayedOrder << "' )";
	DebugMsg("FSDSql", "Query: %s", q.str().c_str());

	pStmt.SqlStatement(q.str());

	/*
	// test it out;
	ULONGLONG x;
	SYSTEMTIME t;
	bool worked = getRecentlyPlayed(playerId, contentId, &x, &t);
	if (worked) {
		wstring ws = SystemTimeToLocalString(t);
		DebugMsg("FSDSql", "Re-read a date of [%s]", wstrtostr(ws).c_str());
	}
	else {
		DebugMsg("FSDSql", "couldn't fetch updated time?");
	}*/
	FSD_UNLOCK;
}

vector<RecentlyPlayed> FSDSql::getMostRecentlyPlayedIds(int playerId, int max_count) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);
	vector<RecentlyPlayed> retval;

	try {
		string gamerProfileId = GetGamerId(playerId);
		ostringstream q;
		q << "SELECT RecentlyPlayedTitleContentId, ContentItemName, MountedDeviceMountPoint from RecentlyPlayedTitles, ";
		q << " ContentItems, ScanPaths ";
		q << " LEFT OUTER JOIN MountedDevices On ScanPathDeviceId = MountedDeviceDeviceId ";
		q << " WHERE RecentlyPlayedTitleGamerProfile = '" << gamerProfileId << "'";
		q << " AND RecentlyPlayedTitleContentId = ContentItemId ";
		q << " AND ContentItemScanPathId = ScanPathId ";
		q << " ORDER BY RecentlyPlayedTitleOrder DESC ";

		int count = 0;
		pStmt.Sql(q.str());
		while (pStmt.FetchRow() && count < max_count) {
			RecentlyPlayed rp;
			rp.contentId = pStmt.GetColumnInt64(0);
			rp.title = FromUtf8(pStmt.GetColumnString(1));
			rp.bIsMounted = pStmt.GetColumnString(2).compare("") == 0 ? false : true;
			retval.push_back(rp);
			count++;
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getMostRecentlyPlayedIds", exception.GetString().c_str());
	}
	FSD_UNLOCK;
	return retval;
}

bool FSDSql::getRecentlyPlayed(int playerId, ULONGLONG contentId, PULONGLONG order, SYSTEMTIME* st) { 
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	string gamerProfileId = GetGamerId(playerId);
	ostringstream q;
	q << "SELECT RecentlyPlayedTitleDateTime, RecentlyPlayedTitleOrder from RecentlyPlayedTitles WHERE RecentlyPlayedTitleContentId = '" << contentId << "' AND RecentlyPlayedTitleGamerProfile = '" << gamerProfileId << "'";

	pStmt.Sql(q.str());
	if (pStmt.FetchRow()) {
		string pO = "0x";
		bool isNull;
		pO.append(pStmt.GetColumnString(1, isNull));
		ULARGE_INTEGER li;
		sscanf_s(pO.c_str(), "%I64X", &li.QuadPart );
		*order = li.QuadPart;

		time_t time = pStmt.GetColumnInt64(0);
		UnixTimeToSystemTime(&time, st);
		pStmt.FreeQuery();
		FSD_UNLOCK;
		return true;
	}

	*order = (ULONGLONG)0; 
	pStmt.FreeQuery();
	FSD_UNLOCK;
	return false;
}

SambaCredentialsEncrypted FSDSql::getSambaClientCredentials(int playerId, string path) 
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	string gamerProfileId = GetGamerId(playerId);
	ostringstream q;
	q << "SELECT SambaClientCredentialLogin, SambaClientCredentialEncryptedPassword from Settings.SambaClientCredentials ";
	q << " WHERE SambaClientCredentialGamerProfile = ? AND SambaClientCredentialPath = ?";

	std::transform(path.begin(), path.end(), path.begin(), ::toupper);

	pStmt.Sql(q.str());
	pStmt.BindString(1, gamerProfileId);
	pStmt.BindString(2, path);
	if (pStmt.FetchRow()) {
		SambaCredentialsEncrypted c;
		c.login = pStmt.GetColumnString(0);
		c.encryptedPassword = pStmt.GetColumnString(1);
		pStmt.FreeQuery();
		FSD_UNLOCK;
		return c;
	}

	FSD_UNLOCK;
	throw 0;
}

vector<SambaCredentialsEncrypted> FSDSql::getAllSambaClientCredentials(int playerId) 
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	string gamerProfileId = GetGamerId(playerId);
	ostringstream q;
	q << "SELECT SambaClientCredentialLogin, SambaClientCredentialEncryptedPassword, SambaClientCredentialPath from Settings.SambaClientCredentials ";
	q << " WHERE SambaClientCredentialGamerProfile = ?";

	vector<SambaCredentialsEncrypted> retval;

	pStmt.Sql(q.str());
	pStmt.BindString(1, gamerProfileId);
	while (pStmt.FetchRow()) {
		SambaCredentialsEncrypted c;
		c.login = pStmt.GetColumnString(0);
		c.encryptedPassword = pStmt.GetColumnString(1);
		c.smbPath = pStmt.GetColumnString(2);
		retval.push_back(c);
	}

	pStmt.FreeQuery();
	FSD_UNLOCK;
	return retval;
}

void FSDSql::deleteSambaClientCredential(int playerId, string path)
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	std::transform(path.begin(), path.end(), path.begin(), ::toupper);

	string gamerProfileId = GetGamerId(playerId);
	ostringstream q;
	q << "DELETE FROM Settings.SambaClientCredentials ";
	q << " WHERE SambaClientCredentialGamerProfile = ? AND SambaClientCredentialPath = ?";
	pStmt.Sql(q.str());
	pStmt.BindString(1, gamerProfileId);
	pStmt.BindString(2, path);
	pStmt.ExecuteAndFree();
	FSD_UNLOCK;
}



void FSDSql::setSambaClientCredential(int playerId, string path, string login, string encryptedPassword) 
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	std::transform(path.begin(), path.end(), path.begin(), ::toupper);

	string gamerProfileId = GetGamerId(playerId);
	ostringstream q;
	q << "INSERT OR REPLACE INTO Settings.SambaClientCredentials ";
	q << " (SambaClientCredentialGamerProfile, SambaClientCredentialPath, SambaClientCredentialLogin, SambaClientCredentialEncryptedPassword) ";
	q << " VALUES (?, ?, ?, ?)";
	pStmt.Sql(q.str());
	pStmt.BindString(1, gamerProfileId);
	pStmt.BindString(2, path);
	pStmt.BindString(3, login);
	pStmt.BindString(4, encryptedPassword);
	pStmt.ExecuteAndFree();
	FSD_UNLOCK;
}

void FSDSql::updateUserRating(int playerId, ULONGLONG contentId, int rating) { 
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	string gamerProfileId = GetGamerId(playerId);
	ostringstream q;
	q << "INSERT OR REPLACE INTO UserRatings (UserRatingContentId, UserRatingGamerProfile, UserRatingRating) values ('" << contentId << "', '" << gamerProfileId << "', " << rating << ")";
	pStmt.SqlStatement(q.str());	
	FSD_UNLOCK;
}

// this isn't expected to be used...rather this information should be pulled by left outer joining to the userRatings table when fetching content
int FSDSql::getUserRating(int playerId, ULONGLONG contentId) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	string val;

	string gamerProfileId = GetGamerId(playerId);
	ostringstream q;
	q << "SELECT UserRatingRating FROM UserRatings WHERE UserRatingContentId = '" << contentId << "' AND UserRatingGamerProfile = '" << gamerProfileId << "'";

	pStmt.Sql(q.str());
	if(pStmt.FetchRow()) {
		pStmt.FreeQuery();
		FSD_UNLOCK;
		return pStmt.GetColumnInt(0);	
	}
	pStmt.FreeQuery();
	FSD_UNLOCK;
	return 0;
}

void FSDSql::updateFavorite(int playerId, ULONGLONG contentId, bool isFavorite) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();

	Kompex::SQLiteStatement pStmt(pDatabase);

	string gamerProfileId = GetGamerId(playerId);
	ostringstream q;

	if (isFavorite) 
		q << "INSERT OR REPLACE INTO Favorites (FavoriteContentId, FavoriteGamerProfile) VALUES (" << contentId << ", '" << gamerProfileId << "')";
	else
		q << "DELETE FROM Favorites WHERE FavoriteContentId = " << contentId << " AND FavoriteGamerProfile = '" << gamerProfileId << "'";

	pStmt.SqlStatement(q.str());	

	FSD_UNLOCK;
}
ScanPath* FSDSql::getScanPath(const string& path, const string& deviceId) 
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	ScanPath* retval = NULL;

	try {
		ostringstream os;
		os << "SELECT ScanPathId, ScanPathPath, ScanPathRetailTabId, ScanPathDevkitTabId, ScanPathDepth, MountedDeviceMountPoint ";
		os << "  FROM ScanPaths, MountedDevices ";
		os << "  WHERE ScanPathDeviceId = MountedDeviceDeviceId ";
		os << "  AND ScanPathDeviceId = ? ";
		os << "  AND ScanPathPath = ? ";
		pStmt.Sql(os.str());
		pStmt.BindString(1, deviceId);
		pStmt.BindString(2, path);
		pStmt.FetchRow();
		int col = 0;
		ULONGLONG id = pStmt.GetColumnInt64(col++);
		string path = pStmt.GetColumnString(col++);
		ULONGLONG retail = pStmt.GetColumnInt64(col++);
		ULONGLONG devkit = pStmt.GetColumnInt64(col++);
		int depth = pStmt.GetColumnInt(col++);
		string root = pStmt.GetColumnString(col++);

		string retailName = SETTINGS::getInstance().DatabaseTabIdToInternalTabName(retail);
		string devkitName = SETTINGS::getInstance().DatabaseTabIdToInternalTabName(devkit);
		retval = new ScanPath(id, root + path, retailName, devkitName, retail, devkit, depth);
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getScanPath", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return retval;
}

string FSDSql::getSerialByScanPathId(DWORD nScanPathId) 
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	string retval;

	try {
		ostringstream os;
		os << "SELECT ScanPathDeviceId ";
		os << "  FROM ScanPaths ";
		os << "  WHERE ScanPathId = ? ";

		pStmt.Sql(os.str());
		pStmt.BindInt(1, nScanPathId);
	
		pStmt.FetchRow();
		int col = 0;
		string serial = pStmt.GetColumnString(col++);

		retval = serial;
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getSerialNumberById", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return retval;
}

// deleteScanPath should probably remove all content that points to this scanpath as well
// and will have to in the future when we make it a foreign key in the database
void FSDSql::deleteScanPath(ULONGLONG scanPathId) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "DELETE FROM ScanPaths WHERE ScanPathId = ?";
		pStmt.Sql(q.str());
		pStmt.BindInt64(1, scanPathId);
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't deleteScanPath [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
}

void FSDSql::deleteTitleUpdate(ULONGLONG titleUpdateId)
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "DELETE FROM TitleUpdates WHERE TitleUpdateId = ?";
		pStmt.Sql(q.str());
		pStmt.BindInt64(1, titleUpdateId);
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't deleteTitleUpdate [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
}


void FSDSql::updateScanPath(const string& path, const string& deviceId, ULONGLONG scanPathId, ULONGLONG retailTabId, ULONGLONG devkitTabId, int depth) 
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "UPDATE ScanPaths SET ScanPathPath = ?, ScanPathDeviceId = ?,ScanPathRetailTabId = ?, ScanPathDevkitTabId = ?, ScanPathDepth = ? WHERE ScanPathId = ?";
		pStmt.Sql(q.str());
		pStmt.BindString(1, path);
		pStmt.BindString(2, deviceId);
		pStmt.BindInt64(3, retailTabId);
		pStmt.BindInt64(4, devkitTabId);
		pStmt.BindInt(5, depth);
		pStmt.BindInt64(6, scanPathId);
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't updateScanPath [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
}

ULONGLONG FSDSql::addScanPath(const string& path, const string& deviceId, ULONGLONG retailTabId, ULONGLONG devkitTabId, int depth) {
	FSD_LOCK;

	ULONGLONG id = -1;
	try {
		Kompex::SQLiteDatabase *pDatabase = getDatabase();
		Kompex::SQLiteStatement pStmt(pDatabase);

		ostringstream q;
		q << "INSERT INTO ScanPaths (ScanPathPath, ScanPathDeviceId, ScanPathRetailTabId, ScanPathDevkitTabId, ScanPathDepth)";
		q << " VALUES (?, ?, ?, ?, ?) ";

		pStmt.Sql(q.str());
		pStmt.BindString(1, path);
		pStmt.BindString(2, deviceId);
		pStmt.BindInt64(3, retailTabId);
		pStmt.BindInt64(4, devkitTabId);
		pStmt.BindInt(5, depth);
		pStmt.ExecuteAndFree();

		id = pDatabase->GetLastInsertRowId();
	}
	catch (Kompex::SQLiteException) // &exception)
	{
		//Let's see if it failed because it's there already, and if so, return the existing id.
		ScanPath* sp = FSDSql::getScanPath(path, deviceId);
		if (sp != NULL) {
			id = sp->PathId;
			delete sp;
		}
		//DebugMsg("FSDSql", "Couldn't addScanPath for %s [%s]", path.c_str(), exception.GetString().c_str());
	}
	FSD_UNLOCK;
	return id;
}

///////////////////////////////////////
// Kai Messages section
///////////////////////////////////////

ULONGLONG FSDSql::addKaiUser(const string& name, const string& password) {
	FSD_LOCK;
	ULONGLONG retval = 0;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "INSERT INTO KaiUsers (KaiUserName, KaiUserPassword) ";
		q << "  VALUES (?, ?)";
		pStmt.Sql(q.str());
		pStmt.BindString(1, name);
		pStmt.BindString(2, password);
		pStmt.ExecuteAndFree();
		retval = pDatabase->GetLastInsertRowId();
	}
	catch (Kompex::SQLiteException) // &exception)
	{
		// unique constraint probably violated
	} 
	FSD_UNLOCK;
	return retval;
}

void FSDSql::addProfileXuid(const string& xuid, ULONGLONG optionalKaiUserId) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "INSERT OR REPLACE INTO ProfileXuids (ProfileXuidXuid, ProfileXuidKaiUserId) VALUES (?, ?)";
		pStmt.Sql(q.str());
		pStmt.BindString(1, xuid);
		pStmt.BindInt64(2, optionalKaiUserId);
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't addProfileXuid [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
}

void FSDSql::deleteKaiUser(ULONGLONG id) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "DELETE FROM KaiUsers WHERE KaiUserId = ?";
		pStmt.Sql(q.str());
		pStmt.BindInt64(1, id);
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't deleteKaiUser [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
}

ULONGLONG FSDSql::addKaiContact(const string& name, const string& path) {
	FSD_LOCK;
	ULONGLONG retval = 0;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "INSERT INTO KaiContacts (KaiContactName, KaiContactIconPath) ";
		q << "  VALUES (?, ?)";
		pStmt.Sql(q.str());
		pStmt.BindString(1, name);
		pStmt.BindString(2, path);
		pStmt.ExecuteAndFree();
		retval = pDatabase->GetLastInsertRowId();
	}
	catch (Kompex::SQLiteException) // &exception)
	{
		// unique constraint probably violated
	} 
	FSD_UNLOCK;
	return retval;
}

vector<ScanPath> FSDSql::getScanPaths() {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	vector<ScanPath> retVec;

	try {


		ostringstream os;
		os << "SELECT ScanPathId, ScanPathPath, ScanPathRetailTabId, ScanPathDevkitTabId, ScanPathDepth, MountedDeviceMountPoint ";
		os << "  FROM ScanPaths, MountedDevices ";
		os << "  WHERE ScanPathDeviceId = MountedDeviceDeviceId ";
		pStmt.Sql(os.str());
		while (pStmt.FetchRow()) {
			int col = 0;
			ULONGLONG id = pStmt.GetColumnInt64(col++);
			string path = pStmt.GetColumnString(col++);
			ULONGLONG retail = pStmt.GetColumnInt64(col++);
			ULONGLONG devkit = pStmt.GetColumnInt64(col++);
			int depth = pStmt.GetColumnInt(col++);
			string root = pStmt.GetColumnString(col++);

			string retailName = SETTINGS::getInstance().DatabaseTabIdToInternalTabName(retail);
			string devkitName = SETTINGS::getInstance().DatabaseTabIdToInternalTabName(devkit);
			ScanPath s(id, root + path, retailName, devkitName, retail, devkit, depth);
			retVec.push_back(s);
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getScanPaths", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return retVec;
}

vector<KaiUser*> FSDSql::getKaiUsers() {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	vector<KaiUser*> retVec;

	try {
		ostringstream os;
		os << "SELECT KaiUserId, KaiUserName, KaiUserPassword, ProfileXuidXuid ";
		os << "  FROM KaiUsers, ProfileXuids ";
		os << "  WHERE KaiUserId = ProfileXuidKaiUserId ";
		pStmt.Sql(os.str());
		while (pStmt.FetchRow()) {
			int col = 0;
			ULONGLONG id = pStmt.GetColumnInt64(col++);
			string name = pStmt.GetColumnString(col++);
			string password = pStmt.GetColumnString(col++);
			string xuid = pStmt.GetColumnString(col++);

			KaiUser* u = new KaiUser(id, name, password, xuid);
			retVec.push_back(u);
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getKaiUsers", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return retVec;
}


vector<KaiContact*> FSDSql::getKaiContactsForUserId(ULONGLONG userId) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	vector<KaiContact*> retVec;

	try {
		ostringstream os;
		os << "SELECT KaiContactId, KaiContactName, KaiContactIconPath ";
		os << "  FROM KaiContacts, KaiUserToContacts ";
		os << "  WHERE KaiUserToContacts.KaiUserToContactsKaiUserId = ? ";
		os << "  AND KaiUserToContacts.KaiUserToContactsKaiContactId = KaiContacts.KaiContactId ";
		pStmt.Sql(os.str());
		pStmt.BindInt64(1, userId);
		while (pStmt.FetchRow()) {
			int col = 0;
			ULONGLONG id = pStmt.GetColumnInt64(col++);
			string name = pStmt.GetColumnString(col++);
			string path = pStmt.GetColumnString(col++);

			KaiContact* c = new KaiContact(id, name, path);
			retVec.push_back(c);
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getKaiContactsForUserId", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return retVec;
}

void FSDSql::setKaiUserPassword(ULONGLONG id, const string& password) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "UPATE KaiUsers SET KaiUserPassword = ? WHERE KaiUserId = ?";
		pStmt.Sql(q.str());
		pStmt.BindString(1, password);
		pStmt.BindInt64(2, id);
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't setKaiUserPassword [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
}


void FSDSql::deleteKaiContact(ULONGLONG id) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "DELETE FROM KaiContacts WHERE KaiContactId = ?";
		pStmt.Sql(q.str());
		pStmt.BindInt64(1, id);
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't deleteKaiContact [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
}

void FSDSql::joinKaiUserToContact(ULONGLONG userId, ULONGLONG contactId) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "INSERT INTO KaiUserToContacts (KaiUserId, KaiContactId) ";
		q << "  VALUES (?, ?)";
		pStmt.Sql(q.str());
		pStmt.BindInt64(1, userId);
		pStmt.BindInt64(2, contactId);
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException) // &exception)
	{
		// unique constraint probably violated
	} 
	FSD_UNLOCK;
}

ULONGLONG FSDSql::getKaiUserIdByName(const string& name) {
	FSD_LOCK;
	ULONGLONG retval = 0;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "SELECT KaiUserId FROM KaiUsers WHERE KaiUserName = ?";
		pStmt.Sql(q.str());
		pStmt.BindString(1, name);
		if (pStmt.FetchRow()) {
			retval = pStmt.GetColumnInt(0);
		}
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't getKaiUserIdByName [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
	return retval;
}

KaiUser* FSDSql::getKaiUserIdByXuid(const string& xuid) {
	FSD_LOCK;
	KaiUser* retval = NULL;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "SELECT KaiUserId, KaiUserName, KaiUserPassword FROM KaiUsers, ProfileXuids WHERE KaiUserId = ProfileXuidKaiUserId AND ProfileXuidXuid = ?";
		pStmt.Sql(q.str());
		pStmt.BindString(1, xuid);
		if (pStmt.FetchRow()) {
			ULONGLONG id = pStmt.GetColumnInt(0);
			string name = pStmt.GetColumnString(1);
			string password = pStmt.GetColumnString(2);
			retval = new KaiUser(id, name, password, xuid);
		}
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't getKaiUserIdByXuid [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
	return retval;
}


ULONGLONG FSDSql::addKaiMessage(const string& sender, const string& message, int flag, time_t time, ULONGLONG userId, ULONGLONG contactId){
	FSD_LOCK;
	ULONGLONG retval = 0;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "INSERT INTO KaiMessages (KaiMessageSender, KaiMessageBody, KaiMessageFlag, KaiMessageDateTime, KaiMessageUserId, KaiMessageContactId) ";
		q << "  VALUES (?, ?, ?, ?, ?, ?)";
		pStmt.Sql(q.str());
		pStmt.BindString(1, sender);
		pStmt.BindString(2, message);
		pStmt.BindInt(3, flag);
		pStmt.BindInt64(4, time);
		pStmt.BindInt64(5, userId);
		pStmt.BindInt64(6, contactId);
		pStmt.ExecuteAndFree();
		retval = pDatabase->GetLastInsertRowId();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't addKaiMessage [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
	return retval;
}

void FSDSql::deleteKaiMessage(ULONGLONG id) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "DELETE FROM KaiMessages WHERE KaiMessageId = ?";
		pStmt.Sql(q.str());
		pStmt.BindInt64(1, id);
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't deleteKaiMessage [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
}

void FSDSql::updateKaiMessageFlag(ULONGLONG id, int flag) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		ostringstream q;
		q << "UPDATE KaiMessages SET KaiMessageFlag = ? WHERE KaiMessageId = ?";
		pStmt.Sql(q.str());
		pStmt.BindInt(1, flag);
		pStmt.BindInt64(2, id);
		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't updateKaiMessageFlag [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
}

vector<KaiMessage*> FSDSql::getKaiMessagesForUserId(ULONGLONG userId) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	vector<KaiMessage*> retVec;

	try {
		ostringstream os;
		os << "SELECT KaiMessageId, KaiMessageSender, KaiMessageBody, KaiMessageFlag, KaiMessageDateTime, ";
		os << "  KaiContactId, KaiContactName, KaiMessageContactPath ";
		os << "  FROM KaiMessages, KaiContacts ";
		os << "  WHERE KaiMessages.KaiMessageContactId = KaiContacts.KaiContactId";
		os << "  AND KaiMessages.KaiMessageUserId = ?";
		pStmt.Sql(os.str());
		pStmt.BindInt64(1, userId);
		while (pStmt.FetchRow()) {
			int col = 0;
			ULONGLONG id = pStmt.GetColumnInt64(col++);
			string sender = pStmt.GetColumnString(col++);
			string body = pStmt.GetColumnString(col++);
			int flag = pStmt.GetColumnInt(col++);
			time_t time = pStmt.GetColumnInt64(col++);
			ULONGLONG contactId = pStmt.GetColumnInt64(col++);
			string contact = pStmt.GetColumnString(col++);
			string contactIconPath = pStmt.GetColumnString(col++);

			KaiMessage* km = new KaiMessage(id, sender, body, flag, time, contactId, contact, contactIconPath);
			retVec.push_back(km);
		}
	}
	catch(Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "getKaiMessagesForUserId", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return retVec;
}

void FSDSql::writeUserSettings(const string& skin) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	string gamerProfile = GetGamerId(0);

	try {
		pStmt.Sql("DELETE FROM UserSettings WHERE UserSettingGamerProfile = ?");
		pStmt.BindString(1, gamerProfile);
		pStmt.ExecuteAndFree();

		ostringstream q;
		q << "INSERT INTO UserSettings (";
		q << " UserSettingGamerProfile, UserSettingSkin ";
		q << " ) VALUES ( ";
		q << " ?, ? ";
		q << ")";

		pStmt.Sql(q.str());
		pStmt.BindString(1, gamerProfile);
		pStmt.BindString(2, skin);

		pStmt.ExecuteAndFree();
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't writeUserSettings [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
}

void FSDSql::writeGlobalSettings(const string& videoPath, const string& musicPath, const string& dvdCopy360Path,
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
		int glTransition, bool bSambaClientOn, const string& weatherLocationName, int ResumeAssetDownload, int DisableAutoScan, 
		int WeatherUnits, const string& smbClientWorkgroup, bool bSambaServerOn, const string& hostname, int FMAdvancedMode, 
		int CoverPoll, const string& CoverPass, const string& CoverUser, bool CoverCheck, int CoverQuality)
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	try {
		pStmt.BeginTransaction();
		pStmt.SqlStatement("DELETE FROM GlobalSettings");
		ostringstream q;
		q << "INSERT INTO GlobalSettings (";
		q << " GlobalSettingVideoPath, GlobalSettingMusicPath, GlobalSettingDVDCopy360Path, "; //3
		q << " GlobalSettingDVDCopyOrigPath, GlobalSettingWeatherLocation, GlobalSettingHOverscan, GlobalSettingVOverscan, "; //7
		q << " GlobalSettingHDisplacement, GlobalSettingVDisplacement, GlobalSettingLetterbox, GlobalSettingFtpPort, "; //11
		q << " GlobalSettingFtpUser, GlobalSettingFtpPassword, GlobalSettingFanSpeed, GlobalSettingLEDFormat, "; //15
		q << " GlobalSettingTimeFormat, GlobalSettingDateFormat, GlobalSettingNTPOnStartup, GlobalSettingUpdateXEXNotice, "; //19
		q << " GlobalSettingUpdateSkinNotice, GlobalSettingUpdatePluginNotice, GlobalSettingUpdateOnBeta, GlobalSettingInstallerState, "; //23
		q << " GlobalSettingFsdUsername, GlobalSettingFsdPassword, GlobalSettingFsdUseLogin, GlobalSettingLocale, "; // 27
		q << " GlobalSettingGameListBackground, GlobalSettingGameListVisual, GlobalSettingMainTempStatus, GlobalSettingMainDiskStatus, ";  //  32
		q << " GlobalSettingMainUseCelsius, GlobalSettingMainShowIp, GlobalSettingMainShowStatus, GlobalSettingMainShowAvatar, GlobalSettingGameListScreenshots, ";
		q << " GlobalSettingKaiAutoLogin, GlobalSettingKaiLinkProfile, GlobalSettingKaiNoticeOnline, GlobalSettingKaiNoticeOffline, ";
		q << " GlobalSettingKaiNoticeInvite, GlobalSettingKaiNoticeContactPM, GlobalSettingKaiNoticeOpponentPM, GlobalSettingKaiServerIp, ";
		q << " GlobalSettingKaiUsername, GlobalSettingKaiPassword, GlobalSettingFtpServerOn, GlobalSettingHttpServerOn, GlobalSettingShowStatusAtBoot, ";
		q << " GlobalSettingDownloadIcon, GlobalSettingDownloadBoxart, GlobalSettingDownloadBanner, GlobalSettingDownloadBackground, ";
		q << " GlobalSettingDownloadVideo, GlobalSettingDownloadScreenshots, GlobalSettingMaxScreenshots, GlobalSettingHTTPDisableSecurity, ";
		q << " GlobalSettingHttpUsername, GlobalSettingHttpPassword, GlobalSettingGameListTransition, GlobalSettingSambaClientOn, ";
		q << " GlobalSettingWeatherLocName, GlobalSettingResumeAssetDownload, GlobalSettingDisableScanAtBoot, GlobalSettingWeatherUnits, ";
		q << " GlobalSettingSambaClientWorkgroup, GlobalSettingSambaServerOn, GlobalSettingHostname, GlobalSettingAdvancedModeOn, ";
		q << " GlobalSettingCoverPollInterval, GlobalSettingCoverPassword, GlobalSettingCoverUserName, GlobalSettingCoverCheck, GlobalSettingCoverQuality ";
		
		q << " ) VALUES ( ";
		q << " ?, ?, ?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ?, ?, ?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ";
		q << " ?, ?, ?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ? ,?, ";
		q << " ?, ?, ?, ?, ";
		q << " ?, ?, ?, ?, ? ";
		q << ")";

		pStmt.Sql(q.str());
		int col = 1;
		pStmt.BindString(col++, videoPath);
		pStmt.BindString(col++, musicPath);
		pStmt.BindString(col++, dvdCopy360Path);

		pStmt.BindString(col++, dvdCopyOrigPath);
		pStmt.BindString(col++, weatherLocation);
		pStmt.BindInt(col++, hOverscan);
		pStmt.BindInt(col++, vOverscan);

		pStmt.BindInt(col++, hDisplacement);
		pStmt.BindInt(col++, vDisplacement);
		pStmt.BindInt(col++, bLetterbox);
		pStmt.BindInt(col++, ftpPort);

		pStmt.BindString(col++, ftpUser);
		pStmt.BindString(col++, ftpPassword);
		pStmt.BindInt(col++, fanSpeed);
		pStmt.BindInt(col++, LEDformat);

		pStmt.BindInt(col++, timeFormat);
		pStmt.BindInt(col++, dateFormat);
		pStmt.BindInt(col++, bNTPOnStartup);
		pStmt.BindInt(col++, bUpdateXEXNotice);

		pStmt.BindInt(col++, bUpdateSkinNotice);
		pStmt.BindInt(col++, bUpdatePluginNotice);
		pStmt.BindInt(col++, bUpdateOnBeta);
		pStmt.BindInt(col++, installerState);
		
		pStmt.BindString(col++, fsdUsername);
		pStmt.BindString(col++, fsdPassword);
		pStmt.BindInt(col++, fsdUseLogin);
		pStmt.BindString(col++, locale);

		pStmt.BindInt(col++, bShowBackground);
		pStmt.BindInt(col++, nGameListVisual);
		pStmt.BindInt(col++, bShowTempStatus);
		pStmt.BindInt(col++, bShowDiskStatus);
		pStmt.BindInt(col++, bUseCelsius);

		pStmt.BindInt(col++, bShowIP);
		pStmt.BindInt(col++, bShowStatus);
		pStmt.BindInt(col++, bShowAvatar);

		pStmt.BindInt(col++, bShowScreenshots);

		pStmt.BindInt(col++, bKaiAutoLogin);
		pStmt.BindInt(col++, bKaiLinkProfile);
		pStmt.BindInt(col++, bKaiNoticeOnline);
		pStmt.BindInt(col++, bKaiNoticeOffline);
		pStmt.BindInt(col++, bKaiNoticeInvite);
		pStmt.BindInt(col++, bKaiNoticeContactPM);
		pStmt.BindInt(col++, bKaiNoticeOpponentPM);
		pStmt.BindString(col++, kaiServerIp);
		pStmt.BindString(col++, kaiUsername);
		pStmt.BindString(col++, kaiPassword);

		pStmt.BindInt(col++, bFtpServerOn ? 1 : 0);
		pStmt.BindInt(col++, bHttpServerOn ? 1 : 0);

		pStmt.BindInt(col++, bShowStatusAtBoot);

		pStmt.BindInt(col++, downloadIcon );
		pStmt.BindInt(col++, downloadBoxart );
		pStmt.BindInt(col++, downloadBanner );
		pStmt.BindInt(col++, downloadBackground );
		pStmt.BindInt(col++, downloadVideo );
		pStmt.BindInt(col++, downloadScreenshots );
		pStmt.BindInt(col++, maxScreenshots );
		pStmt.BindInt(col++, httpDisableSecurity );

		pStmt.BindString(col++, httpUsername );
		pStmt.BindString(col++, httpPassword );
		pStmt.BindInt(col++, glTransition);
		pStmt.BindInt(col++, bSambaClientOn ? 1 : 0);

		pStmt.BindString(col++, weatherLocationName);
		pStmt.BindInt(col++, ResumeAssetDownload);
		pStmt.BindInt(col++, DisableAutoScan);
		pStmt.BindInt(col++, WeatherUnits);

		pStmt.BindString(col++, smbClientWorkgroup);
		pStmt.BindInt(col++, bSambaServerOn ? 1 : 0);
		pStmt.BindString(col++, hostname);

		pStmt.BindInt(col++, FMAdvancedMode);

		pStmt.BindInt(col++, CoverPoll);
		pStmt.BindString(col++, CoverPass);
		pStmt.BindString(col++, CoverUser);
		pStmt.BindInt(col++, CoverCheck ? 0:1);
		pStmt.BindInt(col++, CoverQuality);

		pStmt.ExecuteAndFree();
		pStmt.CommitTransaction();

	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Couldn't writeGlobalSettings [%s]", exception.GetString().c_str());
	} 
	FSD_UNLOCK;
}

bool FSDSql::readUserSettings(string& skin) {
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	bool retval = true;
	string gamerProfile = GetGamerId(0);

	try {
		ostringstream q;
		q << "SELECT ";
		q << " UserSettingSkin "; //4
		q << " FROM UserSettings ";
		q << " WHERE UserSettingGamerProfile = ? ";
		pStmt.Sql(q.str());
		pStmt.BindString(1, gamerProfile);

		pStmt.FetchRow();
		int col = 0;

		skin = pStmt.GetColumnString(col++);
	}
	catch(Kompex::SQLiteException &exception)
	{
		retval = false;
		DebugMsg("FSDSql", "readUserSettings", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return retval;
}

bool FSDSql::readGlobalSettings(string& videoPath, string& musicPath, string& dvdCopy360Path,
		string& dvdCopyOrigPath, string& weatherLocation, int& hOverscan, int& vOverscan,
		int& hDisplacement, int& vDisplacement, int& bLetterbox, int& ftpPort,
		string& ftpUser, string& ftpPassword, int& fanSpeed, int& LEDformat,
		int& timeFormat, int& dateFormat, int& bNTPOnStartup, int& bUpdateXEXNotice,
		int& bUpdateSkinNotice, int& bUpdatePluginNotice, int& bUpdateOnBeta, int& installerState,
		string& fsdUsername, string& fsdPassword, int& fsdUseLogin, string& locale,
		int& bShowBackground, int& nGameListVisual, int& bShowTempStatus, int& bShowDiskStatus, int& bUseCelsius,
		int& bShowIP, int& bShowStatus, int& bShowAvatar, int& bShowScreenshots,
		int& bKaiAutoLogin, int& bKaiLinkProfile, int& bKaiNoticeOnline, int& bKaiNoticeOffline,
		int& bKaiNoticeInvite, int& bKaiNoticeContactPM, int& bKaiNoticeOpponentPM, string& kaiServerIp,
		string& kaiUsername, string& kaiPassword, bool& bFtpServerOn, bool& bHttpServerOn, int& bShowStatusAtBoot,
		int& downloadIcon, int& downloadBoxart, int& downloadBanner, int& downloadBackground, int& downloadVideo,
		int& downloadScreenshots, int& maxScreenshots, int& httpDisableSecurity, string& httpUsername, string& httpPassword,
		int& glTransition, bool& bSambaClientOn, string& weatherLocationName, int& ResumeAssetDownload, int& DisableAutoScan, 
		int& WeatherUnits, string& SambaClientWorkgroup, bool& bSambaServerOn, string& hostname, int& FMAdvancedMode, 
		int& CoverPoll, string& CoverPass, string& CoverUser, bool& CoverCheck, int& CoverQuality )
{
	FSD_LOCK;
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	bool retval = true;

	try {
		ostringstream q;
		q << "SELECT ";
		q << " GlobalSettingVideoPath, GlobalSettingMusicPath, GlobalSettingDVDCopy360Path, "; //4
		q << " GlobalSettingDVDCopyOrigPath, GlobalSettingWeatherLocation, GlobalSettingHOverscan, GlobalSettingVOverscan, "; //8
		q << " GlobalSettingHDisplacement, GlobalSettingVDisplacement, GlobalSettingLetterbox, GlobalSettingFtpPort, "; //12
		q << " GlobalSettingFtpUser, GlobalSettingFtpPassword, GlobalSettingFanSpeed, GlobalSettingLEDFormat, "; //16
		q << " GlobalSettingTimeFormat, GlobalSettingDateFormat, GlobalSettingNTPOnStartup, GlobalSettingUpdateXEXNotice, "; //20
		q << " GlobalSettingUpdateSkinNotice, GlobalSettingUpdatePluginNotice, GlobalSettingUpdateOnBeta, GlobalSettingInstallerState, "; //24
		q << " GlobalSettingFsdUsername, GlobalSettingFsdPassword, GlobalSettingFsdUseLogin, GlobalSettingLocale, "; // 28
		q << " GlobalSettingGameListBackground, GlobalSettingGameListVisual, GlobalSettingMainTempStatus, GlobalSettingMainDiskStatus, ";  //  32
		q << " GlobalSettingMainUseCelsius, GlobalSettingMainShowIp, GlobalSettingMainShowStatus, GlobalSettingMainShowAvatar, GlobalSettingGameListScreenshots, ";
		q << " GlobalSettingKaiAutoLogin, GlobalSettingKaiLinkProfile, GlobalSettingKaiNoticeOnline, GlobalSettingKaiNoticeOffline, ";
		q << " GlobalSettingKaiNoticeInvite, GlobalSettingKaiNoticeContactPM, GlobalSettingKaiNoticeOpponentPM, GlobalSettingKaiServerIp, ";
		q << " GlobalSettingKaiUsername, GlobalSettingKaiPassword, GlobalSettingFtpServerOn, GlobalSettingHttpServerOn, GlobalSettingShowStatusAtBoot, ";
		q << " GlobalSettingDownloadIcon, GlobalSettingDownloadBoxart, GlobalSettingDownloadBanner, GlobalSettingDownloadBackground, ";
		q << " GlobalSettingDownloadVideo, GlobalSettingDownloadScreenshots, GlobalSettingMaxScreenshots, GlobalSettingHTTPDisableSecurity, ";
		q << " GlobalSettingHttpUsername, GlobalSettingHttpPassword, GlobalSettingGameListTransition, GlobalSettingSambaClientOn, ";
		q << " GlobalSettingWeatherLocName, GlobalSettingResumeAssetDownload, GlobalSettingDisableScanAtBoot, GlobalSettingWeatherUnits"; 
		// DON'T ADD ANY MORE HERE... ADD THEM BELOW
		q << " FROM GlobalSettings";
		pStmt.Sql(q.str());
		pStmt.FetchRow();
		int col = 0;

		videoPath = pStmt.GetColumnString(col++);
		musicPath = pStmt.GetColumnString(col++);
		dvdCopy360Path = pStmt.GetColumnString(col++);

		dvdCopyOrigPath = pStmt.GetColumnString(col++);
		weatherLocation = pStmt.GetColumnString(col++);
		hOverscan = pStmt.GetColumnInt(col++);
		vOverscan = pStmt.GetColumnInt(col++);

		hDisplacement = pStmt.GetColumnInt(col++);
		vDisplacement = pStmt.GetColumnInt(col++);
		bLetterbox = pStmt.GetColumnInt(col++);
		ftpPort = pStmt.GetColumnInt(col++);

		ftpUser = pStmt.GetColumnString(col++);
		ftpPassword = pStmt.GetColumnString(col++);
		fanSpeed = pStmt.GetColumnInt(col++);
		LEDformat = pStmt.GetColumnInt(col++);

		timeFormat = pStmt.GetColumnInt(col++);
		dateFormat = pStmt.GetColumnInt(col++);
		bNTPOnStartup = pStmt.GetColumnInt(col++);
		bUpdateXEXNotice = pStmt.GetColumnInt(col++);

		bUpdateSkinNotice = pStmt.GetColumnInt(col++);
		bUpdatePluginNotice = pStmt.GetColumnInt(col++);
		bUpdateOnBeta = pStmt.GetColumnInt(col++);
		installerState = pStmt.GetColumnInt(col++);

		fsdUsername = pStmt.GetColumnString(col++);
		fsdPassword = pStmt.GetColumnString(col++);
		fsdUseLogin = pStmt.GetColumnInt(col++);
		locale = pStmt.GetColumnString(col++);

		bShowBackground = pStmt.GetColumnInt(col++);
		nGameListVisual = pStmt.GetColumnInt(col++);
		bShowTempStatus = pStmt.GetColumnInt(col++);
		bShowDiskStatus = pStmt.GetColumnInt(col++);
		bUseCelsius = pStmt.GetColumnInt(col++);

		bShowIP = pStmt.GetColumnInt(col++);
		bShowStatus = pStmt.GetColumnInt(col++);
		bShowAvatar = pStmt.GetColumnInt(col++);

		bShowScreenshots = pStmt.GetColumnInt(col++);

		bKaiAutoLogin = pStmt.GetColumnInt(col++);
		bKaiLinkProfile = pStmt.GetColumnInt(col++);
		bKaiNoticeOnline = pStmt.GetColumnInt(col++);
		bKaiNoticeOffline = pStmt.GetColumnInt(col++);
		bKaiNoticeInvite = pStmt.GetColumnInt(col++);
		bKaiNoticeContactPM = pStmt.GetColumnInt(col++);
		bKaiNoticeOpponentPM = pStmt.GetColumnInt(col++);
		kaiServerIp = pStmt.GetColumnString(col++);
		kaiUsername = pStmt.GetColumnString(col++);
		kaiPassword = pStmt.GetColumnString(col++);
		
		bFtpServerOn = pStmt.GetColumnInt(col++) ? true: false;
		bHttpServerOn = pStmt.GetColumnInt(col++) ? true: false;

		bShowStatusAtBoot = pStmt.GetColumnInt(col++);

		downloadIcon = pStmt.GetColumnInt(col++);
		downloadBoxart = pStmt.GetColumnInt(col++);
		downloadBanner = pStmt.GetColumnInt(col++);
		downloadBackground = pStmt.GetColumnInt(col++);
		downloadVideo = pStmt.GetColumnInt(col++);
		downloadScreenshots = pStmt.GetColumnInt(col++);
		maxScreenshots = pStmt.GetColumnInt(col++);
		httpDisableSecurity = pStmt.GetColumnInt(col++);

		httpUsername = pStmt.GetColumnString(col++);
		httpPassword = pStmt.GetColumnString(col++);
		glTransition = pStmt.GetColumnInt(col++);
		bSambaClientOn = pStmt.GetColumnInt(col++) ? true: false;

		weatherLocationName = pStmt.GetColumnString(col++);
		ResumeAssetDownload = pStmt.GetColumnInt(col++);
		DisableAutoScan = pStmt.GetColumnInt(col++);
		WeatherUnits = pStmt.GetColumnInt(col++);
		// DON'T ADD ANY MORE HERE, ADD THEM BELOW

		q.str("");
		q << "SELECT ";
		q << " GlobalSettingSambaClientWorkgroup, GlobalSettingSambaServerOn, GlobalSettingHostname, GlobalSettingAdvancedModeOn, ";
		q << " GlobalSettingCoverPollInterval, GlobalSettingCoverPassword, GlobalSettingCoverUsername, GlobalSettingCoverCheck, ";
		q << " GlobalSettingCoverQuality ";
		q << " FROM GlobalSettings";

		pStmt.Sql(q.str());
		pStmt.FetchRow();
		col = 0;

		SambaClientWorkgroup = pStmt.GetColumnString(col++);
		bSambaServerOn = pStmt.GetColumnInt(col++) ? true: false;	
		hostname = pStmt.GetColumnString(col++);
		FMAdvancedMode = pStmt.GetColumnInt(col++);
		CoverPoll = pStmt.GetColumnInt(col++);
		CoverPass = pStmt.GetColumnString(col++);
		CoverUser = pStmt.GetColumnString(col++);
		CoverCheck = pStmt.GetColumnInt(col++) ? false: true;
		CoverQuality = pStmt.GetColumnInt(col++);

	}
	catch(Kompex::SQLiteException &exception)
	{
		retval = false;
		DebugMsg("FSDSql", "readGlobalSettings", exception.GetString().c_str());
	}

	FSD_UNLOCK;
	return retval;
}



int FSDSql::GetSQLTableNames(vector<string>* tableNames)
{
	int rowCount = 0;
	if (tableNames == NULL) return rowCount;
	
	// Lock
	FSD_LOCK;

	// Get our db and statement
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	ostringstream q;
	q << "SELECT name FROM (SELECT name, type FROM sqlite_master UNION ALL SELECT name, type FROM settings.sqlite_master) WHERE type='table' ORDER BY name";
	
	try {
		pStmt.Sql(q.str());
		while (pStmt.FetchRow())
		{
			tableNames->push_back(pStmt.GetColumnString(0));
			rowCount++;
		}
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Exception:  %s", exception.GetString().c_str());
	}

	pStmt.FreeQuery();

	// Unlock
	FSD_UNLOCK;

	return rowCount;
}

bool FSDSql::GetSQLTable(string tableName, SQLTable* table)
{
	int rowCount = 0;
	bool result = true;

	if (table == NULL) return false;
	
	// Lock
	FSD_LOCK;

	// Get our db and statement
	Kompex::SQLiteDatabase *pDatabase = getDatabase();
	Kompex::SQLiteStatement pStmt(pDatabase);

	ostringstream q;
	q << "SELECT * FROM " << tableName;
	
	try {
		pStmt.Sql(q.str());
		
		for (int i = 0; i < pStmt.GetColumnCount(); i++)
		{
			table->ColumnHeaders.push_back(pStmt.GetColumnName(i));
		}
		
		map<string, string> row;
		while (pStmt.FetchRow())
		{
			row.clear();
			for(int i = 0; i < pStmt.GetColumnCount(); i++)
			{
				row.insert(make_pair(pStmt.GetColumnName(i), pStmt.GetColumnString(i)));
			}
			table->Rows.push_back(row);
			rowCount++;
		}
		table->DataBaseName = pStmt.GetColumnDatabaseName(0);
		table->TableName = pStmt.GetColumnTableName(0);
	}
	catch (Kompex::SQLiteException &exception)
	{
		DebugMsg("FSDSql", "Exception:  %s", exception.GetString().c_str());
		result = false;
	}

	pStmt.FreeQuery();

	// Unlock
	FSD_UNLOCK;

	return result;
}