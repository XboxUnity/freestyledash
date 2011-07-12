#include "stdafx.h"
#include "../../Texture/TextureItem/TextureItem.h"
#include "../WorkerThreads/FileTexWorker.h"
#include "FileTexQueue.h"

FileTexQueue::FileTexQueue()
{
	m_terminate = false;
	m_paused = true;
}	

void FileTexQueue::Startup()
{
	m_paused = true;
	m_terminate = false;

	
	for(int x = 0; x < MAX_FILETEXWORKER_COUNT; x++)
	{
		FileTexWorker * wrk = new FileTexWorker();
		m_Workers.push_back(wrk);
	}

	m_paused = false;
	CreateThread(CPU3_THREAD_1);

}

void FileTexQueue::AddToQueue( TextureItem * ref)
{
	this->Lock();
//	printf("FileTexQueue::AddToQueue for TextureItem %08x\n", ref);
//	if (!m_FileTexQueue.empty()) {
//		if(std::find(m_FileTexQueue.begin(), m_FileTexQueue.end(), ref) != m_FileTexQueue.end()) {
//			printf("IT WAS ALREADY ON THE LIST?!\n");
//		} else {
//			m_FileTexQueue.push_back(ref);
//		}
//	}
//	else {
		m_FileTexQueue.push_back(ref);
//	}
	this->Unlock();

}

void FileTexQueue::DebugListFileTexQueue()
{
	for(int x = 0; x < (int)m_Workers.size(); x++)
	{
		FileTexWorker * fileTex = m_Workers.at(x);
		DebugMsg("FileTexQueue", "Is Busy: %s", fileTex->IsBusy() == true ? "true" : "false");
	}
	
	for(unsigned int x = 0; x < (int)m_FileTexQueue.size(); x++)
	{
		TextureItem * currentItem = m_FileTexQueue[x];
 		DebugMsg("FileTexQueue","%s",currentItem->GetTextureFilePath().c_str());
	}

}

unsigned long FileTexQueue::Process(void* parameter)
{
	SetThreadName("FileTexQueue");
	DebugMsg("FileTexQueue", "FileTexQueue Process Thread Running");
	//SetThreadPriority(this->hThread, THREAD_PRIORITY_LOWEST);
	while(!m_terminate)
	{
		if(m_FileTexQueue.size() > 0 &&  !m_paused)
		{
			TextureItem* currentItem = m_FileTexQueue[0];
			m_FileTexQueue.erase(m_FileTexQueue.begin());
			if(currentItem->GetRefCount() > 0)
			{
				bool foundSpot = false;
				while(!foundSpot && !m_paused)
				{
					for(unsigned int x=0;x<m_Workers.size();x++)
					{
						if(!m_Workers.at(x)->IsBusy())
						{
//							printf("%08x FileTexQueue::Process assigning TextureItem %08x to %08x\n", GetCurrentThreadId(), currentItem, m_Workers.at(x));
							m_Workers.at(x)->SetItem(currentItem);
							foundSpot = true;
							break;
						}
					}
					if(!foundSpot)
					{
						Sleep(10);
					}
				}
				if(!foundSpot)
				{
					m_FileTexQueue.push_back(currentItem);
				}
				Sleep(50);
			}
		}
		else
		{
			Sleep(200);
		}

	}

	DebugMsg("FileTexQueue", "FileTexQueue Process Thread Terminated");
	return 0;
}
void FileTexQueue::TerminateAll()
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


void FileTexQueue::Pause()
{
	m_paused = true;
	DebugMsg("FileTexQueue","FileTex Queue Paused");
}


void FileTexQueue::Resume()
{
	m_paused = false;
	DebugMsg("FileTexQueue","FileTex Queue Resumed");
}