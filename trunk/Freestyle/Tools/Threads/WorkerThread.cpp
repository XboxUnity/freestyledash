#include "stdafx.h"
#include "WorkerThread.h"

bool WorkerThread::CreateWorkerThread( void )
{
	bool bResult = CreateThread( CPU3_THREAD_1 );
	SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);

	return bResult;
}

unsigned long WorkerThread::Process (void* parameter)
{
	//a mechanism for terminating thread should be implemented
	//not allowing the method to be run from the main thread
	if (::GetCurrentThreadId () == this->hMainThreadId)
		return 0;
	else {

		return 0;
	}

}