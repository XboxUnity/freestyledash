#pragma once

#include "../../ContentList/ContentManager.h"
#include "../../ContentList/ContentItemNew.h"
#include "../../Managers/GameList/GameListItem.h"
#include "../../Threads/CThread.h"

#include "CallbackMessageTypes.h"

class LoaderCallback : public CThread, private ContentManagerObserver
{
public:
	HRESULT ClearLoaderQueue( void );
	DWORD GetLoaderQueueSize( void );
	HRESULT DumpLoaderQueueToLog( void );

	HRESULT StartLoaderThread( void );
	HRESULT TerminateLoaderThread( void );
	HRESULT PauseLoaderThread( void );
	HRESULT ResumeLoaderThread( void );

	virtual void OnContentUpdated(ContentItemNew * pContent, DWORD dwContentFlags) {}
	virtual void OnContentDatabaseLoaded( DWORD dwItemCount ) {}
	virtual void OnContentScanComplete( DWORD dwItemCount ) {}
	virtual void OnContentLoadAssets( GameListItem * pGameContent ) {}

	virtual HRESULT HandleCompleteAttach( void ) { return S_OK; }

	void ContentItemAdded(ContentItemNew * pContent) {}
	void ContentItemMassAdd(vector<ContentItemNew*> items) {};
	void ContentItemUpdated(ContentItemNew * pContent, DWORD pContentFlags);
	void ContentItemCacheLoaded( DWORD ItemCount ) {}
	void ContentItemDatabaseLoaded( DWORD ItemCount );
	void ContentItemScanComplete( DWORD ItemCount );

	void ContentItemLoadAssets(GameListItem * pGameContent);

	unsigned long Process(void* parameter);

	LoaderCallback();
	~LoaderCallback();

private:
	HRESULT AddToQueue(LoaderMessageData * msg);

	bool m_bPaused;
	bool m_bTerminate;
	HANDLE m_hTerminated;

	// Message Queue
	std::vector<LoaderMessageData*> m_vMessageQueue;

	CRITICAL_SECTION lock;
};