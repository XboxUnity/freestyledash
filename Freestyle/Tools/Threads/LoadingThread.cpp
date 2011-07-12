#include "stdafx.h"
#include "LoadingThread.h"
#include "../ContentList/ContentManager.h"
#include "../ContentList/ContentKaiVector.h"

LoadingThread::LoadingThread(void)
{

}

LoadingThread::~LoadingThread(void)
{
}

unsigned long LoadingThread::Process(void* parameter)
{
	//CreateThread(CPU2_THREAD_2);
	DebugMsg("LoadingThread","Thread Started");
	ContentKaiVector::getInstance().Initialize();
	ContentManager::getInstance().Initialize();
	//ContentList::getInstance().Initialize();
	DebugMsg("LoadingThread","Loaded ContentList");

	DebugMsg("LoadingThread","Thread Finished");
	return 0;
}
