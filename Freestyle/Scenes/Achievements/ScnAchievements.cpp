#include "stdafx.h"

#include "ScnAchievements.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Settings/Settings.h"
#include "../FileBrowser/ScnFileBrowser.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Managers/Achievements/AchievementManager.h"

using namespace std;


HRESULT CScnAchievements::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	DebugMsg("ScnAchievements","CScnAchievements::OnInit");


	m_listPack = GameContentManager::getInstance().getGameListSnapshot(false);

	HRESULT HR = GetChildById( L"Achievements", &m_Achievements);
	isNested = HR == S_OK;

	if (isNested)
	{
		m_Achievements.GetChildById( L"TextAchievementGS", &m_TextAchievementGS );
		m_Achievements.GetChildById( L"TextAchievementDescription", &m_TextAchievementDescription );
		m_Achievements.GetChildById( L"TextAchievementTitle", &m_TextAchievementTitle );
		m_Achievements.GetChildById( L"TextGameTitle", &m_TextGameTitle );
		m_Achievements.GetChildById( L"TextAchievementCount", &m_TextAchievementCount );
		m_Achievements.GetChildById( L"AchievementList", &m_AchievementList );
		m_Achievements.GetChildById( L"TextAchievementDate", &m_TextAchievementDate );
		m_Achievements.GetChildById( L"TextAchievementTime", &m_TextAchievementTime );
		m_Achievements.GetChildById( L"TextAchievementConnectionType", &m_TextAchievementConnection );
		m_Achievements.GetChildById( L"TextGameNotPlayed", &m_TextGameNotPlayed );

	} else {
		GetChildById( L"TextAchievementGS", &m_TextAchievementGS );
		GetChildById( L"TextAchievementDescription", &m_TextAchievementDescription );
		GetChildById( L"TextAchievementTitle", &m_TextAchievementTitle );
		GetChildById( L"TextGameTitle", &m_TextGameTitle );
		GetChildById( L"TextAchievementCount", &m_TextAchievementCount );
		GetChildById( L"AchievementList", &m_AchievementList );
		GetChildById( L"TextAchievementDate", &m_TextAchievementDate );
		GetChildById( L"TextAchievementTime", &m_TextAchievementTime );
		GetChildById( L"TextAchievementConnectionType", &m_TextAchievementConnection );
		GetChildById( L"TextGameNotPlayed", &m_TextGameNotPlayed );
	}
	
	GetChildById( L"Back", &m_BackButton );

	if(m_listPack.CurrentGame == NULL)
		return S_OK;

	secretChievoText = AchievementManager::getInstance().GetAchievementSetting("SECRETACHIEVEMENTTEXT");
	AchievementManager::getInstance().GetListInfo(&m_pAchievementList);

	m_TextGameTitle.SetText(m_listPack.CurrentGame->Content.m_szTitle);
	m_TextAchievementCount.SetText(sprintfaW(L"%d of %d", m_pAchievementList.achievementsEarned, m_pAchievementList.achievementCount).c_str());

	ClearAchievementInfo();

	if(m_pAchievementList.achievementCount > 0)
	{
		m_TextGameNotPlayed.SetShow(false);
		SetFirstAchievementInfo();
	}

	return S_OK;
}

HRESULT CScnAchievements::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled)
{
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEBACKGROUND && !pGetSourceImageData->bItemData)
	{
		if(m_listPack.CurrentGame != NULL && m_listPack.CurrentGame->Textures.m_BkgTexture != NULL)
		{
			if(m_listPack.CurrentGame->Textures.m_BkgTexture->GetCurrentBrushState() == FSDTEX_BRUSHLOADED)
			{
				pGetSourceImageData->hBrush = m_listPack.CurrentGame->Textures.m_BkgTexture->GetTextureBrush();
				bHandled = true;
			}
		}
	}

	return S_OK;
}

HRESULT CScnAchievements::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged * pNotifySel, BOOL& bHandled )
{
	if(hObjSource == m_AchievementList)
	{
		ClearAchievementInfo();

		m_TextAchievementTitle.SetText(m_pAchievementList.pDetails[pNotifySel->iItem].pwszLabel);

		if(AchievementEarned(m_pAchievementList.pDetails[pNotifySel->iItem].dwFlags))
		{
			m_TextAchievementDescription.SetText(m_pAchievementList.pDetails[pNotifySel->iItem].pwszDescription);
			SetAchievedInfo(pNotifySel->iItem, false);
		}
		else if(AchievementEarnedOnline(m_pAchievementList.pDetails[pNotifySel->iItem].dwFlags))
		{
			m_TextAchievementDescription.SetText(m_pAchievementList.pDetails[pNotifySel->iItem].pwszDescription);
			SetAchievedInfo(pNotifySel->iItem, true);
		}
		else
		{
			if(m_pAchievementList.pDetails[pNotifySel->iItem].pwszUnachieved[0] != NULL)
			{
				m_TextAchievementDescription.SetText(m_pAchievementList.pDetails[pNotifySel->iItem].pwszUnachieved);
			}
			else
			{
				m_TextAchievementDescription.SetText(strtowstr(secretChievoText).c_str());
			}
		}

		wstring gamerScore = sprintfaW(L"%d",m_pAchievementList.pDetails[pNotifySel->iItem].dwCred);

		m_TextAchievementGS.SetText(gamerScore.c_str());

	}

	return S_OK;
}



HRESULT CScnAchievements::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (hObjPressed == m_BackButton)
	{
		NavigateBack(XUSER_INDEX_ANY);
		bHandled = TRUE;
	}

	return S_OK;
}

void CScnAchievements::SetFirstAchievementInfo()
{
	m_TextAchievementTitle.SetText(m_pAchievementList.pDetails[0].pwszLabel);

	if(AchievementEarned(m_pAchievementList.pDetails[0].dwFlags))
	{
		m_TextAchievementDescription.SetText(m_pAchievementList.pDetails[0].pwszDescription);
		SetAchievedInfo(0, false);
	}
	else if(AchievementEarnedOnline(m_pAchievementList.pDetails[0].dwFlags))
	{
		m_TextAchievementDescription.SetText(m_pAchievementList.pDetails[0].pwszDescription);
		SetAchievedInfo(0, true);
	}
	else
	{
		if(!m_pAchievementList.pDetails[0].pwszUnachieved == NULL)
		{
			m_TextAchievementDescription.SetText(m_pAchievementList.pDetails[0].pwszUnachieved);
		}
		else
		{
			m_TextAchievementDescription.SetText(strtowstr(secretChievoText).c_str());
		}
	}

	wstring gamerScore = sprintfaW(L"%d",m_pAchievementList.pDetails[0].dwCred);

	m_TextAchievementGS.SetText(gamerScore.c_str());
}

HRESULT CScnAchievements::SetAchievedInfo(int chievoID, bool online)
{
	SYSTEMTIME LocalSysTime;

	FileTimeToSystemTime(&m_pAchievementList.pDetails[chievoID].ftAchieved, &LocalSysTime);

	string achievedTime = GetTime(1, LocalSysTime);
	string achievedDate = GetDate(2, LocalSysTime);

	m_TextAchievementDate.SetText(strtowstr(achievedDate).c_str());
	m_TextAchievementTime.SetText(strtowstr(achievedTime).c_str());

	if(online)
		m_TextAchievementConnection.SetText(L"Online");
	else
		m_TextAchievementConnection.SetText(L"Offline");


	return S_OK;
}

HRESULT CScnAchievements::ClearAchievementInfo()
{
	m_TextAchievementDate.SetText(L"");
	m_TextAchievementTime.SetText(L"");
	m_TextAchievementConnection.SetText(L"");
	m_TextAchievementGS.SetText(L"");
	m_TextAchievementDescription.SetText(L"");
	m_TextAchievementTitle.SetText(L"");

	return S_OK;
}