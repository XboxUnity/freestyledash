#pragma once
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/GameContent/GameContentManager.h"

class CScnGameSaves : public CXuiTabSceneImpl
{
private :

	string curTitleID;
	int curTitleIDCounter;
	GAMECONTENT_LIST_STATE listPack;

protected :

	CXuiList m_List;
	CXuiImageElement m_Banner;
public :
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_IMAGE( OnGetSourceDataImage )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
	XUI_END_MSG_MAP()

	~CScnGameSaves();
	XUI_IMPLEMENT_CLASS(CScnGameSaves, L"ScnGameSaves", XUI_CLASS_TABSCENE);

	HRESULT OnInit( XUIMessageInit *pInitData, BOOL &bHandled );
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
};