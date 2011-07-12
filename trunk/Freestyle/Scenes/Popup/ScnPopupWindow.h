#pragma once

#include "../Abstracts/ConfigurableScene/ConfigurableScene.h"
#include "../Abstracts/HideableScene/HideableScene.h"

class CScnPopup:public HideableScene
{
protected :
	CXuiControl m_ShowButton;
	CXuiControl m_ToggleButton;
	CXuiControl m_HideButton;

	BOOL isVisible;

	bool hasShowButton;
	bool hasHideButton;
	bool hasToggleButton;

public :
	
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
	XUI_END_MSG_MAP()

	~CScnPopup();
	XUI_IMPLEMENT_CLASS(CScnPopup,L"ScnPopup",XUI_CLASS_SCENE);

	HRESULT CScnPopup::OnInit( XUIMessageInit *pInitData, BOOL &bHandled );
	HRESULT CScnPopup::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
};
