#pragma once
#include "../../Threads/cthread.h"
#include "FileOperationManager.h"
#include "../Timers/TimerManager.h"
#include "FileCallBackItem.h"


using namespace std;

class FileCallBack : public CThread, iTimerObserver {
private:

	BOOL						m_IsDisposed;
	BOOL						m_IsBusy;
	BOOL						m_WaitingToStart;
	FileCallBackItem*			m_CurrentItem;
	bool						m_Paused;
	
	vector<FileInformation>		m_Queue;
	
	bool						cancelTime;


public:
	void tick();

	void		addSingleOperation(FileCallBackItem* Itm, FileInformation file, string destination, bool isCut);
	void		addVectorOperation(FileCallBackItem* Itm, vector<FileInformation> file, string destination, bool isCut);

	void		cancel();

	unsigned long Process(void* parameter);

	static FileCallBack& getInstance()
	{
		static FileCallBack singleton;
		return singleton;
	}


	bool IsBusy() { return (m_IsBusy || m_WaitingToStart); }

	FileCallBack();
	~FileCallBack();
	FileCallBack(const FileCallBack&);
	FileCallBack& operator=(const FileCallBack&);
};