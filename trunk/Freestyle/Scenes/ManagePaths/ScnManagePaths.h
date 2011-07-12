#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/Generic/Xboxtools.h"
#include "../../stdafx.h"

using namespace std;


class CScnManagePaths :
	public CXuiSceneImpl
{
private :	
	int Itm;
public:

	CXuiScene m_ManagePaths;
	bool isNested, managePath;
	CXuiList m_PathList, m_Retail, m_Devkit;
	CXuiControl m_BackButton, m_Edit, m_Add, m_Delete, m_PickPath, m_Save;
	CXuiSlider m_ScanDepth;
	CXuiTextElement m_PathText, m_Label_Devkit, m_ScanDepthText, m_Label_Retail, m_Label_PathList,m_LabelScanDepth;
	CXuiElement m_Background_PathList;
	XOVERLAPPED keyboard;
	WCHAR * buffer;
	LPCWSTR * m_msgButtons;
	WAIT_INFO m_WaitInfo;

	CScnManagePaths(){};
	~CScnManagePaths(){if( m_msgButtons )delete [] m_msgButtons;};


	XUI_IMPLEMENT_CLASS( CScnManagePaths, L"ScnManagePaths", XUI_CLASS_TABSCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_MSG_RETURN( OnMsgReturn )
		XUI_ON_XM_NOTIFY_VALUE_CHANGED( OnNotifyValueChanged )
		XUI_ON_XM_NOTIFY_KILL_FOCUS( OnNotifyKillFocus )
		XUI_ON_XM_SET_FOCUS( OnSetFocus )
	XUI_END_MSG_MAP()

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnMsgReturn(XUIMessageMessageBoxReturn *pXUIMessageMessageBoxReturn, BOOL &bHandled);
	HRESULT OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled );
	HRESULT OnSetFocus(HXUIOBJ hObjLosingFocus, BOOL& bHandled);
	HRESULT OnNotifyKillFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled );

	void HideList( bool hide);
	void ShowButtons();
	void HideButtons();
	int HandleBack();
	void AddPath();
};
