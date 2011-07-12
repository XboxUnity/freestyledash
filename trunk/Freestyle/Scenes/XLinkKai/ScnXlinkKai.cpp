#pragma once

#include "stdafx.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/General/KaiManager.h"
#include "../../Tools/XlinkKai/KaiNotifyManager.h"
#include "../../Application/FreestyleApp.h"

#include "ScnXlinkKai.h"

HRESULT CScnXlinkKai::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{	
	DebugMsg("CScnXlinkKai", "CScnXlinkKai::OnInit");

    // Retrieve controls for later use.
    GetChildById( L"SignIn", &m_btnSignIn );
	GetChildById( L"PlayOnline", &m_btnPlayOnline );
	GetChildById( L"Contacts", &m_btnContacts );
	GetChildById( L"Chat", &m_btnChat );
	GetChildById( L"Arena", &m_btnArena );

	m_bIsLoggedIn =  false;

    return S_OK;
}

HRESULT CScnXlinkKai::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
    // which timer is it?
    switch( pTimer->nId )
    {
		case TM_KAISIGNIN:
		{
			KillTimer(TM_KAISIGNIN);
			//EnterCriticalSection(&lock);
			//Logged into Kai enable the Menu Items
			int FrameStart, FrameEnd;
			this->FindNamedFrame(strtowstr("KaiSignIn").c_str(), &FrameStart);
			this->FindNamedFrame(strtowstr("KaiSignInEnd").c_str(), &FrameEnd);
			if(FrameStart != -1 && FrameEnd != -1)
			{
				this->PlayTimeline(FrameStart, FrameStart, FrameEnd, false, false);
			}

			m_bIsLoggedIn =  true;

			bHandled = TRUE;
			//LeaveCriticalSection(&lock);
			break;	
		}
		case TM_KAISIGNOUT:
		{
			KillTimer(TM_KAISIGNOUT);
			//EnterCriticalSection(&lock);
			if(m_bIsLoggedIn)
			{
				DebugMsg("ScnXlinkKai","Signed Out");
				//Logged out Kai disable the Menu Items

				int FrameStart, FrameEnd;
				this->FindNamedFrame(strtowstr("KaiSignOut").c_str(), &FrameStart);
				this->FindNamedFrame(strtowstr("KaiSignOutEnd").c_str(), &FrameEnd);
				if(FrameStart != -1 && FrameEnd != -1)
				{
					this->PlayTimeline(FrameStart, FrameStart, FrameEnd, false, false);
				}

				//Sleep(100);
				CKaiNotifyManager::getInstance().NotifyXlinkKaiOffline("Username Here");
				m_bIsLoggedIn =  false;
			}

			bHandled = TRUE;
			//LeaveCriticalSection(&lock);
			break;
		}
	};

	return S_OK;
}

HRESULT CScnXlinkKai::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (hObjPressed == m_btnSignIn)
	{
		if(!m_bIsLoggedIn)
		{
			if(CFreestyleApp::getInstance().hasInternetConnection() == false) {
				XNotifyQueueUICustom(L"No network connection detected.");
			} else {
				SkinManager::getInstance().setScene("XlinkKaiSignIn.xur", *this, true, "KaiSignIn");
			}
		}
		else
		{
			KaiManager::getInstance().KaiShutdown();
		}

		bHandled = TRUE;
	}
	else if (hObjPressed == m_btnContacts)
	{
		XNotifyQueueUICustom(L"Feature Coming Soon");
		bHandled = TRUE;
	}
	else if (hObjPressed == m_btnArena)
	{
		//End chat before entering as a catch-all
		KaiManager::getInstance().KaiLeaveChatmode();

		SkinManager::getInstance().setScene("XlinkKaiArena.xur", *this, false);
		bHandled = TRUE;
	}
	else if (hObjPressed == m_btnChat)
	{
		KaiManager::getInstance().KaiMessengerMode();

		SkinManager::getInstance().setScene("XlinkKaiChat.xur", *this, false);
		bHandled = TRUE;
	}
	else if(hObjPressed == m_btnPlayOnline)
	{
		VariablesCache::getInstance().setVariable("GameListShowType","XLINKKAI");
		if(SETTINGS::getInstance().getGameListVisual() == 1)
			SkinManager::getInstance().setScene("gamelist_cover.xur", *this, false, "GameListScene");
		else if(SETTINGS::getInstance().getGameListVisual() == 2)
			SkinManager::getInstance().setScene("gamelist_banners.xur", *this, false, "GameListScene");
		else
			SkinManager::getInstance().setScene("gamelist_icon.xur", *this, false, "GameListScene");
		bHandled = TRUE;
	}

    return S_OK;
}

HRESULT CScnXlinkKai::OnNotifyAccountLoggedIn( KAIMessageOnAccountLoggedIn * pOnAccountLoggedIn, BOOL& bHandled )
{
	SetTimer(TM_KAISIGNIN, 50);
	return S_OK;
}

HRESULT CScnXlinkKai::OnNotifyDeinitialise( KAIMessageOnDeinitialise * pOnDeinitialise, BOOL& bHandled )
{
	SetTimer(TM_KAISIGNOUT, 50);
	return S_OK;
}