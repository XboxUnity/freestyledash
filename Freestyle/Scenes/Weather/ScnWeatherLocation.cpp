#include "stdafx.h"

#include "../../Tools/Generic/tools.h"
#include "../../Tools/Managers/Weather/WeatherSearch.h"
#include "../../Tools/Settings/Settings.h"
#include "ScnWeatherLocation.h"

CScnWeatherLocation::~CScnWeatherLocation()
{
	WeatherSearch::getInstance().remove(*this);
}


HRESULT CScnWeatherLocation::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	WeatherSearch::getInstance().add(*this);

	GetChildById( L"Search", &m_Search );
	GetChildById( L"LocationList", &m_LocationList );
	GetChildById( L"CurrentLocation", &m_CurrentLocation );
	GetChildById( L"LocationInput", &m_LocationInput );
	GetChildById( L"DisplayMetric", &m_DisplayMetric );

	m_LocationSearch = "";

	m_CurrentLocation.SetText(strtowstr(SETTINGS::getInstance().getWeatherLocationName()).c_str());
	m_DisplayMetric.SetCheck(SETTINGS::getInstance().getWeatherUnits());

	buffer = new WCHAR[512];

	return S_OK;
}

HRESULT CScnWeatherLocation::OnNotifyPress(HXUIOBJ hObjPressed, BOOL& bHandled)
{
	if ( hObjPressed == m_Search )
	{
		if(strcmp(m_LocationSearch.c_str(),"") != 0 )
			WeatherSearch::getInstance().UpdateInfo(m_LocationSearch);

		bHandled = TRUE;
	}
	else if ( hObjPressed == m_LocationInput )
	{
		memset(&keyboard, 0, sizeof(keyboard));
		memset(buffer, 0, sizeof(buffer));

		temp = strtowstr(m_LocationSearch);

		XShowKeyboardUI(0,VKBD_DEFAULT,temp.c_str(),L"Location Search",L"Enter a Zip Code or City you would like to search for.",buffer,32,&keyboard);
		SetTimer(TM_KEYBOARD, 50);	
	}
	else if ( hObjPressed == m_LocationList )
	{
		int currentIndex = m_LocationList.GetCurSel();
		wstring temp = strtowstr(m_Names[currentIndex]);

		m_CurrentLocation.SetText(temp.c_str());

		SETTINGS::getInstance().setWeatherLocation(m_ID[currentIndex]);
		SETTINGS::getInstance().setWeatherLocationName(m_Names[currentIndex]);
	}
	else if( hObjPressed == m_DisplayMetric ) 
	{
		SETTINGS::getInstance().setWeatherUnits(1 - SETTINGS::getInstance().getWeatherUnits());
		m_DisplayMetric.SetCheck(SETTINGS::getInstance().getWeatherUnits());
		bHandled = TRUE;
	
	}

	return S_OK;
}

HRESULT CScnWeatherLocation::OnTimer(XUIMessageTimer *pTimer, BOOL& bHandled)
{
    switch(pTimer->nId)
    {
		case TM_KEYBOARD:
		{
			if(XHasOverlappedIoCompleted(&keyboard))
            {
				KillTimer(TM_KEYBOARD);
				bHandled = TRUE;
				if(keyboard.dwExtendedError == ERROR_SUCCESS)
                {
					m_LocationSearch = wstrtostr(buffer);

					m_LocationInput.SetText(buffer);
				}				
			}
			break;
		}
    }
    return S_OK;
}

void CScnWeatherLocation::UpdateWeatherSearch(vector<LocationSearch> m_currentLocation)
{
	int m_ListSize = m_currentLocation.size();

	m_Names = new string[m_ListSize];
	m_ID = new string[m_ListSize];

	vector<LocationSearch>::iterator itr;
	int count = 0;
	for(itr = m_currentLocation.begin(); itr != m_currentLocation.end(); itr++)
	{
		m_Names[count] = (*itr).strName;
		m_ID[count] = (*itr).strID;

		count++;
	}
}
