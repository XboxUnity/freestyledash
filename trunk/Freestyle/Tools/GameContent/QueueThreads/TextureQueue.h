#pragma once

#include "../../Threads/CThread.h"
#include "../../Debug/Debug.h"
#include "../../Texture/TextureItem/TextureItem.h"
#include "../WorkerThreads/TextureWorker.h"

#define MAX_TEXTUREWORKER_COUNT 5

class TextureQueue : public CThread//, public iTimerObserver
{
public:
	//void tick();
	static TextureQueue& getInstance()
	{
		static TextureQueue singleton;
		return singleton;
	}

	void AddToQueue(TextureItem * ref);

	int getNbItemsInQueue()
	{
		int retval;
		Lock();
//		printf("getNbItemsInQueue called\n");
		retval = m_TextureQueue.size();
		Unlock();
		return retval;
	}

	void ClearQueue()
	{
		Lock();
//		printf("ClearQueue called\n");
		m_TextureQueue.clear();
		Unlock();
	}

	void DebugListTextureQueue();

	void TerminateAll();
	void Pause();
	void Resume();
	void Startup();

private:
	unsigned long Process(void* parameter);
	bool m_paused;
	bool m_terminate;

	std::vector<TextureWorker*> m_Workers;
	std::vector<TextureItem* > m_TextureQueue;
	//void _NotifyDownloadCompleted(HttpItem* ref);

	TextureQueue();
	~TextureQueue() {}
	TextureQueue(const TextureQueue&);                 // Prevent copy-construction
	TextureQueue& operator=(const TextureQueue&);      // Prevent assignment
};
