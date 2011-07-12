#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/Managers/Weather/WeatherSearch.h"
#include "../../Tools/Managers/Skin/SkinXMLReader/SkinXMLReader.h"

using namespace std;


class CScnWeatherLocation : public CXuiSceneImpl, public iWeatherLocation
{
private:
	XOVERLAPPED keyboard;
	WCHAR * buffer;

	wstring temp;
protected:

	CXuiControl m_Search, m_LocationInput;
	CXuiCheckbox m_DisplayMetric;
	CXuiList m_LocationList;
	CXuiTextElement m_CurrentLocation;

	string * m_Names;
	string * m_ID;
	string m_LocationSearch;

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

	void UpdateWeatherSearch(vector<LocationSearch> m_currentLocation);

	virtual void setWeatherLocation(vector<LocationSearch> m_currentLocation) {  UpdateWeatherSearch(m_currentLocation); }

    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPress(HXUIOBJ hObjPressed, BOOL& bHandled);
	HRESULT OnTimer(XUIMessageTimer *pTimer, BOOL& bHandled);

public:

	CScnWeatherLocation(){}
	~CScnWeatherLocation();

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnWeatherLocation, L"ScnWeatherLocation", XUI_CLASS_TABSCENE )
};
