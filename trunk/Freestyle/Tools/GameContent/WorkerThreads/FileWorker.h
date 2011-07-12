#pragma once
#include "../../Threads/cthread.h"
#include "../../Texture/TextureItem/TextureItem.h"

using namespace std;

class FileWorker : public CThread
{
private :
	bool m_IsBusy;
	bool m_WaitingToStart;
	TextureItem * m_CurrentItem;
public :
	bool m_IsDisposed;
	void SetItem(TextureItem* newItem);
	unsigned long Process(void* parameter);
	void LoadCurrentItem();
	//double GetCurrentItemBytesDownloaded();
	bool IsBusy()
	{
		if(m_IsBusy || m_WaitingToStart)
			return true;
		else
			return false;
	}
	FileWorker(){
		m_WaitingToStart = false;
		m_CurrentItem = NULL;
		m_IsBusy = false;
		m_IsDisposed = false;
		CreateThread(CPU2_THREAD_1);
	};
	~FileWorker(){
	m_IsDisposed = true;
	};
};