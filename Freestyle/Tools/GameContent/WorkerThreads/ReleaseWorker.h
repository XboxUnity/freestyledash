#pragma once
#include "../../Threads/cthread.h"
#include "../../Texture/TextureItem/TextureItem.h"

using namespace std;

class ReleaseWorker : public CThread
{
private :
	bool m_IsBusy;
	bool m_WaitingToStart;

	bool m_releaseFile;
	bool m_releaseTex;
	bool m_releaseBrush;
public :
	bool m_IsDisposed;
	TextureItem * m_CurrentItem;
	void SetItem(TextureItem* newItem, bool releaseFile, bool releaseTexture, bool releaseBrush);
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
	ReleaseWorker(){
		m_WaitingToStart = false;
		m_CurrentItem = NULL;
		m_IsBusy = false;
		m_IsDisposed = false;
		CreateThread(CPU3_THREAD_2);
	};
	~ReleaseWorker(){
	m_IsDisposed = true;
	};
};