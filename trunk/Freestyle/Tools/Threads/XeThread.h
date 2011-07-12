#pragma once
#include "../Debug/Debug.h"
#include "../Generic/xboxtools.h"

class XeThread
{
public:
	XeThread(void);
	~XeThread(void);

	void AttachFunc(string szFunctionName, LPTHREAD_START_ROUTINE lpThreadFunc );

	virtual HRESULT Stop();
	virtual HRESULT Run();
	virtual DWORD getExitCode();

	static DWORD WINAPI Sample(void * Obj);
	bool IsSuspended;
	bool bTerminate;
	bool IsStarted;

protected:
	string m_sFuncName;
private:
	LPTHREAD_START_ROUTINE m_pThreadFunc;
	HANDLE m_hThread;
	DWORD m_dThreadId;
};