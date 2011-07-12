#pragma once

#include "../../Tools/Generic/tools.h"

using namespace std;

class CScnUtilities : public CXuiTabSceneImpl
{

protected:

	CXuiControl m_Weather;

    // Message map.
	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
	XUI_END_MSG_MAP()

    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
    HRESULT InitializeChildren( void );
	
	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );

public:

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnUtilities, L"ScnUtilities", XUI_CLASS_TABSCENE )
};
