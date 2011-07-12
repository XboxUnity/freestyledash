#include "stdafx.h"
#include "../../GameContent/QueueThreads/FileTexQueue.h"
#include "../../../Application/FreestyleApp.h"
#include "AchievementManager.h"

HRESULT AchievementManager::SetListInfo(AchievementInformation *m_pAchievementInfo)
{
	m_AchievementList.CurrentBrush = m_pAchievementInfo->CurrentBrush;
	m_AchievementList.CurrentIndex = m_pAchievementInfo->CurrentIndex;
	m_AchievementList.ListSize = m_pAchievementInfo->ListSize;
	m_AchievementList.pDetails = m_pAchievementInfo->pDetails;
	m_AchievementList.achievementCount = m_pAchievementInfo->achievementCount;
	m_AchievementList.achievementsEarned = m_pAchievementInfo->achievementsEarned;

	return S_OK;
}

HRESULT AchievementManager::GetListInfo(AchievementInformation *m_pAchievementInfo)
{
	*m_pAchievementInfo = m_AchievementList;

	return S_OK;
}

HRESULT AchievementManager::GetAchievementEarnedCount()
{
	for(int i=0; i< m_AchievementList.achievementCount; i++)
	{
		if(AchievementEarned(m_AchievementList.pDetails[i].dwFlags) || AchievementEarnedOnline(m_AchievementList.pDetails[i].dwFlags))
			m_AchievementList.achievementsEarned++;
	}

	return S_OK;
}

UINT AchievementManager::GetAchievementCount(wstring szGameID)
{
	DWORD GameID = strtoul(wstrtostr(szGameID).c_str(),NULL,16);
	DWORD bufferSize = 0;
	HANDLE aeHandle = NULL;
	DWORD result = XamUserCreateAchievementEnumerator(
		GameID,
		0,
		INVALID_XUID,
		XACHIEVEMENT_DETAILS_ALL,
		0,
		150,
		&bufferSize,
		&aeHandle
	);

	if(aeHandle == INVALID_HANDLE_VALUE)
		return 0;
	
	DWORD returnItems = 0;
	m_AchievementList.pDetails = (PXACHIEVEMENT_DETAILS)malloc(bufferSize);
	result = XEnumerate(
		 aeHandle,
		 m_AchievementList.pDetails,
		 bufferSize,
		 &returnItems,
		 NULL
	);

	m_AchievementList.achievementCount = returnItems;
	m_AchievementList.ListSize = returnItems;
	GetAchievementEarnedCount();

	return returnItems;
}

HRESULT AchievementManager::CreateAchievementList(wstring szGameID)
{
	if(m_AchievementList.achievementCount < 1)
		return S_FALSE;

	DWORD GameID = strtoul(wstrtostr(szGameID).c_str(),NULL,16);

	IDirect3DDevice9 * pDevice = CFreestyleApp::getInstance().m_pd3dDevice;

	for(int nIndex = 0; nIndex < m_AchievementList.achievementCount; nIndex++)
	{
		TextureItem * texture = new TextureItem();

		IDirect3DTexture9 * pTexture = NULL;

		HRESULT hr = pDevice->CreateTexture( 64, 64, 1, 0, D3DFMT_LIN_A8R8G8B8, 0, &pTexture, NULL);

		HXUIBRUSH hBrush;

		if(hr == D3D_OK)
		{
			texture->SetD3DTexture(pTexture);
			XuiAttachTextureBrush(pTexture, &hBrush);
 			texture->SetTextureBrush(hBrush);
			texture->SetCurrentBrushState(FSDTEX_BRUSHERROR);
		}
		else{
			texture->SetTextureBrush(NULL);
			texture->SetCurrentBrushState(FSDTEX_BRUSHERROR);
		}

		m_vChievoTextures.push_back(texture);
	}

	D3DLOCKED_RECT rect;
	D3DSURFACE_DESC desc;
	DWORD ret;

	for(int nIndex = 0; nIndex < m_AchievementList.achievementCount; nIndex++)
	{
		m_vChievoTextures.at(nIndex)->GetD3DTexture()->GetLevelDesc( 0, &desc );
		m_vChievoTextures.at(nIndex)->GetD3DTexture()->LockRect(0, &rect, NULL, 0);

		ret = XamReadTileToTexture(0, GameID, m_AchievementList.pDetails[nIndex].dwImageId, 0, (BYTE*)rect.pBits, rect.Pitch, desc.Height, NULL);
		
		if(ret == 0)
		{
			m_vChievoTextures.at(nIndex)->SetCurrentBrushState(FSDTEX_BRUSHLOADED);

		}
		else
		{
			m_vChievoTextures.at(nIndex)->SetTextureBrush(NULL);
			m_vChievoTextures.at(nIndex)->SetCurrentBrushState(FSDTEX_BRUSHERROR);
		}

		m_vChievoTextures.at(nIndex)->GetD3DTexture()->UnlockRect( 0 );
	}

	return S_OK;
}

HXUIBRUSH AchievementManager::GetTextureBrush(int nChievoIndex)
{
	if(nChievoIndex < (int)m_vChievoTextures.size() && nChievoIndex >= 0)
	{
		if(m_vChievoTextures.at(nChievoIndex) != NULL)
			if(m_vChievoTextures.at(nChievoIndex)->GetCurrentBrushState() == FSDTEX_BRUSHLOADED)
				return m_vChievoTextures.at(nChievoIndex)->GetTextureBrush();
	}

	return NULL;
}

HRESULT AchievementManager::Clear()
{
	m_AchievementList.CurrentBrush = NULL;
	m_AchievementList.CurrentIndex = -1;
	m_AchievementList.ListSize = 0;
	m_AchievementList.achievementCount = 0;
	m_AchievementList.achievementsEarned = 0;

	free(m_AchievementList.pDetails);
	m_AchievementList.pDetails = NULL;

	vector<TextureItem*>::iterator itr;
	for(itr = m_vChievoTextures.begin(); itr != m_vChievoTextures.end(); itr++)
	{
		delete *itr;
		*itr = NULL;
	}
	m_vChievoTextures.clear();

	return S_OK;
}

string AchievementManager::GetAchievementSetting(string settingName)
{
	LoadSettings("AchievementManager", "AchievementManager");

	return GetSetting(settingName, "");
}