#pragma once

#include "../../Threads/CThread.h"
#include "../../Debug/Debug.h"
#include "../../Texture/TextureItem/TextureItem.h"
#include "../WorkerThreads/FileTexWorker.h"

#define MAX_FILETEXWORKER_COUNT 6

class FileTexQueue : public CThread//, public iTimerObserver
{
public:
	//void tick();
	void AddToQueue(TextureItem * ref);

	int getNbItemsInQueue()
	{
		return m_FileTexQueue.size();
	}

	static FileTexQueue& getInstance()
	{
		static FileTexQueue singleton;
		return singleton;
	}

	void ClearQueue()
	{
		m_FileTexQueue.clear();
	}

	void DebugListFileTexQueue();

	unsigned long Process(void* parameter);
	void TerminateAll();
	void Pause();
	void Resume();
	void Startup();
private:

	bool m_paused;
	bool m_terminate;

	std::vector<FileTexWorker*> m_Workers;
	std::vector<TextureItem* > m_FileTexQueue;
	//void _NotifyDownloadCompleted(HttpItem* ref);

	FileTexQueue();
	~FileTexQueue() {}
	FileTexQueue(const FileTexQueue&);                 // Prevent copy-construction
	FileTexQueue& operator=(const FileTexQueue&);      // Prevent assignment
};
