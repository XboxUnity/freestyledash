#include "stdafx.h"

#include "ContentManager.h"
#include "../Managers/Drives/DrivesManager.h"
#include "../SQLite/FSDSql.h"
#include "../Managers/TitleUpdates/TitleUpdateManager.h"
#include "ContentExtractor.h"
#include "CoverXML/CoverXMl.h"

#define SCAN_LOCK   EnterCriticalSection(&scanLock)
#define SCAN_UNLOCK LeaveCriticalSection(&scanLock)
#define ITEM_LOCK   EnterCriticalSection(&itemLock)
#define ITEM_UNLOCK LeaveCriticalSection(&itemLock)

ContentManager::ContentManager() {

	// Setup some variables
	paused = FALSE;
	bIsScanning = FALSE;
	Status = MANAGER_STATUS_NONE;
	dwFunction = MANAGER_FUNCTION_IDLE;
	szStatusMsg = L"";
	InitializeCriticalSection(&scanLock);
	InitializeCriticalSection(&itemLock);

	// Now begin our processing loop
	CreateThread(CPU2_THREAD_2);
}
ContentManager::~ContentManager() {

	DeleteCriticalSection(&scanLock);
	DeleteCriticalSection(&itemLock);
}

ContentItemVector ContentManager::GetContentByItemTab(CONTENT_ITEM_TAB itemTab)
{
	vector<ContentItemNew*> vContent;
	int nSize = contentItems.size();

	// iterate through all of our currently stored content items, and place into new vector
	for(int nCount = 0; nCount < nSize; nCount++)
	{
		ContentItemNew * pContent = contentItems.at(nCount);
		// if the content matches what we're looking for, let's add it to our list	
		if(pContent->GetItemTab() == itemTab) {
			vContent.push_back(pContent);
		}
	}

	return vContent;
}

void ContentManager::UpdateTimer() {
	if (!SETTINGS::getInstance().getCoverCheck()) {
		TimerManager::getInstance().remove(*this);
	} else {
		if (TickCount != SETTINGS::getInstance().getCoverPoll()) {
			int MS = SETTINGS::getInstance().getCoverPoll() * 60000;
			if (MS == 0) {
			} else {
				TimerManager::getInstance().add(*this, MS);
			}	
		}
	}
}

void ContentManager::CheckCovers() {
  
}

void ContentManager::tick() {
	CheckCovers();
}

void ContentManager::DownloadCompleted(HttpItem* itm) {
	if(strcmp(itm->getTag().c_str(), "CoverXML")==0)
	{
		if (itm->getResponseCode() == 200) {
			DebugMsg("ContentManager", "Reading Custom Cover DownloadList");
			parseCoverXML(itm);
		}
	} else {
		if (itm->getResponseCode() == 200) {
			for (unsigned int x =0; x < Covers.size(); x++) {
				CoverInfo CI = Covers.at(x);
				if (strcmp(CI.Title.c_str(), itm->getTag().c_str())==0) {
					ProcessCover(itm, CI);
					break;
				}
			}
		}
	}
}

void ContentManager::ProcessCover(HttpItem *Item, CoverInfo CI) {
	MemoryBuffer& buffer = Item->GetMemoryBuffer();
	DWORD TI = strtoul(CI.TitleID.c_str(), NULL, 16);
	ContentItemVector CIVec;
	if (CI.Type == "HomeBrew" ) {
		CIVec = GetContentByItemTab(CONTENT_HOMEBREW);
		for (unsigned int x = 0; x < CIVec.size(); x++) {
			ContentItemNew* CIN = CIVec.at(x);
			string CINtitle = wstrtostr(CIN->getTitle());
			if (strcmp(make_lowercaseA(CINtitle).c_str(), make_lowercaseA(CI.Title).c_str())==0) {
				CIN->AddAsset(ITEM_UPDATE_TYPE_BOXART, buffer.GetData(), buffer.GetDataLength());
			}				
		}
		CIVec = GetContentByItemTab(CONTENT_EMULATOR);
		for (unsigned int x = 0; x < CIVec.size(); x++) {
			ContentItemNew* CIN = CIVec.at(x);
			string CINtitle = wstrtostr(CIN->getTitle());
			if (strcmp(make_lowercaseA(CINtitle).c_str(), make_lowercaseA(CI.Title).c_str())==0) {
				CIN->AddAsset(ITEM_UPDATE_TYPE_BOXART, buffer.GetData(), buffer.GetDataLength());
			}				
		}
	} else {
		CIVec = GetContentByTitleId(TI);
		for (unsigned int x = 0; x < CIVec.size(); x++) {
			DebugMsg("ContentManagar", "Setting Cover to Item");
			ContentItemNew* CIN = CIVec.at(x);
			CIN->AddAsset(ITEM_UPDATE_TYPE_BOXART, buffer.GetData(), buffer.GetDataLength()); 
		}
	}
}


void ContentManager::parseCoverXML(HttpItem* Item) {
	CoverXML xml;
	ATG::XMLParser parser;
	parser.RegisterSAXCallbackInterface(&xml);
	MemoryBuffer& buffer = Item->GetMemoryBuffer();
	parser.ParseXMLBuffer((CHAR*)buffer.GetData(),	buffer.GetDataLength());
	Covers = xml.Covers;
}

ContentItemVector ContentManager::GetContentByTitleId(DWORD titleId)
{
	vector<ContentItemNew*> vContent;
	int nSize = contentItems.size();

	// iterate through all of our currently stored content items, and place into new vector
	for(int nCount = 0; nCount < nSize; nCount++)
	{
		ContentItemNew * pContent = contentItems.at(nCount);
		// if the content matches what we're looking for, let's add it to our list	
		if(pContent->getTitleId() == titleId) {
			vContent.push_back(pContent);
		}
	}

	return vContent;
}

ContentItemNew * ContentManager::GetContentByContentId( DWORD contentId )
{
	ContentItemNew * pContent = NULL;
	int nSize = contentItems.size();

	for( int nCount = 0; nCount < nSize; nCount++)
	{
		pContent = contentItems.at(nCount);
		if(pContent->GetItemId() == contentId)
			break;
	}

	return pContent;
}

//#define ADD_ONE_AT_A_TIME 

void ContentManager::AddContentObserver(ContentManagerObserver& Observer) {
	
	contentObservers.insert(contentItem::value_type(&Observer, &Observer));

	// Our observer was added so lets fire off all the items added at once
	ITEM_LOCK;
#ifdef ADD_ONE_AT_A_TIME
	vector<ContentItemNew*>::iterator iter;
	for(iter = contentItems.begin(); iter != contentItems.end(); ++iter) {
		ContentItemNew * pContent = (*iter);
		Observer.ContentItemAdded(pContent);
	}
#else
	Observer.ContentItemMassAdd(contentItems);
#endif
	Observer.ContentItemCacheLoaded(contentItems.size());
	ITEM_UNLOCK;
}

void ContentManager::RemoveContentObserver(ContentManagerObserver& Observer) {
	contentObservers.erase(&Observer);
}

void ContentManager::ClearObservers() {
	contentObservers.clear();
}
void ContentManager::Initialize() {
	
	// First we must load our db items
	szStatusMsg = L"Database Entries";
	ITEM_LOCK;
	vector<CONTENT_ITEM_LOAD_INFO> dbItems;
	FSDSql::getInstance().LoadContentItemsFromDB(dbItems);
	for(DWORD x = 0; x < dbItems.size(); x++) {
		CONTENT_ITEM_LOAD_INFO& loadInfo = dbItems.at(x);
		ContentItemNew* contentItem = new ContentItemNew(loadInfo, this);
		contentItems.push_back(contentItem);
	}
	ITEM_UNLOCK;

	szStatusMsg = L"Resuming Asset Retrieval";
	for(DWORD x = 0; x < contentItems.size(); x++) {
		ContentItemNew * pContentItem = contentItems.at(x);
		if(pContentItem == NULL) continue;
		
		// If there are outstanding http items, let's readd them to our queue
		if(pContentItem->getHttpItemCount() > 0)
			pContentItem->ResumeGetMarketplaceInfo();

		// If this file container is a xex and has a pending status, we need to readd it to the extractor
		if(pContentItem->GetFileType(true) == CONTENT_FILE_TYPE_XEX_PENDING)
			ContentExtractor::getInstance().AddContentItem(pContentItem);
	}
	szStatusMsg =L"";
	if (SETTINGS::getInstance().getCoverCheck()) {
		CheckCovers();
		if (SETTINGS::getInstance().getCoverPoll() > 0) {
			int pollMS =  SETTINGS::getInstance().getCoverPoll() * 60000;
			if (pollMS != 0)
			 TimerManager::getInstance().add(*this, pollMS);
		}
	}

	// Set the queue to start processing the items that need to be downloaded
	//dwFunction = MANAGER_FUNCTION_DOWNLOAD;
	dwFunction = MANAGER_FUNCTION_SCAN;
	
	// First let's check if auto scanning is disabled before proceeding.
	if(SETTINGS::getInstance().getDisableAutoScan() == FALSE) {
		// Now we can add all scan paths to the queue
		vector<ScanPath> scanPaths = FSDSql::getInstance().getScanPaths();
		vector<ScanPath>::iterator it;
		for (it = scanPaths.begin(); it != scanPaths.end(); it++)
			AddScanPath(*it);
	}
	
	TitleUpdateManager::getInstance().scan();
}

void ContentManager::ScanDirectory(ScanPath* ScanPath, string Directory, int Levels) {
    
	// Check to make sure the default path doesnt already exist in the database
	string defaultFilePath = Directory + "default.xex";
	string defaultPath = defaultFilePath.substr(defaultFilePath.find_first_of(":") + 1);
	DebugMsg("ContentManager", "File Path : %s", defaultFilePath.c_str());
	if(FSDSql::getInstance().ItemPathExistsInDB(defaultPath, ScanPath)) {
		return;
	}

	// Check to make sure the default path doesnt already exist in the database
	defaultFilePath = Directory + "default.xbe";
	defaultPath = defaultFilePath.substr(defaultFilePath.find_first_of(":") + 1);
	DebugMsg("ContentManager", "File Path : %s", defaultFilePath.c_str());
	if(FSDSql::getInstance().ItemPathExistsInDB(defaultPath, ScanPath)) {
		return;
	}


	if(Levels == ScanPath->ScanDepth - 1)
		szStatusMsg = strtowstr(Directory);

	// Try and create default.xex
	ContentItemNew* item = ContentItemNew::CreateItem(ScanPath, Directory, "default.xex");
	if(item != NULL) {
		
		// We found a valid item so no need to continue
		AddItem(item);
		return;
	}

	// Try and create default.xbe
	item = ContentItemNew::CreateItem(ScanPath, Directory, "default.xbe");
	if(item != NULL) {
		
		// We found a valid item so no need to continue
		AddItem(item);
		return;
	}

	// Start finding our files
	string strFind = Directory + "*";
    WIN32_FIND_DATA wfd; HANDLE hFind;
    hFind = FindFirstFile( strFind.c_str(), &wfd );
    if(hFind == INVALID_HANDLE_VALUE) return;

	// Loop and find each
	do {

		// Lets check if this is a directory (and make sure its not our default!)
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			
			string nextDirectory = Directory + wfd.cFileName + "\\";
			if(Levels == -1)
				ScanDirectory(ScanPath, nextDirectory, -1);
			else if(Levels > 0)
				ScanDirectory(ScanPath, nextDirectory, Levels - 1);

			continue;
		}

		// Make sure its not in our db
		string filePath = Directory + wfd.cFileName;
		string filePathShort = filePath.substr(filePath.find_first_of(":") + 1);
		if(FSDSql::getInstance().ItemPathExistsInDB(filePathShort, ScanPath))
			continue;
		
		// Try and create our item
		ContentItemNew* item = ContentItemNew::CreateItem(ScanPath, 
			Directory, wfd.cFileName);
		if(item != NULL)
			AddItem(item);

	}while(FindNextFile(hFind, &wfd));

	// Done finding
	FindClose( hFind );
}
void ContentManager::AddItem(ContentItemNew* ContentItem) {
	
	// First add our item
	ITEM_LOCK;
	ContentItem->SetItemObserver(this);
	contentItems.push_back(ContentItem);
	ITEM_UNLOCK;

	// Now lets notify that we added a item
	ItemAdded(ContentItem);
}
void ContentManager::ItemAdded(ContentItemNew* ContentItem) {

	contentItem::const_iterator it;
	for(it = contentObservers.begin(); it != contentObservers.end(); ++it)
		it->first->ContentItemAdded(ContentItem);
}
void ContentManager::ItemUpdated(ContentItemNew* ContentItem, DWORD UpdateFlags) {
		
	contentItem::const_iterator it;
	for(it = contentObservers.begin(); it != contentObservers.end(); ++it)
		it->first->ContentItemUpdated(ContentItem, UpdateFlags);
}

void ContentManager::ItemDeleted(ContentItemNew* ContentItem) {
	/*
	contentItem::const_iterator it;
	for(it = contentObservers.begin(); it != contentObservers.end(); ++it)
		it->first->ContentItemDeleted(ContentItem);
	*/
}
void ContentManager::NotifyDatabaseLoaded() {
	for(contentItem::const_iterator it = contentObservers.begin(); 
		it != contentObservers.end(); ++it)
		it->first->ContentItemDatabaseLoaded(contentItems.size());
}
void ContentManager::NotifyScanComplete() {
	szStatusMsg = L"";
	for(contentItem::const_iterator it = contentObservers.begin(); 
		it != contentObservers.end(); ++it)
		it->first->ContentItemScanComplete(contentItems.size());
}

void ContentManager::DeleteItem(ContentItemNew* Item) {
	
	// First remove his item from the list
	ITEM_LOCK;
	vector<ContentItemNew*>::iterator iter;
	for(iter = contentItems.begin(); iter != contentItems.end(); iter++) {
		if(*iter == Item) {
			contentItems.erase(iter);
			break;
		}
	}
	ITEM_UNLOCK;

	// Now we can delete our item
	Item->DeleteItem();
	delete Item;
}
CONTENT_ITEM_TAB ContentManager::GetTabIdFromString(string gameType) {

	if(gameType == "XBLA")
		return CONTENT_XBLA;
	if(gameType == "360")
		return CONTENT_360;
	if(gameType == "XBOX1")
		return CONTENT_XBOX1;
	if(gameType == "HOMEBREW")
		return CONTENT_HOMEBREW;
	if(gameType == "EMULATOR")
		return CONTENT_EMULATOR;
	// Special Content Types
	if(gameType == "FAVORITE")
		return CONTENT_FAVORITE;
	if(gameType == "XLINKKAI")
		return CONTENT_XLINKKAI;

	return CONTENT_UNKNOWN;
}
void ContentManager::AddScanPath(ScanPath ScanPath) {

	// We added a scan path so lets add this to the queue
	SCAN_LOCK;
	scanPathQueue.push_back(ScanPath);
	SCAN_UNLOCK;
}
void ContentManager::RemoveScanPath(ScanPath ScanPath) {

	// Remove all the items from this scan path
	ITEM_LOCK; SCAN_LOCK;
	vector<ContentItemNew*>::iterator iter;
Begin:
	for(iter = contentItems.begin(); iter != contentItems.end(); ++iter) {
		ContentItemNew * pContent = (*iter);
		if(pContent->GetScanPathId() == ScanPath.PathId) {
			contentItems.erase(iter);
			pContent->DeleteItem();
			delete pContent;
			goto Begin;
		}
	}
	ITEM_UNLOCK; SCAN_UNLOCK;
}
void ContentManager::UpdateScanPath(ScanPath ScanPath) {

	// Lets remove the path then readd
	RemoveScanPath(ScanPath);
	AddScanPath(ScanPath);
}

unsigned long ContentManager::Process(void* parameter) {

	// Set our thread name and begin processing
	SetThreadName("ContentManager");

	bIsScanning = FALSE;
	// Loop forever
	for(;;)	{

		// Make sure we are not paused
		if(paused) {
			bIsScanning = FALSE;
			Sleep(500);
			continue;
		}

		// Get a item from our queue
		if(scanPathQueue.size() == 0) {
			bIsScanning = FALSE;
			Sleep(500);
			continue;
		}

		bIsScanning = TRUE;
		// Get our item and remove it from the queue
		SCAN_LOCK;
		ScanPath scanPath = scanPathQueue.at(0);
		scanPathQueue.erase(scanPathQueue.begin());
		SCAN_UNLOCK;

		// Now lets scan this directory
		ScanDirectory(&scanPath, scanPath.Path, scanPath.ScanDepth);
		szStatusMsg = L"";

		// Take a little break
		Sleep(500);
	}

	bIsScanning = FALSE;
	// All done
	return 0;
}

void ContentManager::Pause() {
	paused = TRUE;
}
void ContentManager::Resume() {
	paused = FALSE;
}