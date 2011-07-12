#pragma once
#include <AtgXmlParser.h>
#include "../../../Generic/tools.h"
#include "../../../Generic/xboxtools.h"
#include "../../../Debug/Debug.h"


//static const char baseImageURLAlt[] = "http://i.imwx.com/v.20100202.0/web/common/wxicons/52/";
class TimeSegment
{
public :
	int nForecastIcon;
	string strForecastCondition;
	int nWindVelocity;
	string strWindDirection;
	int nChanceofPercip;
	int nHumidity;
};

class CurrentLocaleInfo
{
public:
	string strLocale;
	string strUnitTemp;
	string strUnitDistance;
	string strUnitSpeed;
	string strUnitPressure;
	string strUnitAmount;
};

class CurrentItemGlobal
{
public:
	string strLocation;
	string strTime;
	double dLatitude;
	double dLongitude;
	string strSunrise;
	string strSunste;
	string strTimeZone;
};

class CurrentItem
{
public:
	int nCurrentTemp;
	int nFeelsLikeTemp;
	string strCurrentCondition;
	int nConditionIcon;
	double nBarPressure;
	string strBarChange;
	int nWindVelocity;
	string strWindDirection;
	int nHumidity;
	double nVisibility;
	int nUvIndex;
	string strUvDesc;
	int nDewPoint;
	int nMoonPhaseIcon;
	string strMoonPhase;

};

class ForecastItem
{
public :
	int nDayIndex;
	string strDayofWeek;
	string strDate;
	int nHighTemp;
	int nLowTemp;
	string strSunrise;
	string strSunset;
	vector<TimeSegment> Segments;
};

class WeatherXML : public ATG::ISAXCallback
{
private :

	//SAX Implementation
	vector<LPCWSTR> m_CurrentElementName;
	vector<LPCWSTR> m_StringList;
	map<string,string> m_Attributes;

	//Grouping of subitems
	vector<ForecastItem> m_Forecast;
	ForecastItem * currentForecast;
	TimeSegment * currentTimeSegment;
	CurrentItem currentConditions;
	CurrentItemGlobal currentGlobalConditions;
	CurrentLocaleInfo currentLocaleInfo;
	bool m_bKeyIsValid;

	void parseForecastConditions();
	void parseTimeSegments();
	void parseCurrentConditions();
	void parseGlobalVariables();
	void parseLocaleVariables();

	wstring m_CurrentElementData;

	wstring GetLastElement()
	{
		wstring result = L"";
		if(m_CurrentElementName.size()>0)
		{
			result = m_CurrentElementName.at(m_CurrentElementName.size() - 1);
		}
		return result;
	}
	string GetFullCurrentElementName()
	{
		string result = "";
		int nSize = m_CurrentElementName.size();
		for(int x=0;x<nSize;x++)
		{
			
			result = sprintfa("%s--%s",result.c_str(),wstrtostr(m_CurrentElementName.at(x)).c_str());
		}
	
		return result;
	}

public :	
	HRESULT  StartDocument();
    HRESULT  EndDocument();
	HRESULT  CDATABegin( );
    HRESULT  CDATAData( CONST WCHAR *strCDATA, UINT CDATALen, BOOL bMore );
    HRESULT  CDATAEnd( );

	vector<ForecastItem> getForecastItem()
	{
		return m_Forecast;
	}

	CurrentItem getCurrentConditions()
	{
		return currentConditions;
	}

	CurrentItemGlobal getCurrentGlobalConditions()
	{
		return currentGlobalConditions;
	}

	CurrentLocaleInfo getCurrentLocaleInfo()
	{
		return currentLocaleInfo;
	}

	bool isKeyValid()
	{
		return m_bKeyIsValid;
	}

    VOID Error( HRESULT hError, CONST CHAR *strMessage ) ;

	HRESULT  ElementContent( CONST WCHAR *strData, UINT DataLen, BOOL More );
	HRESULT  ElementBegin( CONST WCHAR* strName, UINT NameLen,CONST ATG::XMLAttribute* pAttributes, UINT NumAttributes );
    HRESULT  ElementEnd( CONST WCHAR *strName, UINT NameLen );

};