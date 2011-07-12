#include "../Threads/cthread.h"
#include "ContentItemNew.h"
#include "../XEX/XeXtractor.h"

#pragma once

class ContentExtractor : public CThread
{
private:

	vector<ContentItemNew*> itemQueue;
	BOOL paused;

	unsigned long Process(VOID* Param);
	void ProcessItem(ContentItemNew* Item);
	void Pause();
	void Resume();

public:

	static ContentExtractor& getInstance()
	{
		static ContentExtractor singleton;
		return singleton;
	}

	ContentExtractor(void);
	~ContentExtractor(void);

	void AddContentItem(ContentItemNew* Item);

	CRITICAL_SECTION lock;
};