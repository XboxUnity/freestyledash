#pragma once

#include "../../../Tools/Generic/xboxtools.h"
#include "../../../Tools/GameContent/GameContentManager.h"
#include "../../../Tools/Managers/Screenshot/ScreenshotManager.h"

class CScreenshotList : public CXuiListImpl
{
private :
	struct
	{
		bool m_bListInitialized;
		int m_nScreenshotCount;
		int m_nListSize;
	} ListVariables;

	bool flag;
	GAMECONTENT_LIST_STATE m_sListState;
	ScreenshotInformation m_pScreenshotInfo;

	vector<string> m_vSSPaths;

public:
    XUI_IMPLEMENT_CLASS( CScreenshotList, L"ScreenshotList", XUI_CLASS_LIST );

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_SET_CURSEL( OnSetCurSel )
	XUI_END_MSG_MAP()

	~CScreenshotList() {ScreenshotManager::getInstance().Clear();}

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
	HRESULT OnSetCurSel(XUIMessageSetCurSel *pSetCurSelData, BOOL &bHandled );
};
