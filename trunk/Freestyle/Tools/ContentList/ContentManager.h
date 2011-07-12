#pragma once

#include "ContentItemNew.h"
#include "../Settings/Settings.h"
#include "CoverXML/CoverXML.h"

typedef enum _MANAGER_STATUS {
	MANAGER_STATUS_NONE          = 0,
	MANAGER_STATUS_LOADING_CACHE = 1,
	MANAGER_STATUS_SCANNING      = 2,
	MANAGER_STATUS_COMPLETE      = 3
} MANAGER_STATUS;

typedef enum _MANAGER_FUNCTION {
	MANAGER_FUNCTION_IDLE,
	MANAGER_FUNCTION_DOWNLOAD,
	MANAGER_FUNCTION_SCAN
} MANAGER_FUNCTION;

class ContentManagerObserver {
public:

	virtual void ContentItemAdded(ContentItemNew* ContentItem) = 0;
	virtual void ContentItemMassAdd(vector<ContentItemNew*> items) = 0;
	virtual void ContentItemUpdated(ContentItemNew* ContentItem, DWORD UpdateFlags) = 0;
	//virtual void ContentItemDeleted(ContentItemNew* ContentItem) = 0;
	virtual void ContentItemDatabaseLoaded(DWORD ItemCount) = 0;
	virtual void ContentItemScanComplete(DWORD ItemCount) = 0;
	virtual void ContentItemCacheLoaded(DWORD ItemCount) = 0;
};

class ContentManager : public CThread, 
	public ContentItemObserver, public iTimerObserver, public iHttpItemRequester {
private:

	CRITICAL_SECTION scanLock;
	CRITICAL_SECTION itemLock;

	BOOL paused;
	BOOL bIsScanning;
	MANAGER_FUNCTION dwFunction;
	vector<ContentItemNew*> httpContentItems;
	vector<ContentItemNew*> contentItems;
	vector<ScanPath> scanPathQueue;

	std::map<ContentManagerObserver* const, ContentManagerObserver* const> contentObservers;
	typedef std::map<ContentManagerObserver* const, ContentManagerObserver* const> contentItem;
		
	unsigned long Process(VOID* Param);
	
	void ItemAdded(ContentItemNew* ContentItem);
	void NotifyDatabaseLoaded();
	void NotifyScanComplete();
	void DownloadCompleted(HttpItem* itm);
	void parseCoverXML(HttpItem* Item);
	void ProcessCover(HttpItem* Item, CoverInfo CI);
	void tick();
	vector<CoverInfo> Covers;
	void ScanDirectory(ScanPath* ScanPath, string Directory, int Levels);

	void AddItem(ContentItemNew* ContentItem);
	void ItemUpdated(ContentItemNew* ContentItem, DWORD UpdateFlags);
	void ItemDeleted(ContentItemNew* ContentItem);

public:
	

	string GetCurrentStatus() { return wstrtostr(szStatusMsg); }

	static ContentManager& getInstance() {
		static ContentManager singleton;
		return singleton;
	}

	MANAGER_STATUS	 Status;
	wstring szStatusMsg;

	ContentManager(  );
	~ContentManager();

	void Initialize();
	void Pause();
	void Resume();	
	
	void AddContentObserver(ContentManagerObserver& Observer);
	void RemoveContentObserver(ContentManagerObserver& Observer);
	void ClearObservers();

	void AddScanPath(ScanPath ScanPath);
	void RemoveScanPath(ScanPath ScanPath);
	void UpdateScanPath(ScanPath ScanPath);
	void LoadTitleUpdates();
	void CheckCovers();
	void UpdateTimer();
	int TickCount;

	BOOL IsCurrentlyScanning(void) { return bIsScanning; }

	ContentItemVector GetContentByItemTab(CONTENT_ITEM_TAB itemTab);
	ContentItemVector GetContentByTitleId(DWORD titleId);
	ContentItemNew * GetContentByContentId( DWORD contentId );
	
	void DeleteItem(ContentItemNew* Item);

	CONTENT_ITEM_TAB GetTabIdFromString(string gameType);
};