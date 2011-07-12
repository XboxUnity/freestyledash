#include "stdafx.h"
#include "../../Texture/TextureItem/TextureItem.h"
#include "../WorkerThreads/TextureWorker.h"
#include "TextureQueue.h"

TextureQueue::TextureQueue()
{
	m_paused = true;
	m_terminate = false;
	//InitializeCriticalSection(&lock);

/*	for(int x = 0; x < MAX_TEXTUREWORKER_COUNT; x++)
	{
		TextureWorker * wrk = new TextureWorker();
		m_Workers.push_back(wrk);
	}
	//TimerManager::getInstance().add(*this,333);
	CreateThread(CPU3_THREAD_1);
	*/
}	

void TextureQueue::Startup()
{
	m_paused = true;
	m_terminate = false;
	
	for(int x = 0; x < MAX_TEXTUREWORKER_COUNT; x++)
	{
		TextureWorker * wrk = new TextureWorker();
		m_Workers.push_back(wrk);
	}

	m_paused = false;
	CreateThread(CPU3_THREAD_1);
}

void TextureQueue::AddToQueue( TextureItem * ref)
{
	Lock();
//	printf("AddToQueue called\n");
	m_TextureQueue.push_back(ref);
	Unlock();
}

void TextureQueue::DebugListTextureQueue()
{
	for(unsigned int x = 0; x < m_TextureQueue.size(); x++)
	{
		TextureItem * currentItem = m_TextureQueue[x];
		DebugMsg("TextureQueue","%s",currentItem->GetTextureFilePath().c_str());
	}
}

unsigned long TextureQueue::Process(void* parameter)
{
	SetThreadName("TextureQueue");
	DebugMsg("TextureQueue", "FileTexQueue Process Thread Running");
	//SetThreadPriority(this->hThread, THREAD_PRIORITY_LOWEST);
	while(!m_terminate)
	{
		if(m_TextureQueue.size() > 0 &&  !m_paused)
		{
			long priority = -1;
			int index = 0;
			for (unsigned int i=0; i< m_TextureQueue.size(); i++) {
				TextureItem * pTex = m_TextureQueue.at(i);

				if (pTex->GetPriority() > priority) {
					priority = pTex->GetPriority();
					index = i;
				}
			}

			TextureItem* currentItem = m_TextureQueue[index];
//			printf("BEFORE size of the textureQueue is %d and index is %d\n", m_TextureQueue.size(), index);
			m_TextureQueue.erase(m_TextureQueue.begin()+index);
//			printf("AFTER size of the textureQueue is %d and index is %d\n", m_TextureQueue.size(), index);
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
					Sleep(50);
				}
			}
			if(!foundSpot)
			{
				m_TextureQueue.push_back(currentItem);
			}
			Sleep(0);
		}
		else
		{
			Sleep(100);
		}

	}
	DebugMsg("TextureQueue", "TextureQueue Process Thread Terminated");
	return 0;
}

void TextureQueue::TerminateAll()
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


void TextureQueue::Pause()
{
	m_paused = true;
	DebugMsg("TextureQueue","Texture Queue Paused");
}


void TextureQueue::Resume()
{
	m_paused = false;
	DebugMsg("TextureQueue","TextureQueue Resumed");
}