#include "stdafx.h"
#include "../../GameContent/QueueThreads/FileTexQueue.h"
#include "ScreenshotManager.h"

HRESULT ScreenshotManager::SetListInfo(ScreenshotInformation *m_pScreenshotInfo)
{
	m_ScreenshotList.CurrentBrush = m_pScreenshotInfo->CurrentBrush;
	m_ScreenshotList.CurrentIndex = m_pScreenshotInfo->CurrentIndex;
	m_ScreenshotList.ListSize = m_pScreenshotInfo->ListSize;
	m_ScreenshotList.maxVisible = m_pScreenshotInfo->maxVisible;

	return S_OK;
}

HRESULT ScreenshotManager::GetListInfo(ScreenshotInformation *m_pScreenshotInfo)
{
	*m_pScreenshotInfo = m_ScreenshotList;

	return S_OK;
}

UINT ScreenshotManager::GetScreenshotCount(GameListItem * pGameContent)
{
	// If the pGameContent or its contentitem reference is null, return size 0
	if(pGameContent == NULL || pGameContent->ContentRef == NULL)
		return 0;

	// Grab the count of the screenshots
	DWORD dwCount = pGameContent->ContentRef->getScreenshotCount();

	// Clear the ScreenshotPaths vector
	m_vSSPaths.clear();

	// Add all the paths to the Screenshot Paths vector
	for(int nCount = 0; nCount < (int)dwCount; nCount++) {
		string szScreenshotPath = pGameContent->ContentRef->getScreenshotPath(nCount);
		m_vSSPaths.push_back(szScreenshotPath);
	}
	
	return (int)dwCount;
}

HRESULT ScreenshotManager::CreateScreenshotList(GameListItem * pGameContent)
{
	int nPathCount = GetScreenshotCount(pGameContent);
	
	if(nPathCount < 1)
		return S_FALSE;

	for(int nIndex = 0; nIndex < nPathCount; nIndex++)
	{
		TextureItem * texture = new TextureItem();

		texture->SetTextureInfo(m_vSSPaths.at(nIndex), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2);
		texture->AddRef();
		FileTexQueue::getInstance().AddToQueue(texture);
		m_vSSTextures.push_back(texture);
	}

	m_ScreenshotList.listLoaded = true;

	return S_OK;
}

HXUIBRUSH ScreenshotManager::GetTextureBrush(int nSSIndex)
{
	if(nSSIndex < (int)m_vSSPaths.size() && nSSIndex >= 0)
	{
		if(m_vSSTextures.at(nSSIndex) != NULL)
			return m_vSSTextures.at(nSSIndex)->GetTextureBrush();
	}

	return NULL;
}

HRESULT ScreenshotManager::Clear()
{
	m_ScreenshotList.CurrentBrush = NULL;
	m_ScreenshotList.CurrentIndex = -1;
	m_ScreenshotList.isRendered = false;
	m_ScreenshotList.listLoaded = false;
	m_ScreenshotList.ListSize = 0;
	m_ScreenshotList.maxVisible = 0;

	vector<TextureItem*>::iterator itr;
	for(itr = m_vSSTextures.begin(); itr != m_vSSTextures.end(); itr++)
	{
		delete *itr;
		*itr = NULL;
	}
	m_vSSTextures.clear();

	return S_OK;
}

HRESULT MoveLeft(int min, int max)
{
	
	return S_OK;
}

HRESULT MoveRight(int min, int max)
{

	return S_OK;
}