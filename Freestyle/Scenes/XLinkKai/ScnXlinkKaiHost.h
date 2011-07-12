#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/XlinkKai/KaiMessage.h"

using namespace std;

class CScnXlinkKaiHost : public CXuiSceneImpl
{
private:
	XOVERLAPPED keyboard;
	WCHAR * buffer;

	wstring temp;

protected:
    CXuiControl m_AddPlayer, m_SubtractPlayer, m_OK, m_back;
	CXuiControl m_Password, m_Description;
	CXuiCheckbox m_PrivateArena;

	CXuiTextElement m_PlayerLimit;
	CXuiScene m_KaiControls;

	bool isNested, enabled;
	int maxPlayers, inputType;
	string password, description;

	HRESULT EnableControls(bool toggle);
	string PasswordMask(int size);

public:

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_TIMER( OnTimer )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
	XUI_END_MSG_MAP()

    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnTimer(XUIMessageTimer *pTimer, BOOL& bHandled);
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnXlinkKaiHost, L"ScnXlinkKaiHost", XUI_CLASS_TABSCENE )
};
