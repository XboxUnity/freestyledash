#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/Managers/Weather/WeatherFeed.h"
#include "../../Tools/Managers/Skin/SkinXMLReader/SkinXMLReader.h"
#include "../../Tools/Threads/ThreadLock.h"

using namespace std;


class CScnWeather : public CXuiSceneImpl, public SkinXMLReader, public iWeatherDisplayer
{
protected:
	struct {
		CXuiTextElement Current_Temp;
		CXuiTextElement Current_Feels;
		CXuiTextElement Current_Wind;
		CXuiTextElement Current_Humidity;
		CXuiTextElement Current_DewPoint;
		CXuiTextElement Current_Pressure;
		CXuiTextElement Current_Visibility;
		CXuiTextElement Current_UV;
		CXuiTextElement Current_Sunrise;
		CXuiTextElement Current_Sunset;
		CXuiTextElement Current_Moon_Type;
		CXuiImageElement Current_Image;
		CXuiImageElement Current_Moon_Image;
	} CurrentConditionControls;

	struct DayConditions{
		CXuiTextElement Forecast_Day_Conditions;
		CXuiTextElement Forecast_Title;
		CXuiTextElement Forecast_Day_High;
		CXuiTextElement Forecast_Day_Wind;
		CXuiTextElement Forecast_Day_Precip;
		CXuiTextElement Forecast_Day_Humidity;
		CXuiTextElement Forecast_Day_Sunrise;
		CXuiImageElement Forecast_Day_Image;
		CXuiTextElement Forecast_Night_Conditions;
		CXuiTextElement Forecast_Night_Low;
		CXuiTextElement Forecast_Night_Wind;
		CXuiTextElement Forecast_Night_Precip;
		CXuiTextElement Forecast_Night_Humidity;
		CXuiTextElement Forecast_Night_Sunset;
		CXuiImageElement Forecast_Night_Image;
	} m_Conditions[5];

	CXuiTextElement m_TitleText;
	CXuiScene m_CurrentConditions;
	CXuiScene m_ForecastDays[5];

	bool bUseCelcius;
	bool bUseMPH;

	ThreadLock m_Lock;
	string strMoonBasePath;
	string strTypeBasePath;
	string fileType;

	bool m_WaitingForDownload;

	void UpdateWeatherInfo( WeatherInfo inf );

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
	XUI_END_MSG_MAP()

	virtual void setWeather(WeatherInfo inf) {  UpdateWeatherInfo(inf); }

    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	void InitializeChildren( );

public:

	CScnWeather(){}
	~CScnWeather();


    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnWeather, L"ScnWeather", XUI_CLASS_TABSCENE )
};
