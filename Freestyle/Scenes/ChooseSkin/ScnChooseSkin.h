#pragma once

#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Generic/tools.h"

class CScnChooseSkin : CXuiSceneImpl
{
public:
	
	CXuiList m_SkinList;
	CXuiControl m_Back, m_Reload, m_customizeSkin;
	CXuiImageElement m_PreviewImage, m_AuthorIcon;
	CXuiTextElement m_SkinTitle, m_Version, m_Author, m_XZP;

	bool skinChanged;
	bool listLoaded;

	int activeSkin;
	int currentSelected;

	LPCWSTR* btnReload;

	XUI_IMPLEMENT_CLASS( CScnChooseSkin, L"ScnChooseSkin", XUI_CLASS_TABSCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_MSG_RETURN( OnMsgReturn )
	XUI_END_MSG_MAP()
	    
	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnMsgReturn(XUIMessageMessageBoxReturn *pXUIMessageMessageBoxReturn, BOOL &bHandled);
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged * pNotifySel, BOOL& bHandled );

	void LoadSkinList();
	void ListInfo(int item);
};

