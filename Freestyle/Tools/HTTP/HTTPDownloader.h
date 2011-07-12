#pragma once

#include "HttpItem.h"
#include "../Threads/CThread.h"
#include "./Base/HttpClient.h"
#include "../Debug/Debug.h"
#include "../Managers/VariablesCache/VariablesCache.h"
#include "../TextScroller/TextScroller.h"
#include "../Managers/Timers/TimerManager.h"
#include "HTTPWorker.h"
#include <list>

#define HTTP_NUM_WORKERS    5

class HTTPDownloader:public CThread, public iTimerObserver, iHttpItemRequester
{
public:

	void tick();
	void AddToQueue(HttpItem* ref);

	string getStatus(){ return currentStatus; }
	DWORD getNbItemsInQueue() {

		DWORD retVal = 0;
		for(DWORD x = HTTP_PRIORITY_START; x <= HTTP_PRIOIRTY_END; x++)
		{
			vector<HttpItem* >* queue = GetQueue((HTTP_PRIORITY)x);
			retVal += queue->size();
		}		
		return retVal;
	}

	static HTTPDownloader& getInstance()
	{
		static HTTPDownloader singleton;
		return singleton;
	}

	void DebugListDownloadQueue();

	void DestroyHTTPItem( HttpItem * item );

	HttpItem* CreateHTTPItem(const CHAR* Url, const CHAR* OutputFilePath = NULL, 
		REQUEST_METHOD RequestMethod = REQUEST_METHOD_GET);

	unsigned long Process(void* parameter);
	void Pause();
	void Resume();
	double getBytesLoaded();

	void DownloadCompleted(HttpItem*);

private:
	
	CRITICAL_SECTION lock;
	string currentStatus;
	BOOL   paused;

	std::vector<HTTPWorker*> m_Workers;
	std::map<HTTP_PRIORITY, std::vector<HttpItem*>*> m_PriorisedQueue;

	std::map<HttpItem* const,HttpItem* const> m_LiveHttpItems;
	typedef std::map<HttpItem* const,HttpItem* const> LiveItem;

	std::vector<HttpItem* >* GetQueue(HTTP_PRIORITY priority);	
	void NotifyDownloadCompleted(HttpItem* ref);

	HTTPDownloader();
	~HTTPDownloader() {}
	HTTPDownloader(const HTTPDownloader&);
	HTTPDownloader& operator=(const HTTPDownloader&);
};