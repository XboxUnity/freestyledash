#pragma once

#include "../../Threads/CThread.h"
#include "../../Debug/Debug.h"
#include "../../Texture/TextureItem/TextureItem.h"
#include "../WorkerThreads/ReleaseWorker.h"

#define MAX_RELEASEWORKER_COUNT 10

typedef struct
{
	bool bFileData;
	bool bTextureData;
	bool bBrushData;
	TextureItem * ref;
} ReleaseItem;

class ReleaseQueue : public CThread//, public iTimerObserver
{
public:
	//void tick();
	void AddToQueue( TextureItem * ref, bool bFileData, bool bTextureData, bool bBrushData);

	int getNbItemsInQueue()
	{
		return m_ReleaseQueue.size();
	}

	static ReleaseQueue& getInstance()
	{
		static ReleaseQueue singleton;
		return singleton;
	}

	void ClearQueue()
	{
		m_ReleaseQueue.clear();
	}


	void DebugListReleaseQueue();

	unsigned long Process(void* parameter);
	void TerminateAll();
	void Pause();
	void Resume();
	void Startup();

private:

	bool m_paused;
	bool m_terminate;

	std::vector<ReleaseWorker*> m_Workers;
	std::vector<ReleaseItem> m_ReleaseQueue;
	//void _NotifyDownloadCompleted(HttpItem* ref);

	ReleaseQueue();
	~ReleaseQueue() {}
	ReleaseQueue(const ReleaseQueue&);                 // Prevent copy-construction
	ReleaseQueue& operator=(const ReleaseQueue&);      // Prevent assignment
};
