#include "stdafx.h"

#include "../../Tools/Generic/tools.h"
#include "../../Tools/Managers/Weather/WeatherFeed.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Settings/Settings.h"
#include "ScnWeather.h"

CScnWeather::~CScnWeather()
{
	WeatherFeed::getInstance().remove(*this);
}

HRESULT CScnWeather::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	InitializeChildren();
	WeatherFeed::getInstance().add(*this);
	WeatherFeed::getInstance().UpdateInfo();

	LoadSettings("ScnWeather", "Weather");

	string skinName = SkinManager::getInstance().getCurrentScenePath();

	fileType = GetSetting("FILETYPE", "dds");

	strMoonBasePath = GetSetting("MOONBASEPATH", "");
	strTypeBasePath = GetSetting("TYPEBASEPATH", "");

	if(SkinManager::getInstance().isCurrentCompressed())
	{
		strMoonBasePath = sprintfaA("file://%s%s",skinName.c_str(),strMoonBasePath.c_str());
		strTypeBasePath = sprintfaA("file://%s%s",skinName.c_str(),strTypeBasePath.c_str());
	}
	else
	{
		strMoonBasePath = sprintfaA("file://%s\\%s",skinName.c_str(),strMoonBasePath.c_str());
		strTypeBasePath = sprintfaA("file://%s\\%s",skinName.c_str(),strTypeBasePath.c_str());
	}

	strMoonBasePath = str_replaceallA(strMoonBasePath,"\\\\","\\");
	strTypeBasePath = str_replaceallA(strTypeBasePath,"\\\\","\\");

	bUseCelcius = false;
	return S_OK;
}

void CScnWeather::UpdateWeatherInfo( WeatherInfo inf )
{
	/////////////////////////////////////////////////////////
	// Get Locale Based Measurements
	/////////////////////////////////////////////////////////
	string UnitTemp		= inf.currentLocaleInfo.strUnitTemp;
	string UnitDistance	= inf.currentLocaleInfo.strUnitDistance;
	string UnitSpeed	= inf.currentLocaleInfo.strUnitSpeed;
	string UnitPressure	= inf.currentLocaleInfo.strUnitPressure;
	string UnitAmount	= inf.currentLocaleInfo.strUnitAmount;

	/////////////////////////////////////////////////////////
	// Set Global Scn Variables
	/////////////////////////////////////////////////////////
	m_TitleText.SetText(strtowstr(inf.currentGlobalConditions.strLocation).c_str());

	/////////////////////////////////////////////////////////
	// Set Current Conditions
	/////////////////////////////////////////////////////////
	string CurrentTemp	= sprintfaA("%d °%s", inf.currentConditions.nCurrentTemp, UnitTemp.c_str());
	string FeelsLike	= sprintfaA("%d °%s", inf.currentConditions.nFeelsLikeTemp, UnitTemp.c_str());
	string Humidity		= sprintfaA("%d%%", inf.currentConditions.nHumidity);
	string DewPoint		= sprintfaA("%d °%s", inf.currentConditions.nDewPoint, UnitTemp.c_str());
	string Wind			= sprintfaA("From %s at %d%s",inf.currentConditions.strWindDirection.c_str(), inf.currentConditions.nWindVelocity, UnitSpeed.c_str());
	string Pressure		= sprintfaA("%04.2f %s", inf.currentConditions.nBarPressure, UnitPressure.c_str());
	string Visibility	= sprintfaA("%04.2f %s", inf.currentConditions.nVisibility, UnitDistance.c_str());
	string UVIndex		= sprintfaA("%d - %s", inf.currentConditions.nUvIndex, inf.currentConditions.strUvDesc.c_str());
	string TypePath		= sprintfaA("%s\\%d.%s", strTypeBasePath.c_str(),inf.currentConditions.nConditionIcon, fileType.c_str());
	string MoonPath		= sprintfaA("%s\\%d.%s", strMoonBasePath.c_str(),inf.currentConditions.nMoonPhaseIcon, fileType.c_str());

	m_Lock.Lock();
	CurrentConditionControls.Current_Sunrise.SetText(strtowstr(inf.currentGlobalConditions.strSunrise).c_str());
	CurrentConditionControls.Current_Sunset.SetText(strtowstr(inf.currentGlobalConditions.strSunste).c_str());
	CurrentConditionControls.Current_Temp.SetText(strtowstr(CurrentTemp).c_str());
	CurrentConditionControls.Current_DewPoint.SetText(strtowstr(DewPoint).c_str());
	CurrentConditionControls.Current_Feels.SetText(strtowstr(FeelsLike).c_str());
	CurrentConditionControls.Current_Humidity.SetText(strtowstr(Humidity).c_str());
	CurrentConditionControls.Current_Moon_Type.SetText(strtowstr(inf.currentConditions.strMoonPhase).c_str());
	CurrentConditionControls.Current_Pressure.SetText(strtowstr(Pressure).c_str());
	CurrentConditionControls.Current_Temp.SetText(strtowstr(CurrentTemp).c_str());
	CurrentConditionControls.Current_UV.SetText(strtowstr(UVIndex).c_str());
	CurrentConditionControls.Current_Visibility.SetText(strtowstr(Visibility).c_str());
	CurrentConditionControls.Current_Wind.SetText(strtowstr(Wind).c_str());
	CurrentConditionControls.Current_Moon_Image.SetImagePath(strtowstr(MoonPath).c_str());
	CurrentConditionControls.Current_Image.SetImagePath(strtowstr(TypePath).c_str());
	m_Lock.Unlock();

	/////////////////////////////////////////////////////////
	// Set Day Conditions
	/////////////////////////////////////////////////////////
	vector<ForecastItem>::iterator itr;
	int counter = 0;
	for(itr = inf.m_Forecast.begin(); itr != inf.m_Forecast.end(); itr++)
	{
		string HighTemp		= sprintfaA("%d °%s", (*itr).nHighTemp, UnitTemp.c_str());
		string LowTemp		= sprintfaA("%d °%s", (*itr).nLowTemp, UnitTemp.c_str());
		string Sunrise		= (*itr).strSunrise;
		string Sunset		= (*itr).strSunset;
		string DayDate		= sprintfaA("%s - %s", (*itr).strDayofWeek.c_str(), (*itr).strDate.c_str());

		vector<TimeSegment>::iterator itr2;
		bool day = true;
		for(itr2 = (*itr).Segments.begin(); itr2 != (*itr).Segments.end(); itr2++)
		{
			string ForecastCondition	= (*itr2).strForecastCondition;
			string Wind					= sprintfaA("From %s at %d%s", (*itr2).strWindDirection.c_str() ,(*itr2).nWindVelocity ,UnitSpeed.c_str() );
			string PrecipChance			= sprintfaA("%d%%", (*itr2).nChanceofPercip );
			string Humidity				= sprintfaA("%d%%", (*itr2).nHumidity );
			string TypePath				= sprintfaA("%s\\%d.%s", strTypeBasePath.c_str(), (*itr2).nForecastIcon, fileType.c_str() );

			if(day)
			{
				if(strcmp(ForecastCondition.c_str(),"N/A") == 0)//Set Day vals to N/A if they are no longer available
				{
					Wind = "N/A";
					PrecipChance = "N/A";
					Humidity = "N/A";
					HighTemp = "N/A";
					Sunrise = "N/A";
				}

				m_Conditions[counter].Forecast_Day_Conditions.SetText(strtowstr(ForecastCondition).c_str());
				m_Conditions[counter].Forecast_Day_Wind.SetText(strtowstr(Wind).c_str());
				m_Conditions[counter].Forecast_Day_Precip.SetText(strtowstr(PrecipChance).c_str());
				m_Conditions[counter].Forecast_Day_Humidity.SetText(strtowstr(Humidity).c_str());
				m_Conditions[counter].Forecast_Day_Image.SetImagePath(strtowstr(TypePath).c_str());

				day = false;
			}
			else
			{
				m_Conditions[counter].Forecast_Night_Conditions.SetText(strtowstr(ForecastCondition).c_str());
				m_Conditions[counter].Forecast_Night_Wind.SetText(strtowstr(Wind).c_str());
				m_Conditions[counter].Forecast_Night_Precip.SetText(strtowstr(PrecipChance).c_str());
				m_Conditions[counter].Forecast_Night_Humidity.SetText(strtowstr(Humidity).c_str());
				m_Conditions[counter].Forecast_Night_Image.SetImagePath(strtowstr(TypePath).c_str());
			}

		}

		m_Conditions[counter].Forecast_Day_Sunrise.SetText(strtowstr(Sunrise).c_str());
		m_Conditions[counter].Forecast_Day_High.SetText(strtowstr(HighTemp).c_str());
		m_Conditions[counter].Forecast_Night_Sunset.SetText(strtowstr(Sunset).c_str());
		m_Conditions[counter].Forecast_Night_Low.SetText(strtowstr(LowTemp).c_str());
		m_Conditions[counter].Forecast_Title .SetText(strtowstr(DayDate).c_str());

		counter++;
	}

	// Weather Found Show 5-Day Forecast
	int FrameStart, FrameEnd;
	this->FindNamedFrame(strtowstr("WeatherFound").c_str(), &FrameStart);
	this->FindNamedFrame(strtowstr("EndWeatherFound").c_str(), &FrameEnd);
	if(FrameStart != -1 && FrameEnd != -1)
	{
		this->PlayTimeline(FrameStart, FrameStart, FrameEnd, false, false);
	}
}

void CScnWeather::InitializeChildren( )
{
	// Get Current Condition Children
	GetChildById( L"TitleText", &m_TitleText);
	GetChildById( L"Tab2", &m_CurrentConditions );
	m_CurrentConditions.GetChildById( L"Current_Temp", &CurrentConditionControls.Current_Temp );
	m_CurrentConditions.GetChildById( L"Current_DewPoint", &CurrentConditionControls.Current_DewPoint );
	m_CurrentConditions.GetChildById( L"Current_Feels", &CurrentConditionControls.Current_Feels );
	m_CurrentConditions.GetChildById( L"Current_Humidity", &CurrentConditionControls.Current_Humidity );
	m_CurrentConditions.GetChildById( L"Current_Image", &CurrentConditionControls.Current_Image );
	m_CurrentConditions.GetChildById( L"Current_Moon_Image", &CurrentConditionControls.Current_Moon_Image );
	m_CurrentConditions.GetChildById( L"Current_Moon_Type", &CurrentConditionControls.Current_Moon_Type );
	m_CurrentConditions.GetChildById( L"Current_Pressure", &CurrentConditionControls.Current_Pressure );
	m_CurrentConditions.GetChildById( L"Current_Sunrise", &CurrentConditionControls.Current_Sunrise );
	m_CurrentConditions.GetChildById( L"Current_Sunset", &CurrentConditionControls.Current_Sunset );
	m_CurrentConditions.GetChildById( L"Current_UV", &CurrentConditionControls.Current_UV );
	m_CurrentConditions.GetChildById( L"Current_Visibility", &CurrentConditionControls.Current_Visibility );
	m_CurrentConditions.GetChildById( L"Current_Wind", &CurrentConditionControls.Current_Wind );

	GetChildById( L"Tab3", &m_ForecastDays[0] );
	GetChildById( L"Tab4", &m_ForecastDays[1] );
	GetChildById( L"Tab5", &m_ForecastDays[2] );
	GetChildById( L"Tab6", &m_ForecastDays[3] );
	GetChildById( L"Tab7", &m_ForecastDays[4] );

	for(int i = 0; i < 5; i++)
	{
		m_ForecastDays[i].GetChildById( L"Forecast_Day_Conditions", &m_Conditions[i].Forecast_Day_Conditions );
		m_ForecastDays[i].GetChildById( L"Forecast_Title", &m_Conditions[i].Forecast_Title );
		m_ForecastDays[i].GetChildById( L"Forecast_Day_High", &m_Conditions[i].Forecast_Day_High );
		m_ForecastDays[i].GetChildById( L"Forecast_Day_Wind", &m_Conditions[i].Forecast_Day_Wind );
		m_ForecastDays[i].GetChildById( L"Forecast_Day_Precip", &m_Conditions[i].Forecast_Day_Precip );
		m_ForecastDays[i].GetChildById( L"Forecast_Day_Humidity", &m_Conditions[i].Forecast_Day_Humidity );
		m_ForecastDays[i].GetChildById( L"Forecast_Day_Sunrise", &m_Conditions[i].Forecast_Day_Sunrise );
		m_ForecastDays[i].GetChildById( L"Forecast_Day_Image", &m_Conditions[i].Forecast_Day_Image );
		m_ForecastDays[i].GetChildById( L"Forecast_Night_Conditions", &m_Conditions[i].Forecast_Night_Conditions );
		m_ForecastDays[i].GetChildById( L"Forecast_Night_Low", &m_Conditions[i].Forecast_Night_Low );
		m_ForecastDays[i].GetChildById( L"Forecast_Night_Wind", &m_Conditions[i].Forecast_Night_Wind );
		m_ForecastDays[i].GetChildById( L"Forecast_Night_Precip", &m_Conditions[i].Forecast_Night_Precip );
		m_ForecastDays[i].GetChildById( L"Forecast_Night_Humidity", &m_Conditions[i].Forecast_Night_Humidity );
		m_ForecastDays[i].GetChildById( L"Forecast_Night_Sunset", &m_Conditions[i].Forecast_Night_Sunset );
		m_ForecastDays[i].GetChildById( L"Forecast_Night_Image", &m_Conditions[i].Forecast_Night_Image );
	}
}
