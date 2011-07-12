#pragma once

#include "../../Threads/CThread.h"
#include "../../Debug/Debug.h"
#include "../../Texture/TextureItem/TextureItem.h"
#include "../WorkerThreads/FileWorker.h"

#define MAX_FILEWORKER_COUNT 5

class FileQueue : public CThread//, public iTimerObserver
{
public:
	//void tick();
	void AddToQueue(TextureItem * ref);

	int getNbItemsInQueue()
	{
		return m_FileQueue.size();
	}

	void ClearQueue()
	{
		m_FileQueue.clear();
	}

	static FileQueue& getInstance()
	{
		static FileQueue singleton;
		return singleton;
	}

	void DebugListFileQueue();

	unsigned long Process(void* parameter);
	void TerminateAll();
	void Pause();
	void Resume();
	void Startup();

private:

	bool m_paused;
	bool m_terminate;
	long m_priority;

	std::vector<FileWorker*> m_Workers;
	std::vector<TextureItem *> m_FileQueue;
	//void _NotifyDownloadCompleted(HttpItem* ref);

	FileQueue();
	~FileQueue() {}
	FileQueue(const FileQueue&);                 // Prevent copy-construction
	FileQueue& operator=(const FileQueue&);      // Prevent assignment
};