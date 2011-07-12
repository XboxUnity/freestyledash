#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/Generic/Xboxtools.h"
#include "../../Tools/GameContent/GameContentManager.h"
#include "AchievementList/AchievementList.h"

class CScnAchievements : public CXuiTabSceneImpl
{
private :

	string secretChievoText;
public:

	CXuiScene m_Achievements;
	bool isNested;
	GAMECONTENT_LIST_STATE m_listPack;
	AchievementInformation m_pAchievementList;

	CXuiList m_AchievementList;
	CXuiControl m_BackButton;
	CXuiTextElement m_TextAchievementGS, 
					m_TextAchievementDescription, 
					m_TextAchievementTitle, 
					m_TextGameTitle, 
					m_TextAchievementCount,
					m_TextAchievementDate,
					m_TextAchievementTime,
					m_TextAchievementConnection,
					m_TextGameNotPlayed;


	XUI_IMPLEMENT_CLASS( CScnAchievements, L"ScnAchievements", XUI_CLASS_TABSCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
	XUI_END_MSG_MAP()

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged * pNotifySel, BOOL& bHandled );
	
	void SetFirstAchievementInfo();
	HRESULT SetAchievedInfo(int chievoID, bool online);
	HRESULT ClearAchievementInfo();

};
