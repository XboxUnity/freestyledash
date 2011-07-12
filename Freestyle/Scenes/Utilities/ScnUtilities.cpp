#include "stdafx.h"

#include "../../Tools/Settings/Settings.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Application/FreestyleApp.h"
#include "../../Tools/Managers/Skin/SkinManager.h"

#include "ScnUtilities.h"

HRESULT CScnUtilities::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	string strBuffer = "";
	InitializeChildren();

    return S_OK;
}


//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScnUtilities::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if(hObjPressed == m_Weather)
	{
		if(CFreestyleApp::getInstance().hasInternetConnection() == false)
		{
			XNotifyQueueUICustom(L"No network connection detected.");
		}
		else if(strcmp(SETTINGS::getInstance().getWeatherLocation().c_str(),"undefined") == 0)
		{
			XNotifyQueueUICustom(L"Please set location in settings.");
		}
		else
		{
			SkinManager::getInstance().setScene("Weather.xur", *this, false, "Weather");
		}

		bHandled = TRUE;
	}

    return S_OK;
}


HRESULT CScnUtilities::InitializeChildren( void )
{
	GetChildById(L"Weather", &m_Weather);

	return S_OK;
}