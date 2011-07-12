#pragma once

#include "threadlock.h"

#include "../Generic/tools.h"

using namespace std;

#define MS_VC_EXCEPTION 0x406D1388

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

enum HardwareThread 
{
    CPU1_THREAD_1,
    CPU1_THREAD_2,
    CPU2_THREAD_1,
    CPU2_THREAD_2,
    CPU3_THREAD_1,
    CPU3_THREAD_2
};

class CThread {

public:
	//constructors
	CThread ();
	ThreadLock lock;

	//destructor
	virtual ~CThread ();

	bool CreateThread (DWORD hdwID);

	int	IsCreated ()
	{	return (this->hThread != NULL);	}

	DWORD		Timeout;

	HANDLE	GetThreadHandle ()
	{	return this->hThread;	}
	DWORD	GetThreadId ()
	{	return this->hThreadId;	}
	HANDLE	GetMainThreadHandle ()
	{	return this->hMainThread;	}
	DWORD	GetMainThreadId ()
	{	return this->hMainThreadId;	}

	void SetThreadName( char* threadName);

	string name;
	void Lock();
	void Unlock();
	int terminate;
	void Terminate();
	void TriggerEvent();

protected:

	//overrideable
	virtual unsigned long Process (void* parameter);	
	//void WaitForEvent(DWORD wait);

public:
	DWORD		hThreadId;
	HANDLE		hThread;
	DWORD		hMainThreadId;
	HANDLE		hMainThread;

	
	
private:

	//HANDLE hEvent;
	static int runProcess (void* Param);
};

struct param {
	CThread*	pThread;
};
