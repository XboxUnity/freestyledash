#include "stdafx.h"

#include "HttpItem.h"
#include "../Threads/CThread.h"
#include "./Base/HttpClient.h"
#include "../Debug/Debug.h"
#include "HTTPWorker.h"
#include "HTTPDownloader.h"

HTTPWorker::HTTPWorker() {

	m_WaitingToStart = FALSE;
	m_CurrentItem = NULL;
	m_IsBusy = FALSE;
	m_IsDisposed = FALSE;
	m_bytesLastTick = 0;
	CreateThread(CPU2_THREAD_2);
}
HTTPWorker::~HTTPWorker() {
	m_IsDisposed = TRUE;
}
void HTTPWorker::SetItem(HttpItem* newItem) {

	// Wait till we are not busy
	while(IsBusy())
		Sleep(100);

	m_WaitingToStart = TRUE;
	m_CurrentItem = newItem;
}
DWORD HTTPWorker::GetCurrentItemBytesDownloaded() {

	if(m_CurrentItem != NULL)
		return m_CurrentItem->outputBytesDownloaded;

	return 0;
}
DWORD HTTPWorker::GetCurrentItemBytesToDownload() {
		
	if(m_CurrentItem != NULL)
		return m_CurrentItem->outputBytesToDownload;

	return 0;
}
DWORD HTTPWorker::GetBytesDownloadedSinceLastTick() {
	
	// Get how many bytes we downloaded
	DWORD bytesDownloaded = GetCurrentItemBytesDownloaded();
	DWORD difference = 0;
	if (m_bytesLastTick <= bytesDownloaded) {
		difference = bytesDownloaded - m_bytesLastTick;
	}
	m_bytesLastTick = bytesDownloaded;

	return difference;
}
void HTTPWorker::DownloadCurrentItem() {

	// Create our client
	HttpClient m_HttpClient;
	m_HttpClient.ExecuteItem(m_CurrentItem);
	
	// Set our responce into our item
	m_CurrentItem->responseCode = m_HttpClient.GetResponseCode();
	m_CurrentItem->completed = TRUE;

	// Trigger the download complete function
	if(m_CurrentItem->downloader != NULL)
		m_CurrentItem->downloader->DownloadCompleted(m_CurrentItem);
}
unsigned long HTTPWorker::Process(void* parameter) {

	// Set our name
	SetThreadName("HTTPWorker");

	// Now loop while we are not disposed
	while(!m_IsDisposed){

		// If our item is not null lets download
		if(m_CurrentItem != NULL)
		{
			m_IsBusy = TRUE;
			m_WaitingToStart = FALSE;
			m_bytesLastTick = 0;

			// Download
			DownloadCurrentItem();

			// Dispose of this item
			HTTPDownloader::getInstance().DestroyHTTPItem(m_CurrentItem);

			// Set our item as null
			m_CurrentItem = NULL;

			// Waiting to start
			m_WaitingToStart = FALSE;
			m_IsBusy = FALSE;
		}

		// Sleep
		Sleep(100);
	}

	// Out of our loop
	return 0;
}