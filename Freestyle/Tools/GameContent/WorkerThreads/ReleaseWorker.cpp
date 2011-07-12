#include "stdafx.h"

#include "../../Texture/TextureItem/TextureItem.h"
#include "../../Texture/TextureCache.h"
#include "../../Threads/CThread.h"
#include "../../Debug/Debug.h"
#include "ReleaseWorker.h"

void ReleaseWorker::SetItem(TextureItem* newItem, bool releaseFile, bool releaseTexture, bool releaseBrush)
{
	while(IsBusy())
	{
		Sleep(10);
	}
	m_WaitingToStart = true;
	m_CurrentItem = newItem;
	m_releaseFile = releaseFile;
	m_releaseTex = releaseTexture;
	m_releaseBrush = releaseBrush;
}

void ReleaseWorker::LoadCurrentItem()
{
	//ATG::Timer m_Timer;
	//double timeA = m_Timer.GetAbsoluteTime();
	m_CurrentItem->ReleaseAll();
/*	if(m_releaseBrush){
		m_CurrentItem->ReleaseBrush();
	}

	if(m_releaseTex){
		m_CurrentItem->ReleaseTexture();
	}

	if(m_releaseFile){
		m_CurrentItem->ReleaseFilePointer();
	}
*/
	//double timeB = m_Timer.GetAbsoluteTime();
	//DebugMsg("ReleaseWorker", "TextureItem Released:  %s (%d,%d,%d) - %4.2fms", m_CurrentItem->GetTextureFilePath().c_str(), m_releaseFile, m_releaseTex, m_releaseBrush, (timeB-timeA)*1000.0f);

}
unsigned long ReleaseWorker::Process(void* parameter)
{
	SetThreadName("ReleaseWorker");
	//SetThreadPriority(this->hThread, THREAD_PRIORITY_LOWEST);
	while(!m_IsDisposed)
	{
		if(m_CurrentItem != NULL)
		{
			if(m_CurrentItem->GetRefCount() <= 0)
			{
				//DebugMsg("ReleaseQueue", "Releasing:  %s (Ref Count: %d)", m_CurrentItem->GetTextureFilePath().c_str(), m_CurrentItem->GetRefCount());
				m_IsBusy = true;
				m_WaitingToStart = false;
				LoadCurrentItem();
				m_CurrentItem = NULL;
				m_WaitingToStart = false;
				m_IsBusy = false;
				m_releaseFile = false;
				m_releaseTex = false;
				m_releaseBrush = false;			
			}
		}
		Sleep(50);
	}
	return 0;
}