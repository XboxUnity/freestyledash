#include "stdafx.h"
#include "../Threads/cthread.h"
#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"
//end project dependencies


//*********************************************************
//default constructor for class CThread
//*********************************************************
CThread::CThread ()
{
	//GenSRand();

	name = "Unnamed";
	this->hThread		= NULL;
	this->hThreadId		= 0;
	this->hMainThread	= GetCurrentThread ();
	this->hMainThreadId = GetCurrentThreadId ();
	this->Timeout		= 0; //milliseconds
	terminate = 0;
	this->Timeout		= 500; //milliseconds

	//hEvent = CreateEvent(NULL,FALSE,FALSE,"");
}

//*********************************************************
//destructor for class CObject
//*********************************************************
CThread::~CThread ()
{
	//waiting for the thread to terminate
	Terminate();

//	CloseHandle(hEvent);
	//DebugMsg("Thread Finnish : %s, 0x%X",name.c_str(),(int)hThreadId);
}

//*********************************************************
//working method
//*********************************************************
unsigned long CThread::Process (void* parameter)
{
	printf("CThread::Process\n");
	//a mechanism for terminating thread should be implemented
	//not allowing the method to be run from the main thread
	if (::GetCurrentThreadId () == this->hMainThreadId)
		return 0;
	else {

		return 0;
	}

}

//*********************************************************
//creates the thread
//*********************************************************
bool CThread::CreateThread (DWORD hdwID)
{
	if (!this->IsCreated ()) {
		
		param*	this_param = new param;
		this_param->pThread	= this;
		this->hThread = ::CreateThread (NULL, 0, (unsigned long (__stdcall *)(void *))this->runProcess, (void *)(this_param), CREATE_SUSPENDED, &this->hThreadId);
		XSetThreadProcessor(this->hThread, hdwID);
		::ResumeThread(this->hThread);

		Sleep(0);
		return this->hThread ? true : false;
	}
	return false;

}

//*********************************************************
//creates the thread
//*********************************************************
int CThread::runProcess (void* Param)
{
	CThread*	thread;
	thread			= (CThread*)((param*)Param)->pThread;
	delete	((param*)Param);
	return thread->Process (0);
}

void CThread::Lock()
{
	lock.Lock();	
}

void CThread::Unlock()
{
	lock.Unlock();
}

void CThread::TriggerEvent()
{
	
	
//		SetEvent(hEvent);


}
/*
void CThread::WaitForEvent(DWORD wait)
{
	WaitForSingleObject(hEvent,wait);
}*/

void CThread::Terminate()
{
	if (this->hThread) {
		terminate = 1;
		TriggerEvent();

		if (::WaitForSingleObject (this->hThread, this->Timeout) == WAIT_TIMEOUT)
		{
			DebugMsg("CThread","CThread: Forcing termination of thread %s 0x%X",name.c_str(),(int)hThreadId);
			//TerminateThread (this->hThread, 99);
		}

		CloseHandle (this->hThread);

		this->hThread = 0;
	}
}

void CThread::SetThreadName( char* threadName)
{
   Sleep(10);
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   DebugMsg("CThread", "This thread is named [%s]", threadName);
   info.dwThreadID = (DWORD)-1;
   info.dwFlags = 0;

   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}
