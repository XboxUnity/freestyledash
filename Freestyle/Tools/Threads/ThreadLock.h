#pragma once

class ThreadLock
{
public:
	CRITICAL_SECTION lock;
	int LockCount;
	DWORD LockThreadId;

public:
	ThreadLock();
	~ThreadLock();
	void Lock();
	void Unlock();
};
