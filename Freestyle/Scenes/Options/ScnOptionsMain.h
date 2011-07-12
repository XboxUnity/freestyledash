#pragma once

#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Generic/tools.h"

class CScnOptionsMain : CXuiSceneImpl
{
public:
	enum SettingType 
	{
		GeneralSettings,
		ContentSettings,
		SystemSettings,
		XlinkKai,
		SettingListSize,
	};

	// Game List Setting Controls
	CXuiList m_SettingList;
	CXuiTextElement m_SettingTitle;


	// General Controls
	CXuiList m_GeneralSettingList;
	CXuiList m_GeneralSettingTitle;
	CXuiScene m_SettingScenes[4];

	int currentSelection;


	// Implement Class
    XUI_IMPLEMENT_CLASS( CScnOptionsMain, L"ScnOptionsMain", XUI_CLASS_SCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
	XUI_END_MSG_MAP()

	// Constructor / Destructor
	CScnOptionsMain();
	~CScnOptionsMain();

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );

	HRESULT InitializeChildren( void );
	void ShowSetting(int value);
};