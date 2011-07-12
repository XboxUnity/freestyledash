#pragma once
#include "../Threads/cthread.h"

class LoadingThread :
	public CThread
{
public:
	LoadingThread(void);
	~LoadingThread(void);

	unsigned long Process (void* parameter);
};
