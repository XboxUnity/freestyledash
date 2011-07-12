#include "stdafx.h"
#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"

#include "XeThread.h"

XeThread::XeThread()
{
	bTerminate = false;
	IsSuspended = false;
	AttachFunc("Sample", XeThread::Sample);
}

XeThread::~XeThread()
{
	DebugMsg("XeThread", "Killing %s [%X]", m_sFuncName.c_str(), m_dThreadId);
	Stop();	
}

void XeThread::AttachFunc( string szFunctionName, LPTHREAD_START_ROUTINE lpThreadFunc )
{
	m_sFuncName = szFunctionName;
	m_pThreadFunc = lpThreadFunc;
};

HRESULT XeThread::Stop()
{
	if(m_hThread)
	{
		CloseHandle(m_hThread);
		IsStarted = false;
		m_hThread = 0;
	}
	return S_OK;
}

HRESULT XeThread::Run()
{
	m_hThread = CreateThread( 
        NULL,                   // default security attributes
        0,                      // use default stack size  
		m_pThreadFunc,			// thread function name
        this,					// argument to thread function 
        CREATE_SUSPENDED,                      // use default creation flags 
        &m_dThreadId			// returns the thread identifier 
	);   
	bTerminate = false;
	IsStarted = true;

	DebugMsg("XeThread", "Starting: %s [%X]", m_sFuncName.c_str(), m_dThreadId);

	XSetThreadProcessor(m_hThread, 5);
	SetThreadPriority(m_hThread,THREAD_PRIORITY_LOWEST);
	ResumeThread(m_hThread);
	CloseHandle(m_hThread);
	return S_OK;
}

DWORD XeThread::getExitCode()
{
	DWORD ExitCode;
	GetExitCodeThread(m_hThread,&ExitCode);
	return ExitCode;
}

/** Sample Thread **/
DWORD WINAPI XeThread::Sample(void * Obj)
{
	XeThread * _this = NULL;
	_this = (XeThread*)Obj;

	while(!_this->bTerminate)
	{
		if(!_this->IsSuspended)
		{
			DebugMsg("XeThread", "Sample");
			Sleep(500);
		}
		else
		{
			Sleep(100);
		}
	}

	return 0;
}