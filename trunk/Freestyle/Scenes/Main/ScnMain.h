
#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/Monitors/DVD/DVDMonitor.h"
#include "../../Tools/SMC/smc.h"
#include "../../Tools/DVDInfo/DVDItem.h"
#include "../../Tools/DVDInfo/DVDWorker.h"
#include "../../Tools/Monitors/SignIn/SignInMonitor.h"


using namespace std;

class CScnMain : public CXuiSceneImpl, public iDVDObserver, public iDVDItemRequester, public iSignInObserver
{

protected:

	//ThreadLock m_lock;

	CXuiScene Tab2;
	CXuiScene GamerView;

    // Message map.
    CXuiControl btnXBox360Games;
    CXuiControl btnXBox1Games;
    CXuiControl btnXBoxLiveArcade;
    CXuiControl btnHomebrew;
	CXuiControl btnEmulators;
	CXuiControl btnFavourites;
	CXuiControl btnRecent;
	CXuiControl m_LaunchButton;
	CXuiControl m_EjectButton;
	CXuiControl m_SignOutButton;
	CXuiControl m_ProfileSignIn;

	CXuiControl m_GameIcon;
	CXuiControl m_Title;
	CXuiControl m_Slot;
	
	CXuiTextElement m_PingLimit;

	DVDItem* Item;

	void handleStatusChange();

	void handleTrayOpenEvent();
	void handleTrayOpeningEvent();
	void handleTrayClosingEvent();
	void handleTrayCloseEvent();
	void handle360GameDVDInsertedEvent();
	void handleXbox1GameDVDInsertedEvent();
	void handleVideoDVDInsertedEvent();
	void handleDataDVDInsertedEvent();
	void handleNoDVDEvent();
	void handleUnknownDVDInsertedEvent();


	void ExtractCompleted(DVDItem* itm);
	void ReadingDisc( void );

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
	XUI_END_MSG_MAP()

    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );

public:

	void refreshFromCurrent();
	void SetTitle();
	void SetSlot();
	void SetGameIcon();


    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnMain, L"ScnMain", XUI_CLASS_TABSCENE )
};
