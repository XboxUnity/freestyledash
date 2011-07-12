#include "stdafx.h"
#include "../../Texture/TextureItem/TextureItem.h"
#include "../../Texture/TextureCache.h"
#include "../../Threads/CThread.h"
#include "../../Debug/Debug.h"
#include "TextureWorker.h"

void TextureWorker::SetItem(TextureItem* newItem)
{
	while(IsBusy())
	{
		Sleep(10);
	}
	m_WaitingToStart = true;
	m_CurrentItem = newItem;
//	printf("TextureWorker %08x working on %08x\n", this, newItem);
}

void TextureWorker::LoadCurrentItem()
{
	m_CurrentItem->AddRef();
	//DebugMsg("TextureWorker", "Creating Texture From File:  %s", m_CurrentItem->GetTextureFilePath().c_str());
	//ATG::Timer m_Timer;
	//double timeA, timeB;
	TextureCache myCache;
	
	HRESULT hr = NULL;
	BYTE * fileData;
	DWORD fileSize;

	UINT nTextureWidth;
	UINT nTextureHeight;
	IDirect3DTexture9 * pTexture;
	HXUIBRUSH hTextureBrush;

	nTextureWidth = m_CurrentItem->GetTextureWidth();
	nTextureHeight = m_CurrentItem->GetTextureHeight();
	m_CurrentItem->SetCurrentBrushState(FSDTEX_LOADING);
	m_CurrentItem->GetFilePointer(&fileData, &fileSize);

	//timeA = m_Timer.GetAbsoluteTime();
	if(m_CurrentItem->GetCurrentFileState() == FSDTEX_FILELOADED)
	{
		hr = myCache.LoadD3DTextureFromFileInMemory(fileData, fileSize, nTextureWidth, nTextureHeight, &pTexture, true);
		if(hr == S_OK)
		{
			// Set the Direct3DTexture
			m_CurrentItem->SetD3DTexture(pTexture);
		}
		else
		{
			m_CurrentItem->SetD3DTexture(NULL);
			m_CurrentItem->SetCurrentFileState(FSDTEX_TEXERROR);
			m_CurrentItem->SetCurrentBrushState(FSDTEX_BRUSHERROR);
			//DebugMsg("TextureWorker", "Creating Texture Failed:  %s", m_CurrentItem->GetTextureFilePath().c_str());
		}
	}

	if(m_CurrentItem->GetCurrentTextureState() == FSDTEX_TEXLOADED)
	{
		pTexture = m_CurrentItem->GetD3DTexture();
		hr = XuiAttachTextureBrush(pTexture, &hTextureBrush);
		if(hr == S_OK)
		{
			m_CurrentItem->SetTextureBrush(hTextureBrush);
			//timeB = m_Timer.GetAbsoluteTime();
			//DebugMsg("TextureWorker", "Texture/Brush Created:  %s (%X) - %4.2fms", m_CurrentItem->GetTextureFilePath().c_str(), pTexture, (timeB-timeA)*1000.0f);
		}
		else
		{
			m_CurrentItem->SetFilePointer(NULL, 0);
			m_CurrentItem->SetCurrentFileState(FSDTEX_BRUSHERROR);
			//DebugMsg("FileWorker", "Creating Brush Failed:  %s", m_CurrentItem->GetTextureFilePath().c_str());
		}
	}

	m_CurrentItem->ReleaseFilePointer();
	m_CurrentItem->Release();

}

unsigned long TextureWorker::Process(void* parameter)
{
	SetThreadName("TextureWorker");
	while(!m_IsDisposed)
	{
		if(m_CurrentItem != NULL)
		{
			if(m_CurrentItem->GetCurrentTextureState() == FSDTEX_TEXREADY && 
				m_CurrentItem->GetCurrentFileState() == FSDTEX_FILELOADED)
			{
				m_IsBusy = true;
				m_WaitingToStart = false;
				//m_CurrentItem->SetTextureItemState(FSDTEX_LOADING);
				LoadCurrentItem();
				m_CurrentItem = NULL;
				m_WaitingToStart = false;
				m_IsBusy = false;
			}
			else if(m_CurrentItem->GetCurrentFileState() == FSDTEX_FILEERROR)
			{
				m_CurrentItem = NULL;
				m_WaitingToStart = false;
				m_IsBusy = false;
			}
		}
		Sleep(50);
	}
	return 0;
}