#pragma once
#include "../../Threads/XeThread.h"
#include "../../Managers/Skin/SkinXMLReader/SkinXMLReader.h"
#include "../../GameContent/GameContentManager.h"

typedef struct 
{
	int ListSize;
	int CurrentIndex;
	int achievementCount;
	int achievementsEarned;
	HXUIBRUSH CurrentBrush;
	PXACHIEVEMENT_DETAILS pDetails;

} AchievementInformation;

class AchievementManager : public SkinXMLReader
{
public:
	static AchievementManager& getInstance()
	{
		static AchievementManager singleton;
		return singleton;
	}

	UINT GetAchievementCount(wstring szGameID);
	HRESULT CreateAchievementList(wstring szGameID);
	HXUIBRUSH GetTextureBrush(int nSSIndex);
	HRESULT SetListInfo(AchievementInformation *m_pAchievementInfo);
	HRESULT GetListInfo(AchievementInformation *m_pAchievementInfo);
	HRESULT Clear();
	string GetAchievementSetting(string settingName);

private:
	// Vector used to store our texture list
	vector<TextureItem *> m_vChievoTextures;
	// Vector used to store the file paths 
	vector<string> m_vChievoPaths;
	vector<TextureItem *> m_vDisplayList;

	IDirect3DTexture9 * pTexture;
	AchievementInformation m_AchievementList;

	HRESULT GetAchievementEarnedCount();

	AchievementManager()
	{
		m_AchievementList.CurrentBrush = NULL;
		m_AchievementList.CurrentIndex = 0;
		m_AchievementList.ListSize = 0;
		m_AchievementList.pDetails = NULL;
		m_AchievementList.achievementCount = 0;
		m_AchievementList.achievementsEarned = 0;
	}
	~AchievementManager(){}
	
	AchievementManager(const AchievementManager&);		            // Prevent copy-construction
	AchievementManager& operator=(const AchievementManager&);		    // Prevent assignment

};