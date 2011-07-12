#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../ManagePaths/RemovePathCallBack.h"

class CScnWait : public CXuiTabSceneImpl
{

protected:
	CXuiScene m_Wait;

	CXuiTextElement m_OperationTitle;

	bool isNested;

	//pInitData Data Vars
	LPCWSTR waitTitle;
	int waitType;
	bool waitReboot;

    // Message map.
	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
    HRESULT InitializeChildren( void );
	
	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );

public:

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnWait, L"ScnWait", XUI_CLASS_TABSCENE )
};
