#include "stdafx.h"

#include "../../Texture/TextureItem/TextureItem.h"
#include "../../Texture/TextureCache.h"
#include "../../Threads/CThread.h"
#include "../../Debug/Debug.h"
#include "FileTexWorker.h"

FileTexWorker::FileTexWorker()
{
	m_WaitingToStart = false;
	m_CurrentItem = NULL;
	m_IsBusy = false;
	m_IsDisposed = false;
	CreateThread(CPU3_THREAD_2);
};


void FileTexWorker::SetItem(TextureItem* newItem)
{
	while(IsBusy())
	{
		Sleep(10);
	}
	m_WaitingToStart = true;
	m_CurrentItem = newItem;
//	printf("FileTexWorker %08x working on %08x\n", this, newItem);
}

void FileTexWorker::LoadCurrentItem()
{
	// Signify that the current item is in use
	m_CurrentItem->AddRef();

	TextureCache myCache;
	m_CurrentItem->SetCurrentBrushState(FSDTEX_LOADING);
	
	HRESULT hr = NULL;
	BYTE * fileData;
	DWORD fileSize;

	//DebugMsg("FileWorker", "Loading File Data:  %s", m_CurrentItem->GetTextureFilePath().c_str());
	if(m_CurrentItem->IsXZPTexture())
		hr = myCache.LoadXZPResourceToMemory(m_CurrentItem->GetTextureFilePath(), &fileData, &fileSize);
	else
		hr = myCache.LoadFileToMemory(m_CurrentItem->GetTextureFilePath(), &fileData, &fileSize);

	if(hr == S_OK)
	{
		m_CurrentItem->SetFilePointer(fileData, fileSize);
		//m_CurrentItem->SetCurrentFileState(FSDTEX_FILELOADED);
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

	UINT nTextureWidth;
	UINT nTextureHeight;
	IDirect3DTexture9 * pTexture;
	HXUIBRUSH hTextureBrush;

	nTextureWidth = m_CurrentItem->GetTextureWidth();
	nTextureHeight = m_CurrentItem->GetTextureHeight();
	//m_CurrentItem->GetFilePointer(&fileData, &fileSize);

	//timeA = m_Timer.GetAbsoluteTime();
	if(m_CurrentItem->GetCurrentFileState() == FSDTEX_FILELOADED)
	{
		hr = myCache.LoadD3DTextureFromFileInMemory(fileData, fileSize, nTextureWidth, nTextureHeight, &pTexture, true);
		if(hr == S_OK)
		{
			m_CurrentItem->SetD3DTexture(pTexture);
			//m_CurrentItem->SetCurrentTextureState(FSDTEX_TEXLOADED);
		}
		else
		{
			//m_CurrentItem->ReleaseFilePointer();
			m_CurrentItem->SetCurrentFileState(FSDTEX_TEXERROR);
			//DebugMsg("TextureWorker", "Creating Texture Failed:  %s", m_CurrentItem->GetTextureFilePath().c_str());
		}

		if(m_CurrentItem->GetCurrentTextureState() == FSDTEX_TEXLOADED)
		{
			pTexture = m_CurrentItem->GetD3DTexture();
			hr = XuiAttachTextureBrush(pTexture, &hTextureBrush);
			if(hr == S_OK)
			{
				m_CurrentItem->SetTextureBrush(hTextureBrush);
				//m_CurrentItem->SetCurrentBrushState(FSDTEX_BRUSHLOADED);
				//ReleaseQueue::getInstance().AddToQueue(m_CurrentItem, true, false, false);
				
				//timeB = m_Timer.GetAbsoluteTime();
				//DebugMsg("TextureWorker", "Texture/Brush Created:  %s (%X) - %4.2fms", m_CurrentItem->GetTextureFilePath().c_str(), pTexture, (timeB-timeA)*1000.0f);
			}
			else
			{
				//m_CurrentItem->ReleaseTexture();
				//m_CurrentItem->ReleaseFilePointer();
				m_CurrentItem->SetCurrentFileState(FSDTEX_BRUSHERROR);
				//DebugMsg("FileWorker", "Creating Brush Failed:  %s", m_CurrentItem->GetTextureFilePath().c_str());
			}
		}
	}

	m_CurrentItem->ReleaseFilePointer();
	m_CurrentItem->Release();
}

unsigned long FileTexWorker::Process(void* parameter)
{
	SetThreadName("FileTexWorker");
	//SetThreadPriority(this->hThread, THREAD_PRIORITY_LOWEST);
	while(!m_IsDisposed)
	{
		if(m_CurrentItem != NULL)
		{
			if(m_CurrentItem->GetRefCount() > 0)
			{
				m_IsBusy = true;
				m_WaitingToStart = false;
				//m_CurrentItem->SetTextureItemState(FSDTEX_LOADING);
				LoadCurrentItem();
				//DebugMsg("FileTexWorker", "Processed:  %s - Brush State:  %d", m_CurrentItem->GetTextureFilePath().c_str(), m_CurrentItem->GetCurrentBrushState());
				//m_CurrentItem->SetTextureItemState(FSDTEX_ALLREADY);
				m_CurrentItem = NULL;
				m_WaitingToStart = false;
				m_IsBusy = false;
			}
			else
			{
				m_CurrentItem = NULL;
				m_WaitingToStart = false;
				m_IsBusy = false;
			}
		}
		//DebugMsg("FileTexWorker", "CurrentItem Pointer:  %X", m_CurrentItem);
		Sleep(25);
	}
	return 0;
}