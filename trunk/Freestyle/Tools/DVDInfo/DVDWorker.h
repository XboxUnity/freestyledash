#pragma once
#include "../Threads/cthread.h"
#include "DVDItem.h"

using namespace std;

class DVDWorker : public CThread {
private:

	BOOL			  m_IsDisposed;
	BOOL			  m_IsBusy;
	BOOL			  m_WaitingToStart;
	DVDItem*		  m_CurrentItem;

	void ExtractCurrent360Item();
	void ExtractCurrentClassicItem();
	void ExtractCurrentDataDVDItem();
	void ExtractCurrentVideoDVDItem();
	void ExtractCurrentUnknownItem();
	void SetNull();
	void ScanDVDContents();
	void RecursiveScan(string Path, string dest);


public:

	void StartItem(DVDItem* newItem);
	unsigned long Process(void* parameter);
	static DVDWorker& getInstance()
	{
		static DVDWorker singleton;
		return singleton;
	}


	bool IsBusy() { return (m_IsBusy || m_WaitingToStart); }

	DVDWorker();
	~DVDWorker();
	DVDWorker(const DVDWorker&);
	DVDWorker& operator=(const DVDWorker&);
};