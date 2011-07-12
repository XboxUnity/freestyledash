#include "stdafx.h"

#include "../../Texture/TextureItem/TextureItem.h"
#include "../../Texture/TextureCache.h"
#include "../../Threads/CThread.h"
#include "../../Debug/Debug.h"
#include "FileWorker.h"

void FileWorker::SetItem(TextureItem* newItem)
{
	while(IsBusy())
	{
		Sleep(10);
	}
	m_WaitingToStart = true;
	m_CurrentItem = newItem;
//	printf("FileWorker %08x working on %08x\n", this, newItem);
}

void FileWorker::LoadCurrentItem()
{
	m_CurrentItem->AddRef();
	//DebugMsg("FileWorker", "Loading File Data:  %s", m_CurrentItem->GetTextureFilePath().c_str());
	//ATG::Timer m_Timer;
	//double timeA = m_Timer.GetAbsoluteTime();
	TextureCache myCache;
	m_CurrentItem->SetCurrentBrushState(FSDTEX_LOADING);
	HRESULT hr = NULL;
	BYTE * fileData;
	DWORD fileSize;

	if(m_CurrentItem->IsXZPTexture())
		hr = myCache.LoadXZPResourceToMemory(m_CurrentItem->GetTextureFilePath(), &fileData, &fileSize);
	else
		hr = myCache.LoadFileToMemory(m_CurrentItem->GetTextureFilePath(), &fileData, &fileSize);

	if(hr == S_OK)
	{
		m_CurrentItem->SetFilePointer(fileData, fileSize);
		//double timeB = m_Timer.GetAbsoluteTime();
		//DebugMsg("FileWorker", "File Data Loaded:  %s (%X,%X) - %4.2fms", m_CurrentItem->GetTextureFilePath().c_str(), fileData, fileSize, (timeB-timeA)*1000.0f);
	}
	else
	{
		m_CurrentItem->SetFilePointer(NULL, 0);
		m_CurrentItem->SetCurrentFileState(FSDTEX_FILEERROR);
		m_CurrentItem->SetCurrentBrushState(FSDTEX_BRUSHERROR);
		//DebugMsg("FileWorker", "Loading File Data Failed:  %s", m_CurrentItem->GetTextureFilePath().c_str());
	}

	m_CurrentItem->Release();

}
unsigned long FileWorker::Process(void* parameter)
{
	SetThreadName("FileWorker");
	while(!m_IsDisposed)
	{
		if(m_CurrentItem != NULL)
		{
			if(m_CurrentItem->GetTextureItemState() == FSDTEX_INITIALIZED)
			{
				m_IsBusy = true;
				m_WaitingToStart = false;
				//m_CurrentItem->SetTextureItemState(FSDTEX_LOADING);
				LoadCurrentItem();
				m_CurrentItem = NULL;
				m_WaitingToStart = false;
				m_IsBusy = false;
			}
		}
		Sleep(50);
	}
	return 0;
}