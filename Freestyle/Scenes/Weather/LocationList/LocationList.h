#pragma once

#include "../../../Tools/Generic/tools.h"
#include "../../../Tools/Settings/Settings.h"
#include "../../../Tools/Managers/Weather/WeatherSearch.h"

class CWeatherLocationList : public CXuiListImpl, public iWeatherLocation
{
private :
	wstring temp;
	void UpdateWeatherSearch(vector<LocationSearch> m_currentLocation);

	virtual void setWeatherLocation(vector<LocationSearch> m_currentLocation) {  UpdateWeatherSearch(m_currentLocation); }

public:

	string * m_Names;
	string * m_ID;

	int m_ListSize;
	bool m_bListLoaded;

	XUI_IMPLEMENT_CLASS( CWeatherLocationList, L"WeatherLocationList", XUI_CLASS_LIST );

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
	XUI_END_MSG_MAP()


	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);

	~CWeatherLocationList(){WeatherSearch::getInstance().remove(*this);}
};
