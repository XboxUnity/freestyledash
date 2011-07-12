#include "stdafx.h"
#include "../../Texture/TextureItem/TextureItem.h"
#include "../WorkerThreads/FileWorker.h"
#include "FileQueue.h"

FileQueue::FileQueue()
{
	m_paused = true;
	m_terminate = false;
	m_priority = 0;

/*
	for(int x = 0; x < MAX_FILEWORKER_COUNT; x++)
	{
		FileWorker * wrk = new FileWorker();
		m_Workers.push_back(wrk);
	}
	//TimerManager::getInstance().add(*this,333);
	CreateThread(CPU3_THREAD_1);*/
}

void FileQueue::Startup()
{
	m_paused = true;
	m_terminate = false;

	
	for(int x = 0; x < MAX_FILEWORKER_COUNT; x++)
	{
		FileWorker * wrk = new FileWorker();
		m_Workers.push_back(wrk);
	}

	m_paused = false;
	CreateThread(CPU3_THREAD_1);
}

void FileQueue::AddToQueue( TextureItem * ref)
{
	m_FileQueue.push_back(ref);

}

void FileQueue::DebugListFileQueue()
{
	for(unsigned int x = 0; x < m_FileQueue.size(); x++)
	{
		TextureItem * currentItem = m_FileQueue[x];
		DebugMsg("FileQueue","%s",currentItem->GetTextureFilePath().c_str());
	}
}

unsigned long FileQueue::Process(void* parameter)
{
	SetThreadName("FileQueue");
	DebugMsg("FileQueue", "FileQueue Process Thread Running");
	//SetThreadPriority(this->hThread, THREAD_PRIORITY_LOWEST);
	while(!m_terminate)
	{
		if(m_FileQueue.size() > 0 &&  !m_paused)
		{
			long priority = -1;
			int index = 0;
			for (unsigned int i=0; i< m_FileQueue.size(); i++) {
				TextureItem * pTex = m_FileQueue.at(i);

				if (pTex->GetPriority() > priority) {
					priority = pTex->GetPriority();
					index = i;
				}
			}

			TextureItem* currentItem = m_FileQueue[index];
			m_FileQueue.erase(m_FileQueue.begin()+index);
			bool foundSpot = false;
			while(!foundSpot && !m_paused)
			{
				for(unsigned int x=0;x<m_Workers.size();x++)
				{
					if(!m_Workers.at(x)->IsBusy())
					{
						m_Workers.at(x)->SetItem(currentItem);
						foundSpot = true;
						break;
					}
				}
				if(!foundSpot)
				{
					Sleep(100);
				}
			}
			if(!foundSpot)
			{
				m_FileQueue.push_back(currentItem);
			}
			Sleep(0);
		}
		else
		{
			Sleep(100);
		}

	}

	DebugMsg("FileQueue", "FileQueue Process Thread Terminated");

	return 0;
}

void FileQueue::TerminateAll()
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


void FileQueue::Pause()
{
	m_paused = true;
	DebugMsg("FileQueue","File Queue Paused");
}


void FileQueue::Resume()
{
	m_paused = false;
	DebugMsg("FileQueue","FileQueue Resumed");
}