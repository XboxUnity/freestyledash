#include "stdafx.h"
#include "HttpDownloader.h"
#include "../../Application/FreestyleApp.h"
#include "../../Tools/SQLite/FSDSql.h"

#define HTTP_LOCK   EnterCriticalSection(&lock)
#define HTTP_UNLOCK LeaveCriticalSection(&lock)

HTTPDownloader::HTTPDownloader() {

	// Initialize some stuff
	InitializeCriticalSection(&lock);
	currentStatus = "";
	paused = TRUE;

	// Setup our prioritized queue
	for(DWORD x = HTTP_PRIORITY_START; x <= HTTP_PRIOIRTY_END; x++)
	{
		vector<HttpItem* >* itm = new vector<HttpItem*>();
		m_PriorisedQueue.insert(map<HTTP_PRIORITY, std::vector<HttpItem* >*>::value_type((HTTP_PRIORITY)x, itm));
	}

	// Setup our workers
	for(DWORD x = 0; x < HTTP_NUM_WORKERS; x++)
	{
		HTTPWorker* wrk = new HTTPWorker();
		m_Workers.push_back(wrk);
	}

	// Setup our timer
	TimerManager::getInstance().add(*this, 1000);

	// Now begin our processing loop
	CreateThread(CPU2_THREAD_2);
}

void RestoreFromQueue( DWORD queueID, iHttpItemRequester* Requester  )
{

}

void HTTPDownloader::tick() {

	// Get our last bytes a second
	double lastSecsBytesLoaded = 0;

	// Get our queue size
	DWORD QueueSize = getNbItemsInQueue();

	// Now loop through and get 
	for(DWORD x = 0; x < HTTP_NUM_WORKERS; x++)	{

		if(m_Workers.at(x)->IsBusy()) QueueSize++;
		lastSecsBytesLoaded += m_Workers.at(x)->
			GetBytesDownloadedSinceLastTick();
	}

	// Calculate our kbps
	double kbps = lastSecsBytesLoaded / 1024;

	// Set our status
	if(QueueSize == 0)
		currentStatus = "";
	else {

		string status = sprintfa("%d items left(%.2f kBps)",
			QueueSize, kbps);
		currentStatus = status;
	}
}

HttpItem* HTTPDownloader::CreateHTTPItem(const CHAR* Url, const CHAR* OutputFilePath, REQUEST_METHOD RequestMethod)
{
	// Create a new HTTP Item
	HttpItem * item = new HttpItem(Url, OutputFilePath, RequestMethod);

	item->setDownloader(this);

	// Push this item into our LiveHttpItems list  
	m_LiveHttpItems.insert(LiveItem::value_type(item,item));
	DebugMsg("HTTPDownload", "Queue Size:  %d", m_LiveHttpItems.size());

	return item;
}

void HTTPDownloader::DestroyHTTPItem( HttpItem * item )
{
	
	// Search teh database for the item in teh queue, and remove
	FSDSql::getInstance().HttpQueueRemoveDownload(item);

	m_LiveHttpItems.erase(item);
	if( item != NULL ) {
		delete item;
	}
}

void HTTPDownloader::DownloadCompleted(HttpItem* item)
{
	// forward this download complete notice on to the requester
	if(item!=NULL && item->requester!=NULL) {
		item->requester->DownloadCompleted(item);
	}
}

vector<HttpItem* >* HTTPDownloader::GetQueue(HTTP_PRIORITY priority) {
		
	// Find our queue
	std::vector<HttpItem* >* retVal;
	retVal = m_PriorisedQueue.find(priority)->second;
	
	// Return it
	return retVal;
}
void HTTPDownloader::AddToQueue(HttpItem* ref) {

	// if the app doesnt have an internet connection, skip adding items to the queue
	if(CFreestyleApp::getInstance().hasInternetConnection() == false)
		return;

	if(ref->getRetainQueue())
		FSDSql::getInstance().HttpQueueAddDownload(ref);

	// NEED TO HANDLE INSTANT

	// Get our queue and set add our item
	HTTP_LOCK;
	GetQueue((HTTP_PRIORITY)ref->GetPriority())->push_back(ref);
	HTTP_UNLOCK;
}
void HTTPDownloader::DebugListDownloadQueue() {

	// Loop through all our prioritys
	for(DWORD x = HTTP_PRIORITY_START; x <= HTTP_PRIOIRTY_END; x++)
	{
		// Get our current priority
		std::vector<HttpItem* >* queue = GetQueue((HTTP_PRIORITY)x);
		for(DWORD y = 0; x < queue->size(); x++)
		{
			// Get the item for this queue and output
			HttpItem* currentItem = queue->at(y);
			DebugMsg("HTTPDownloader", "%s", 
				currentItem->getSaveAsPath());
		}
	}
}
unsigned long HTTPDownloader::Process(void* parameter) {

	// Set our thread name and begin processing
	SetThreadName("HTTPClient");

	// Loop forever
	for(;;)	{

		// Make sure we are not paused
		if(paused) {
			Sleep(500);
			continue;
		}

		// Loop through our prioritys and get an item to process
		HttpItem* currentItem = NULL;
		for(DWORD x = HTTP_PRIORITY_START; x <= HTTP_PRIOIRTY_END; x++) {
				
			// Get our queue
			std::vector<HttpItem* >* queue = 
				GetQueue((HTTP_PRIORITY)x);
						
			// Get an item if we have some
			if(queue != NULL && queue->size() > 0) {

				// Get our item and erase it
				HTTP_LOCK;
				currentItem = queue->at(0);
				queue->erase(queue->begin());
				HTTP_UNLOCK;
				break;
			}

			// Restart if we hit the end without an item
			if(x == HTTP_PRIOIRTY_END) {
				Sleep(500);			
				x = HTTP_PRIORITY_START - 1;
			}
		}

		// Loop through our workers and try to assign it
		for(DWORD x = 0; x < HTTP_NUM_WORKERS; x++) {

			// Check if it is busy
			if(!m_Workers.at(x)->IsBusy()) {
			
				// Its not so lets set the item and exit the loop
				m_Workers.at(x)->SetItem(currentItem);
				break;
			}

			// If we are done lets sleep and start again
			if(x == HTTP_NUM_WORKERS - 1) {
				Sleep(500);
				x = -1;
			}
		}

		// Take a little break
		Sleep(500);
	}

	// All done
	return 0;
}
void HTTPDownloader::Pause() {
	paused = TRUE;
	DebugMsg("HTTPDownloader","HTTP Downloader Paused");
}
void HTTPDownloader::Resume() {
	paused = FALSE;
	DebugMsg("HTTPDownloader","HTTP Downloader Resumed");
}
double HTTPDownloader::getBytesLoaded() {

	double retVal = 0;
	for(DWORD x = 0; x < HTTP_NUM_WORKERS; x++)
		retVal += m_Workers.at(x)->GetCurrentItemBytesDownloaded();

	return retVal;
}