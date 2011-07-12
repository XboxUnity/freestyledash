#include "stdafx.h"
#include "ThreadLock.h"

ThreadLock::ThreadLock()
{
	InitializeCriticalSection(&lock);
	LockCount = 0;
	LockThreadId = 0;
}

ThreadLock::~ThreadLock()
{
	DeleteCriticalSection(&lock);
}

void ThreadLock::Lock()
{
//	if (LockCount == 0)
//	{
		EnterCriticalSection(&lock);
//		LockThreadId = GetCurrentThreadId();
//	}
//	else // already locked, are we in same thread???
//	{
//		DWORD CurrentThreadID = GetCurrentThreadId();
//		if (LockThreadId > 0 && CurrentThreadID != LockThreadId)
//		{
//			// basically, we need to do an enter crit sect to lock the current thread until the previous one unlocks
//			//_ASSERT(false);
//			EnterCriticalSection(&lock);
//			LockThreadId = GetCurrentThreadId();
//		}
//	}

//	LockCount++;
}


void ThreadLock::Unlock()
{
//	LockCount--;

//	if (LockCount == 0)
//	{
		LeaveCriticalSection(&lock);
//		LockThreadId = 0;
//	}
}

