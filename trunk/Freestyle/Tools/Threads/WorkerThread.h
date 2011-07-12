#pragma once

#include "../Generic/tools.h"
#include "../Threads/cthread.h"


class WorkerThread : public CThread
{
public:

	WorkerThread() {}
	~WorkerThread( void ) {}
	bool CreateWorkerThread( void );

protected:
	virtual unsigned long Process (void* parameter);	
};

