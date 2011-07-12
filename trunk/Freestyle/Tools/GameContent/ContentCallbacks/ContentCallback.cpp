#pragma once
#include "stdafx.h"
#include "../../Debug/Debug.h"
#include "ContentCallback.h"

#include "CallbackMessageTypes.h"

ContentCallback::ContentCallback() {
	// Constructor
	m_bPaused = true;
	m_bTerminate = false;
	m_hTerminated = NULL;

	InitializeCriticalSection(&lock);
}

ContentCallback::~ContentCallback() {
	// Deconstructor

	TerminateContentThread();
	DeleteCriticalSection(&lock);
}

HRESULT ContentCallback::StartContentThread( void ) {

	// Reset the message handler
	m_bPaused = true;
	m_bTerminate = false;
	m_hTerminated = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_bPaused = false;

	// Start up the thread on Core#5
	CreateThread(CON_THREAD_CONTENT_CORE);

	return S_OK;
}

HRESULT ContentCallback::TerminateContentThread( void )
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
	ClearContentQueue();
	Terminate();

	// return results
	return bError ? S_FALSE : S_OK;
}

HRESULT ContentCallback::PauseContentThread( void )
{
	m_bPaused = true;
	SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
	DebugMsg("GameViewManager", "Message Queue:  Processing thread has been paused.  Thread priority lowered.");
	return S_OK;
}

HRESULT ContentCallback::ResumeContentThread( void )
{
	m_bPaused = false;
	SetThreadPriority(hThread, CON_THREAD_CONTENT_PRIORITY);
	DebugMsg("GameViewManager", "Message Queue:  Processing thread has resumed.  Restoring thread priority.");
	return S_OK;
}

HRESULT ContentCallback::AddToQueue(ContentMessageData * msg) {
	// Protect our vector while adding a new message into the queue
	EnterCriticalSection(&lock);
	m_vMessageQueue.push_back(msg);
	LeaveCriticalSection(&lock);
	return S_OK;
}

HRESULT ContentCallback::ClearContentQueue( void ) {
	// Protect our queue before clearing it
	EnterCriticalSection(&lock);
	m_vMessageQueue.clear();
	LeaveCriticalSection(&lock);
	return S_OK;
}

DWORD ContentCallback::GetContentQueueSize( void ) {
	return (DWORD)m_vMessageQueue.size();
}

HRESULT ContentCallback::DumpContentQueueToLog( void ) {
	// Protect our queue while iterating through and dumping list to log
	EnterCriticalSection(&lock);
	DWORD dwQueueSize = GetContentQueueSize();

	for(DWORD dwCount = 0; dwCount < dwQueueSize; dwCount++) {
		ContentMessageData * msg = m_vMessageQueue.at(dwCount);
		DebugMsg("GameListManager", "MessageQueue[%d]:  Type=%d, ContentItemPtr=%X, Flags=%X", 
			dwCount, 
			msg->nMsgType, 
			msg->pContentItem, 
			msg->dwMessageFlags
		);
	}

	LeaveCriticalSection(&lock);
	return S_OK;
}

unsigned long ContentCallback::Process( void * parameter )
{
	// Thread to handle list messages
	SetThreadName("GameContentCallback");
	SetThreadPriority(hThread, CON_THREAD_CONTENT_PRIORITY);

		// Register class to the ContentObserver
	ContentManager::getInstance().AddContentObserver(*this);

	while(!m_bTerminate) {
		if(!m_bPaused)
		{
			// Thread is active, proceed to process any queued messages
			if( GetContentQueueSize() > 0 ) {
				EnterCriticalSection(&lock);
				ContentMessageData *msg = m_vMessageQueue.at(0);
				m_vMessageQueue.erase(m_vMessageQueue.begin());
				LeaveCriticalSection(&lock);

				CallbackMessageType msgType = msg->nMsgType;
				switch (msgType) {
					case ContentAdded:			// Received a ContentAdded Message
						OnContentAdded(msg->pContentItem);
						break;
					case ContentMassAdd:
						OnContentMassAdd(msg->pVec);
						break;
					case CacheLoaded:
						OnContentCacheLoaded(msg->dwMessageFlags);
						break;
					case UnknownMessage:		// Received a UnknownMessage
					default:
						DebugMsg("GameViewManager", "MessageQueue:   Ignoring unknown message type.");
				};

				delete msg;
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

	DebugMsg("ContentCallback", "ContentCallback Thread has Successfully Terminated");
	return 0;
}

// ContentManagerObserver implementation
void ContentCallback::ContentItemAdded(ContentItemNew * pContent)
{
	ContentMessageData * msg = new ContentMessageData();
	msg->dwMessageFlags = 0;
	msg->nMsgType = ContentAdded;
	msg->pContentItem = pContent;

	AddToQueue(msg);
}

void ContentCallback::ContentItemMassAdd(vector<ContentItemNew *> items)
{
	ContentMessageData * msg = new ContentMessageData();
	msg->dwMessageFlags = 0;
	msg->nMsgType = ContentMassAdd;
	msg->pVec = items;
	AddToQueue(msg);
}


void ContentCallback::ContentItemCacheLoaded( DWORD ItemCount )
{
	ContentMessageData * msg = new ContentMessageData();
	msg->dwMessageFlags = ItemCount;
	msg->nMsgType = CacheLoaded;
	msg->pContentItem = NULL;

	AddToQueue(msg);
}