#pragma once
#include "stdafx.h"
#include "../../Debug/Debug.h"
#include "LoaderCallback.h"

#include "CallbackMessageTypes.h"

LoaderCallback::LoaderCallback() {
	// Constructor
	m_bPaused = true;
	m_bTerminate = false;
	m_hTerminated = NULL;

	InitializeCriticalSection(&lock);
}

LoaderCallback::~LoaderCallback() {
	// Deconstructor

	TerminateLoaderThread();
	DeleteCriticalSection(&lock);
}

HRESULT LoaderCallback::StartLoaderThread( void ) {

	// Reset the message handler
	m_bPaused = true;
	m_bTerminate = false;
	m_hTerminated = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_bPaused = false;

	// Start up the thread on Core#5
	CreateThread(CON_THREAD_LOADER_CORE);

	return S_OK;
}

HRESULT LoaderCallback::TerminateLoaderThread( void )
{
	bool bError = false;

	m_bPaused = true;
	m_bTerminate = true;

	// Wait to ensure that the thread has returned before proceeding to clear the list
	DWORD retVal = WaitForSingleObject(m_hTerminated, CON_THREAD_TERMINATE_TIMEOUT);
	if(retVal == WAIT_TIMEOUT) {
		DebugMsg("GameViewManager", "MessageQueue:  FATAL ERROR*** Termiantion event timeout period elapsed.");
		bError = true;
	}

	ContentManager::getInstance().RemoveContentObserver(*this);

	// Clear the queue and terminate the thread to release the memory
	ClearLoaderQueue();
	Terminate();

	// return results
	return bError ? S_FALSE : S_OK;
}

HRESULT LoaderCallback::PauseLoaderThread( void )
{
	m_bPaused = true;
	SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
	DebugMsg("GameViewManager", "Message Queue:  Processing thread has been paused.  Thread priority lowered.");
	return S_OK;
}

HRESULT LoaderCallback::ResumeLoaderThread( void )
{
	m_bPaused = false;
	SetThreadPriority(hThread, CON_THREAD_LOADER_PRIORITY);
	DebugMsg("GameViewManager", "Message Queue:  Processing thread has resumed.  Restoring thread priority.");
	return S_OK;
}

HRESULT LoaderCallback::AddToQueue(LoaderMessageData * msg) {
	// Protect our vector while adding a new message into the queue
	EnterCriticalSection(&lock);
	m_vMessageQueue.push_back(msg);
	LeaveCriticalSection(&lock);
	return S_OK;
}

HRESULT LoaderCallback::ClearLoaderQueue( void ) {
	// Protect our queue before clearing it
	EnterCriticalSection(&lock);
	m_vMessageQueue.clear();
	LeaveCriticalSection(&lock);
	return S_OK;
}

DWORD LoaderCallback::GetLoaderQueueSize( void ) {
	return (DWORD)m_vMessageQueue.size();
}

HRESULT LoaderCallback::DumpLoaderQueueToLog( void ) {
	// Protect our queue while iterating through and dumping list to log
	EnterCriticalSection(&lock);
	DWORD dwQueueSize = GetLoaderQueueSize();

	for(DWORD dwCount = 0; dwCount < dwQueueSize; dwCount++) {
		LoaderMessageData * msg = m_vMessageQueue.at(dwCount);
		DebugMsg("GameListManager", "MessageQueue[%d]:  Type=%d, ContentItemPtr=%X, GameListItemPtr=%X, Flags=%X", 
			dwCount, 
			msg->nMsgType, 
			msg->pContentItem, 
			msg->pGameContent,
			msg->dwMessageFlags
		);
	}

	LeaveCriticalSection(&lock);
	return S_OK;
}

unsigned long LoaderCallback::Process( void * parameter )
{
	// Thread to handle list messages
	SetThreadName("GameLoaderCallback");
	SetThreadPriority(hThread, CON_THREAD_LOADER_PRIORITY);

	HandleCompleteAttach();

		// Register class to the ContentObserver
	ContentManager::getInstance().AddContentObserver(*this);

	while(!m_bTerminate) {
		if(!m_bPaused)
		{
			// Thread is active, proceed to process any queued messages
			if( GetLoaderQueueSize() > 0 ) {
				EnterCriticalSection(&lock);
				if (GetLoaderQueueSize() > 0) {
					LoaderMessageData *msg = m_vMessageQueue.at(0);
					m_vMessageQueue.erase(m_vMessageQueue.begin());
					LeaveCriticalSection(&lock);

					CallbackMessageType msgType = msg->nMsgType;
					switch (msgType) {
						case ContentUpdated:		// Received a ContentUpdatedMessage
							OnContentUpdated(msg->pContentItem, msg->dwMessageFlags);
							break;
						case DatabaseLoaded:
							OnContentDatabaseLoaded(msg->dwMessageFlags);
							break;
						case ScanCompleted:
							OnContentScanComplete(msg->dwMessageFlags);
							break;
						case LoadAssets:
							OnContentLoadAssets(msg->pGameContent);
							break;
						case UnknownMessage:		// Received a UnknownMessage
						default:
							DebugMsg("GameViewManager", "MessageQueue:   Ignoring unknown message type.");
					};

					delete msg;
				}
				else {
					LeaveCriticalSection(&lock);
				}
			}
			else 
			{
				// Queue was empty, sleep for 10ms while waiting
				Sleep(CON_THREAD_SLEEP_IDLE);
			}
		}
		else
		{
			// Thread is paused, sleep for now		
			Sleep(CON_THREAD_SLEEP_PAUSE);
		}
	};

	// Terminating thread, trigger event
	SetEvent(m_hTerminated);
	DebugMsg("LoaderCallback", "LoaderCallback Thread has Successfully Terminated");
	return 0;
}

// ContentManagerObserver implementation
void LoaderCallback::ContentItemUpdated(ContentItemNew * pContent, DWORD dwContentFlags)
{
	LoaderMessageData * msg = new LoaderMessageData();
	msg->dwMessageFlags = dwContentFlags;
	msg->nMsgType = ContentUpdated;
	msg->pContentItem = pContent;

	AddToQueue(msg);
}


void LoaderCallback::ContentItemDatabaseLoaded( DWORD ItemCount )
{
	LoaderMessageData * msg = new LoaderMessageData();
	msg->dwMessageFlags = ItemCount;
	msg->nMsgType = DatabaseLoaded;
	msg->pContentItem = NULL;

	AddToQueue(msg);
}

void LoaderCallback::ContentItemScanComplete( DWORD ItemCount )
{
	LoaderMessageData * msg = new LoaderMessageData();
	msg->dwMessageFlags = ItemCount;
	msg->nMsgType = ScanCompleted;
	msg->pContentItem = NULL;

	AddToQueue(msg);
}

void LoaderCallback::ContentItemLoadAssets(GameListItem * pGameContent)
{
	LoaderMessageData * msg = new LoaderMessageData();

	msg->dwMessageFlags = 0;
	msg->nMsgType = LoadAssets;
	msg->pGameContent = pGameContent;
	msg->pContentItem = pGameContent->ContentRef;
	
	AddToQueue(msg);
};