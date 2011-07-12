#pragma once

#include "../../../../Tools/Generic/tools.h"
#include "../../../../Tools/Managers/Theme/TabManager/TabManager.h"
#include "../../../../Scenes/Abstracts/ConfigurableScene/ConfigurableScene.h"
#include "../../../../Tools/Threads/ThreadLock.h"

using namespace std;

class CScnTabPresenter : public ConfigurableScene, public iThemeTabObserver
{
protected:

	// XUI Elements
	CXuiTextElement m_ParentIndex;
	CXuiTextElement m_ChildIndex;
	CXuiImageElement m_Background;
	CXuiImageElement m_Reflection;
	CXuiTextElement m_MainText;
	CXuiTextElement m_ReflectionText;
	CXuiControl m_Icon;
	CXuiControl m_Animation;
	CXuiControl m_Gradient;

	int nPresenter;
	
	// Variables to store which controls are present
	bool hasParentIndex, hasChildIndex;
	bool hasBackground, hasReflection;
	bool hasMainText, hasReflectionText;
	bool hasIcon, hasAnimation, hasGradient;

	// Access to Thread
	ThreadLock m_lock;

	// Message Map
	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_SET_FOCUS( OnNotifySetFocus )
	XUI_END_MSG_MAP()

	// Xui Message Handlers
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifySetFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled );

private:
	// Handle notification to update tab properties
	void UpdateTabProperties(int nParent, int nChild, int nFlag );
	void RefreshMenu(int nFlag);

	//Store the values of the ParentIndex and ChildIndex text elements
	int thisParent, thisChild;
	
	HRESULT InitializeChildren( void );

public:
	
	// Constructor / Destructor
	CScnTabPresenter();
	~CScnTabPresenter();

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnTabPresenter, L"ScnTabPresenter", XUI_CLASS_TABSCENE );

	// iThemeTabObserver Implementation
	virtual void handleMenuRefresh( int nFlag )
	{
		RefreshMenu( nFlag );
	}
	virtual void handleContentTabChange( int nParent, int nChild, int nFlag )
	{ 
		UpdateTabProperties(nParent, nChild, nFlag);
	}
};