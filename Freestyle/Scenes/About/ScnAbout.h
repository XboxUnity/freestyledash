#pragma once

#include "../../Tools/Generic/tools.h"

using namespace std;

class CScnAbout : public CXuiTabSceneImpl
{

protected:

	CXuiTextElement m_SkinVersion;
	CXuiTextElement m_KernelVersion;
	CXuiTextElement m_SkinName;
	CXuiTextElement m_XexVersion;
	CXuiTextElement m_MemberList1;
	CXuiTextElement m_MemberList2;
	CXuiTextElement m_MemberList3;
	CXuiTextElement m_ThanksList1;
	CXuiTextElement m_ThanksList2;
	CXuiTextElement m_ThanksList3;

	bool hasSkinVersion, hasXexVersion, hasKernelVersion;
	bool hasSkinName, hasMemberList1, hasMemberList2, hasMemberList3;
	bool hasThanksList1, hasThanksList2, hasThanksList3;

    // Message map.
	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
    HRESULT InitializeChildren( void );
	void ReadSkinData( void );
	void SetFSDCredits( void );
	void SetThanksCredits( void );
	
	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );

public:

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnAbout, L"ScnAbout", XUI_CLASS_TABSCENE )
};
