#include "stdafx.h"

#include "../../Tools/Settings/Settings.h"
#include "../../Tools/Managers/VariablesCache/VariablesCache.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Managers/Drives/DrivesManager.h"
#include "../../Tools/Monitors/DVD/DVDMonitor.h"
#include "../../Tools/SMC/smc.h"
#include "../../Tools/SQLite/FSDSql.h"
#include "../../Tools/ContentList/ContentManager.h"
#include "../../Tools/DVDInfo/DVDItem.h"
#include "../../Tools/DVDInfo/DVDWorker.h"
#include "../../Tools/Managers/Theme/TabManager/TabManager.h"
#include "../../Tools/Monitors/SignIn/SignInMonitor.h"


#include "ScnMain.h"

void CScnMain::handleStatusChange() 
{
	if(IsUserSignedIn(0)) { 
		m_SignOutButton.SetShow(TRUE);
		m_ProfileSignIn.SetText(L"Switch");
	} else {
		m_SignOutButton.SetShow(FALSE);
		m_ProfileSignIn.SetText(L"Sign In");
	}
}

HRESULT CScnMain::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{	
	DebugMsg("ScnMain","CScnMain::OnInit");
	Item = NULL;

	SignInMonitor::getInstance().AddObserver(*this);

    // Retrieve controls for later use.
    HRESULT hr = GetChildById( L"XBox360Games", &btnXBox360Games );
    hr = GetChildById( L"XBox1Games", &btnXBox1Games );
    hr = GetChildById( L"XBoxLiveArcade", &btnXBoxLiveArcade );
    hr = GetChildById( L"Homebrew", &btnHomebrew );
    hr = GetChildById( L"Emulators", &btnEmulators );
    hr = GetChildById( L"Favourites", &btnFavourites );
    hr = GetChildById( L"RecentGames", &btnRecent );
	hr = GetChildById( L"PingLimit", &m_PingLimit );
	hr = GetChildById( L"LaunchButton", &m_LaunchButton );
	hr = GetChildById( L"EjectButton", &m_EjectButton );
	hr = GetChildById( L"SignOutButton", &m_SignOutButton );
	hr = GetChildById( L"ProfileSignIn", &m_ProfileSignIn );
	GetChildById( L"Tab2", &Tab2 );
	Tab2.GetChildById( L"GamerView", &GamerView );

	GamerView.GetChildById( L"GameIconScene", &m_GameIcon );
	GamerView.GetChildById( L"TitleScene", &m_Title );
	GamerView.GetChildById( L"SlotScene", &m_Slot );

	DWORD * pingAddress = NULL;
	if(VersionManager::getInstance().getKernelVersion().Build == 9199)
		pingAddress = (DWORD*)0x819831A8; // 9199
	else if(VersionManager::getInstance().getKernelVersion().Build == 12611)
		pingAddress = (DWORD*)0x8191D33C; // 12611



	if(pingAddress != NULL && *pingAddress == 0x60000000)
	{
		DebugMsg("ScnMain", "System Link Patch Active");
		m_PingLimit.SetShow(true);
	}
	else
	{
		DebugMsg("Test", "System Link Patch Inactive");
		m_PingLimit.SetShow(false);
	}

	if(IsUserSignedIn(0)) {
		m_SignOutButton.SetShow(TRUE);
		m_ProfileSignIn.SetText(L"Switch");
	} else {
		m_SignOutButton.SetShow(FALSE);
		m_ProfileSignIn.SetText(L"Sign In");
	}
	
	smc mySmc;
	if (mySmc.GetTrayState() == CLOSED ||  mySmc.GetTrayState() == CLOSING )
	{
		m_EjectButton.SetText(L"Open Tray");
		TabManager::getInstance().setAlternateText(0, 0, "Open Tray", TM_MENU_ACTIVE);
	} 
	else if (mySmc.GetTrayState() == OPEN ||  mySmc.GetTrayState() == OPENING )
	{
		m_EjectButton.SetText(L"Close Tray");
		TabManager::getInstance().setAlternateText(0, 0, "Close Tray", TM_MENU_ACTIVE);
	}

	CXuiControl m_Control; bool temp;
	XuiInitControl(L"DVDRegister", m_Control, &temp);
	if(temp)
		DVDMonitor::getInstance().add(*this);

    return S_OK;
}


//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScnMain::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if(hObjPressed == btnXBoxLiveArcade)
	{
		if (Item != NULL)
			Item->setRequester(NULL);
		VariablesCache::getInstance().setVariable("GameListShowType","XBLA");
		if(SETTINGS::getInstance().getGameListVisual() == 1)
			SkinManager::getInstance().setScene("gamelist_cover.xur", *this, false, "GameListScene");
		else if(SETTINGS::getInstance().getGameListVisual() == 2)
			SkinManager::getInstance().setScene("gamelist_banners.xur", *this, false, "GameListScene");
		else
			SkinManager::getInstance().setScene("gamelist_icon.xur", *this, false, "GameListScene");
		bHandled = true;
	} else
	if(hObjPressed == btnXBox360Games)
	{
		if (Item != NULL)
			Item->setRequester(NULL);
		VariablesCache::getInstance().setVariable("GameListShowType","360");
		if(SETTINGS::getInstance().getGameListVisual() == 1)
			SkinManager::getInstance().setScene("gamelist_cover.xur", *this, false, "GameListScene");
		else if(SETTINGS::getInstance().getGameListVisual() == 2)
			SkinManager::getInstance().setScene("gamelist_banners.xur", *this, false, "GameListScene");
		else
			SkinManager::getInstance().setScene("gamelist_icon.xur", *this, false, "GameListScene");
		bHandled = true;
	} else
	if(hObjPressed == btnHomebrew)
	{
		if (Item != NULL)
			Item->setRequester(NULL);
		VariablesCache::getInstance().setVariable("GameListShowType","HOMEBREW");
		if(SETTINGS::getInstance().getGameListVisual() == 1)
			SkinManager::getInstance().setScene("gamelist_cover.xur", *this, false, "GameListScene");
		else if(SETTINGS::getInstance().getGameListVisual() == 2)
			SkinManager::getInstance().setScene("gamelist_banners.xur", *this, false, "GameListScene");
		else
			SkinManager::getInstance().setScene("gamelist_icon.xur", *this, false, "GameListScene");
		bHandled = true;
	} else
	if(hObjPressed == btnEmulators)
	{
		if (Item != NULL)
			Item->setRequester(NULL);
		VariablesCache::getInstance().setVariable("GameListShowType","EMULATOR");
		if(SETTINGS::getInstance().getGameListVisual() == 1)
			SkinManager::getInstance().setScene("gamelist_cover.xur", *this, false, "GameListScene");
		else if(SETTINGS::getInstance().getGameListVisual() == 2)
			SkinManager::getInstance().setScene("gamelist_banners.xur", *this, false, "GameListScene");
		else
			SkinManager::getInstance().setScene("gamelist_icon.xur", *this, false, "GameListScene");
	} else
	if(hObjPressed == btnXBox1Games)
	{
		if (Item != NULL)
			Item->setRequester(NULL);
		VariablesCache::getInstance().setVariable("GameListShowType","XBOX1");
		if(SETTINGS::getInstance().getGameListVisual() == 1)
			SkinManager::getInstance().setScene("gamelist_cover.xur", *this, false, "GameListScene");
		else if(SETTINGS::getInstance().getGameListVisual() == 2)
			SkinManager::getInstance().setScene("gamelist_banners.xur", *this, false, "GameListScene");
		else
			SkinManager::getInstance().setScene("gamelist_icon.xur", *this, false, "GameListScene");
	} else
	if(hObjPressed == btnFavourites)
	{
		if(strcmp(VariablesCache::getInstance().getVariable("GameListShowFavorite","No").c_str(),"No") == 0)
		{
			VariablesCache::getInstance().setVariable("GameListShowFavorite","Yes");
		}
		else
		{
			VariablesCache::getInstance().setVariable("GameListShowFavorite","No");
		}
		bHandled = true;
	} 
	else if( hObjPressed == m_SignOutButton )
	{
		for(int i = 0; i < 4; i ++ )
		{
			ForcePlayerSignOut(i);
		}
		XNotifyQueueUICustom(L"You have signed out of your profile.");
	}
	else if( hObjPressed == m_ProfileSignIn )
	{
		XShowSigninUI(1, 0);
	}
	else if(hObjPressed == btnRecent)
	{
		if(strcmp(VariablesCache::getInstance().getVariable("GameListShowRecent","No").c_str(),"No") == 0)
		{
			VariablesCache::getInstance().setVariable("GameListShowRecent","Yes");
		}
		else
		{
			VariablesCache::getInstance().setVariable("GameListShowRecent","No");
		}
		bHandled = true;
	} else if(hObjPressed == m_LaunchButton)
	{
		bHandled= true;
		smc mySmc;
		DebugMsg("ScnQuickGameView","Launch");

		if(Item != NULL && mySmc.GetTrayState() == CLOSED )
		{
			//WGAMES current = m_GameList.getCurrentGame();
			DebugMsg("ScnQuickGameView","Launch");
			bHandled = true;
			Sleep(100);
			if (Item->getFileType() == CONTENT_DVD_TYPE_XEX)
			{
				XLaunchNewImage("dvd:\\default.xex", 0);
			} else if (Item->getFileType() == CONTENT_DVD_TYPE_XBE)
			{
				XLaunchNewImage("dvd:\\default.xbe", 0);
			} else if (Item->getFileType() == CONTENT_DVD_TYPE_DVD) 
			{
				// Launch our DVD			
				// Create our launch data and zero it
				BYTE launchData[0x3FC];
				ZeroMemory(launchData, 0x3FC);

				// Set our 2 magic bytes
				launchData[7] = 0x07;
				launchData[11] = 0xFF;
			
				DWORD status = XSetLaunchData(launchData, 0x3FC);
				XLaunchNewImage(XLAUNCH_KEYWORD_DASH, 0);
			}
		
		}
		else
		{
			//If tray is open close, if tray is closed open
			smc mySmc;
			bHandled = true;

			if (mySmc.GetTrayState() == CLOSED ||  mySmc.GetTrayState() == CLOSING )
			{
				mySmc.OpenTray();
				m_EjectButton.SetText(L"Close Tray");
				//TabManager::getInstance().setAlternateText(0, 0, "Close Tray", TM_MENU_ACTIVE);
			} 
			else if (mySmc.GetTrayState() == OPEN ||  mySmc.GetTrayState() == OPENING )
			{
				mySmc.CloseTray();
				m_EjectButton.SetText(L"Open Tray");
				//TabManager::getInstance().setAlternateText(0, 0, "Open Tray", TM_MENU_ACTIVE);
			}
		}
	} else if (hObjPressed == m_EjectButton)
	{
		smc mySmc;
		bHandled = true;

		if (mySmc.GetTrayState() == CLOSED ||  mySmc.GetTrayState() == CLOSING )
		{
			mySmc.OpenTray();
			//m_EjectButton.SetText(L"Close Tray");
			//TabManager::getInstance().setAlternateText(0, 0, "Close Tray", TM_MENU_ACTIVE);
		} 
		else if (mySmc.GetTrayState() == OPEN ||  mySmc.GetTrayState() == OPENING )
		{
			mySmc.CloseTray();
			//m_EjectButton.SetText(L"Open Tray");
			//TabManager::getInstance().setAlternateText(0, 0, "Open Tray", TM_MENU_ACTIVE);
		}
	}
	VariablesCache::getInstance().DumpVariablesToDebug();

    return S_OK;
}


void CScnMain::handleTrayOpenEvent()
{
	TabManager::getInstance().setAlternateText(0, 0, "Close Tray", TM_MENU_ACTIVE);
	m_EjectButton.SetText(L"Close Tray");
	m_Title.SetText(L"");
	m_Slot.SetImage(L"");
	Item = NULL;
	m_GameIcon.SetImage(L"");
}

void CScnMain::handleUnknownDVDInsertedEvent()
{
	TabManager::getInstance().setAlternateText(0, 0, "Unknown Disc", TM_MENU_ACTIVE);
	m_EjectButton.SetText(L"Open Tray");
	m_Title.SetText(L"");
	m_Slot.SetImage(L"");
	Item = NULL;
	m_GameIcon.SetImage(L"");
}

void CScnMain::handleTrayOpeningEvent()
{
	TabManager::getInstance().setAlternateText(0, 0, "Opening Tray...", TM_MENU_ACTIVE);
	m_EjectButton.SetText(L"Close Tray");
	m_Title.SetText(L"");
	m_Slot.SetImage(L"");
	Item = NULL;
	m_GameIcon.SetImage(L"");
}

void CScnMain::handleTrayClosingEvent()
{
	TabManager::getInstance().setAlternateText(0, 0, "Closing Tray...", TM_MENU_ACTIVE);
	m_EjectButton.SetText(L"Open Tray");
	m_Title.SetText(L"");
	m_Slot.SetImage(L"");
	Item = NULL;
	m_GameIcon.SetImage(L"");
}

void CScnMain::handleTrayCloseEvent()
{
	TabManager::getInstance().setAlternateText(0, 0, "Checking For Disc...", TM_MENU_ACTIVE);
	m_EjectButton.SetText(L"Open Tray");
	m_Title.SetText(L"");
	m_Slot.SetImage(L"");
	Item = NULL;
	m_GameIcon.SetImage(L"");

}

void CScnMain::handleNoDVDEvent()
{
	TabManager::getInstance().setAlternateText(0, 0, "Open Tray", TM_MENU_ACTIVE);
	m_Title.SetText(L"");
	m_Slot.SetImage(L"");
	Item = NULL;
	m_GameIcon.SetImage(L"");
}

void CScnMain::handleDataDVDInsertedEvent()
{
	//m_lock.Lock();
	DVDItem* item = new DVDItem(CONTENT_DVD_TYPE_DATA);
	item->setRequester(this);
	DVDWorker::getInstance().StartItem(item);
	//m_lock.Unlock();

}

void CScnMain::SetTitle()
{
	DebugMsg("ScnMain", "SetTitle to [%s]", Item->getTitle().c_str());
	TabManager::getInstance().setAlternateText(0, 0, Item->getTitle(), TM_MENU_ACTIVE);
	//m_Title.SetText(strtowstr(Item->getTitle()).c_str());
}


void CScnMain::SetSlot()
{
	m_Slot.SetImage(L"");

	BYTE * bIconData;
	DWORD iconSize;
	iconData data;
	data = Item->getSlotData();
	iconSize = data.thumbnailSize;
	bIconData = data.thumbnailData;
	string p1 = sprintfaA("memory://%X,%X", bIconData, iconSize);
	DebugMsg("CScnDVDImage", "SetSlotIcon [%s]", p1.c_str());
	string path = p1;
	DebugMsg("CScnDVDImage", "SetSlotIcon2 [%s]", path.c_str());

	DebugMsg("CCScnDVDImage", "SetSlotIcon:: path = %s", path.c_str());
	if (Item->getHasSlot())
	{
		DebugMsg("CScnDVDImage", "it exists");
		HRESULT hr = m_Slot.SetImage(strtowstr(path).c_str());
		DWORD le = GetLastError();
		DebugMsg("CScnDVDImage", "SetImage returned [%08X] le [%08X]", hr, le);
	}
	else {
		DebugMsg("CScnDVDImage", "it DOESN'T exist");
	}
}


void CScnMain::SetGameIcon()
{
	m_GameIcon.SetImage(L"");

	BYTE * bIconData;
	DWORD iconSize;
	iconData data;
	data = Item->getThumbData();
	iconSize = data.thumbnailSize;
	bIconData = data.thumbnailData;
	string p1 = sprintfaA("memory://%X,%X", bIconData, iconSize);
	DebugMsg("ScnMain", "SetGameIcon [%s]", p1.c_str());
	string path = p1;
	DebugMsg("ScnMain", "SetGameIcon2 [%s]", path.c_str());

	DebugMsg("CScnMain", "SetGameIcon:: path = %s", path.c_str());
	if (Item->getHasThumb())
	{
		DebugMsg("ScnMain", "it exists");
		HRESULT hr = m_GameIcon.SetImage(strtowstr(path).c_str());
		DWORD le = GetLastError();
		DebugMsg("ScnMain", "SetImage returned [%08X] le [%08X]", hr, le);
	}
	else {
		DebugMsg("ScnMain", "it DOESN'T exist");
	}
}


void CScnMain::refreshFromCurrent()
{ 
	//m_lock.Lock();
	if(Item != NULL)
	{	
		SetTitle();
		if (Item->getHasSlot())
			SetSlot();
		if (Item->getHasThumb())
			SetGameIcon();
	}
	//Show();
	//m_lock.Unlock();
}

void CScnMain::handle360GameDVDInsertedEvent()
{
	//m_lock.Lock();
	DVDItem* itm = new DVDItem(CONTENT_DVD_TYPE_XEX);
	itm->setRequester(this);
	DVDWorker::getInstance().StartItem(itm);

	//m_lock.Unlock();

}

void CScnMain::handleVideoDVDInsertedEvent()
{
	//m_lock.Lock();
	DVDItem* item = new DVDItem(CONTENT_DVD_TYPE_DVD);
	item->setRequester(this);
	DVDWorker::getInstance().StartItem(item);
	//m_lock.Unlock();
}

void CScnMain::handleXbox1GameDVDInsertedEvent()
{
	DebugMsg("Main", "Xbox 1 Inserted");
	//m_lock.Lock();
	DVDItem* itm = new DVDItem(CONTENT_DVD_TYPE_XBE);
	itm->setRequester(this);
	DVDWorker::getInstance().StartItem(itm);

	//m_lock.Unlock();
	
}

void CScnMain::ExtractCompleted(DVDItem* itm) {
	
	Item = itm;
	refreshFromCurrent();
}

void CScnMain::ReadingDisc() 
{
	// Update tab to show status of disc read
	TabManager::getInstance().setAlternateText(0, 0, "Reading Disc...", TM_MENU_ACTIVE);
}