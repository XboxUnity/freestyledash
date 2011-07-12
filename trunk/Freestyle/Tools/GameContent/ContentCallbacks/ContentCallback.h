#pragma once

#include "../../ContentList/ContentManager.h"
#include "../../ContentList/ContentItemNew.h"
#include "../../Threads/CThread.h"

#include "CallbackMessageTypes.h"

class ContentCallback : public CThread, private ContentManagerObserver
{
public:
	HRESULT ClearContentQueue( void );
	DWORD GetContentQueueSize( void );
	HRESULT DumpContentQueueToLog( void );

	HRESULT StartContentThread( void );
	HRESULT TerminateContentThread( void );
	HRESULT PauseContentThread( void );
	HRESULT ResumeContentThread( void );

	unsigned long Process(void* parameter);

	virtual void OnContentAdded(ContentItemNew * pContent) {}
	virtual void OnContentMassAdd(vector<ContentItemNew*> items) {}
	virtual void OnContentCacheLoaded( DWORD dwItemCount ) {}

	void ContentItemAdded(ContentItemNew * pContent);
	void ContentItemMassAdd(vector<ContentItemNew*> items);
	void ContentItemUpdated(ContentItemNew * pContent, DWORD pContentFlags) {}
	void ContentItemCacheLoaded( DWORD ItemCount );
	void ContentItemDatabaseLoaded( DWORD ItemCount ) {}
	void ContentItemScanComplete( DWORD ItemCount ) {}

	ContentCallback();
	~ContentCallback();

private:
	HRESULT AddToQueue(ContentMessageData * msg);

	bool m_bPaused;
	bool m_bTerminate;
	HANDLE m_hTerminated;

	// Message Queue
	std::vector<ContentMessageData*> m_vMessageQueue;

	CRITICAL_SECTION lock;
};