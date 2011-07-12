#pragma once
#include "../../Tools/Generic/xboxtools.h"
#include "ScreenshotList/ScreenshotList.h"

class CScnScreenshots : public CXuiTabSceneImpl
{
private :

	CScreenshotList m_hScreenshotList;
	CXuiControl m_Back, m_showFullScreen;
	ScreenshotInformation ScreenshotListInfo;
	CXuiImageElement m_FullScreen;
	CXuiList m_ScreenshotList;

	CXuiScene m_Screenshots;
	bool isNested;

	GameListItem * pGameContent;

	bool fullScreen;
	bool m_bFullscreenMode;

	void CenterList();

protected :

public :
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_GET_SOURCE_IMAGE( OnGetSourceDataImage )
	XUI_END_MSG_MAP()

	CScnScreenshots() {}
	~CScnScreenshots() {}
	XUI_IMPLEMENT_CLASS(CScnScreenshots, L"ScnScreenshots", XUI_CLASS_TABSCENE);

	HRESULT OnInit( XUIMessageInit *pInitData, BOOL &bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
};