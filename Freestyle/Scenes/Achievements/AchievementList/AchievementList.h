#pragma once

#include "../../../Tools/Generic/tools.h"
#include "../../Abstracts/ConfigurableList/ConfigurableList.h"
#include "../../../Tools/Settings/Settings.h"
#include "../../../Tools/GameContent/GameContentManager.h"
#include "../../../Tools/Managers/Achievements/AchievementManager.h"

class CAchievementList : ConfigurableList 
{
private :
	struct {
			wstring szMissingAchievementIcon;
	} AchievementIconPaths;

	GAMECONTENT_LIST_STATE m_sListState;
	AchievementInformation m_AchievementList;

public:

	XUI_IMPLEMENT_CLASS( CAchievementList, L"AchievementList", XUI_CLASS_LIST );

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
	XUI_END_MSG_MAP()


	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);

	~CAchievementList(){AchievementManager::getInstance().Clear();};
};
