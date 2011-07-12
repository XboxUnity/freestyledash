#include "stdafx.h"
#include "../../Texture/TextureItem/TextureItem.h"
#include "../WorkerThreads/ReleaseWorker.h"
#include "ReleaseQueue.h"

ReleaseQueue::ReleaseQueue()
{
	m_paused = true;
	m_terminate = false;
/*
	for(int x = 0; x < MAX_RELEASEWORKER_COUNT; x++)
	{
		ReleaseWorker * wrk = new ReleaseWorker();
		m_Workers.push_back(wrk);
	}
	//TimerManager::getInstance().add(*this,333);
	CreateThread(CPU3_THREAD_1);
	*/
}	

void ReleaseQueue::Startup()
{
	m_paused = true;
	m_terminate = false;

	
	for(int x = 0; x < MAX_RELEASEWORKER_COUNT; x++)
	{
		ReleaseWorker * wrk = new ReleaseWorker();
		m_Workers.push_back(wrk);
	}

	m_paused = false;
	CreateThread(CPU3_THREAD_1);
}

void ReleaseQueue::AddToQueue( TextureItem * ref, bool bFileData, bool bTextureData, bool bBrushData)
{
	ReleaseItem item;
	item.bBrushData = bBrushData;
	item.bTextureData = bTextureData;
	item.bFileData = bFileData;
	item.ref = ref;

	//DebugMsg("ITemAdded", "AddItem %s, (RefCount:  %d)", ref->GetTextureFilePath().c_str(), ref->GetRefCount());
	Lock();
	m_ReleaseQueue.push_back(item);
	Unlock();
}

void ReleaseQueue::DebugListReleaseQueue()
{
	for(int x = 0; x < (int)m_Workers.size(); x++)
	{
		ReleaseWorker * fileTex = m_Workers.at(x);
		if(fileTex->m_CurrentItem != NULL)
			DebugMsg("ReleaseQueue", "Is Busy: %s, Holding:  %s, RefCount:  %d", fileTex->IsBusy() == true ? "true" : "false", fileTex->m_CurrentItem->GetTextureFilePath().c_str(), fileTex->m_CurrentItem->GetRefCount());
		else
			DebugMsg("ReleaseQueue", "Is Busy:  %s", fileTex->IsBusy() == true ? "true" : "false");
	}
	for(unsigned int x = 0; x < m_ReleaseQueue.size(); x++)
	{
		ReleaseItem currentItem = m_ReleaseQueue[x];
		DebugMsg("ReleaseQueue","%s (%d,%d,%d) - Status:  (%d,%d,%d,%d)",currentItem.ref->GetTextureFilePath().c_str(), currentItem.bFileData, currentItem.bTextureData, currentItem.bBrushData, currentItem.ref->GetCurrentFileState(),currentItem.ref->GetCurrentTextureState(), currentItem.ref->GetCurrentBrushState(), currentItem.ref->GetTextureItemState());
	}
}

unsigned long ReleaseQueue::Process(void* parameter)
{
	SetThreadName("ReleaseQueue");
	DebugMsg("ReleaseQueue", "ReleaseQueue Process Thread Running");
	//SetThreadPriority(this->hThread, THREAD_PRIORITY_LOWEST);
	while(!m_terminate)
	{
		if(m_ReleaseQueue.size() > 0 &&  !m_paused)
		{
			Lock();
			ReleaseItem currentItem = m_ReleaseQueue[0];
			m_ReleaseQueue.erase(m_ReleaseQueue.begin());
			Unlock();
			bool foundSpot = false;
			while(!foundSpot && !m_paused)
			{
				for(unsigned int x=0;x<m_Workers.size();x++)
				{
					if(!m_Workers.at(x)->IsBusy())
					{
						m_Workers.at(x)->SetItem(currentItem.ref, currentItem.bFileData, currentItem.bTextureData, currentItem.bBrushData);
						foundSpot = true;
						break;
					}
				}
				if(!foundSpot)
				{
					Sleep(50);
				}
			}
			if(!foundSpot)
			{
				m_ReleaseQueue.push_back(currentItem);
			}
			Sleep(0);
		}
		else
		{
			Sleep(100);
		}

	}

	DebugMsg("ReleaseQueue", "ReleaseQueue Process Thread Terminated");
	return 0;
}


void ReleaseQueue::Pause()
{
	m_paused = true;
	DebugMsg("ReleaseQueue","Release Queue Paused");
}

void ReleaseQueue::TerminateAll()
{
	m_paused = true;
	m_terminate = true;
	for(int i = 0; i < (int)m_Workers.size(); i++)
	{
		m_Workers.at(i)->m_IsDisposed = true;
		delete m_Workers.at(i);
	}
	
	m_Workers.clear();

	this->Terminate();
}

void ReleaseQueue::Resume()
{
	m_paused = false;
	DebugMsg("ReleaseQueue","Release Queue Resumed");
}