#pragma once
#include "../Threads/cthread.h"
#include "HttpItem.h"

using namespace std;

class HTTPWorker : public CThread {
private:

	BOOL			  m_IsDisposed;
	BOOL			  m_IsBusy;
	BOOL			  m_WaitingToStart;
	HttpItem* m_CurrentItem;

	DWORD			  m_bytesLastTick;

	void DownloadCurrentItem();

public:

	void SetItem(HttpItem* newItem);
	unsigned long Process(void* parameter);

	DWORD GetCurrentItemBytesDownloaded();
	DWORD GetCurrentItemBytesToDownload();
	DWORD GetBytesDownloadedSinceLastTick();

	bool IsBusy() { return (m_IsBusy || m_WaitingToStart); }

	HTTPWorker();
	~HTTPWorker();
};