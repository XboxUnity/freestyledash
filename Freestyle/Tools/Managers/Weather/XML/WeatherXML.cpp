#pragma once
#include "stdafx.h"

#include "../../../Generic/tools.h"
#include "../../../Generic/xboxtools.h"
#include "../../../Debug/Debug.h"
#include "WeatherXML.h"

HRESULT  WeatherXML::StartDocument()
{

	m_CurrentElementData = L"";
	m_bKeyIsValid = true;
	return S_OK;
}

VOID WeatherXML::Error( HRESULT hError, CONST CHAR *strMessage )
{

} 

HRESULT  WeatherXML::CDATABegin( )
{
	
	return S_OK;
}

HRESULT  WeatherXML::CDATAData( CONST WCHAR *strCDATA, UINT CDATALen, BOOL bMore )
{
	
	LPCWSTR newString = (LPCWSTR)malloc((CDATALen*2)+2);
	memset((VOID*)newString,0,(CDATALen*2)+2);
	memcpy((VOID*)newString,strCDATA,(CDATALen*2));
	m_StringList.push_back(newString);
	m_CurrentElementData  = sprintfaW(L"%s%s",m_CurrentElementData.c_str(),newString);
	return S_OK;
}

HRESULT  WeatherXML::CDATAEnd( )
{
	
	return S_OK;
}

HRESULT  WeatherXML::EndDocument()
{
	int nSize = m_StringList.size();
	for(int x=0;x<nSize;x++)
	{
		LPCWSTR theStr = (LPCWSTR)m_StringList.at(x);
		delete theStr;
	}
	return S_OK;
}

HRESULT  WeatherXML::ElementBegin( const WCHAR* strName, UINT NameLen,const ATG::XMLAttribute* pAttributes, UINT NumAttributes )
{
	LPCWSTR newString = (LPCWSTR)malloc((NameLen*2)+2);
	memset((VOID*)newString,0,(NameLen*2)+2);
	memcpy((VOID*)newString,strName,(NameLen*2));
	m_StringList.push_back(newString);
	m_CurrentElementName.push_back(newString);

	// Pass attribute names and values to a map that self deletes when the element has ended.
	int nCount = NumAttributes;

	for(int i = 0; i < nCount; i++)
	{
		LPCWSTR attrName = (LPCWSTR)malloc((pAttributes[i].NameLen*2)+2);
		memset((VOID*)attrName,0,(pAttributes[i].NameLen*2)+2);
		memcpy((VOID*)attrName,pAttributes[i].strName,(pAttributes[i].NameLen*2));

		LPCWSTR attrValue = (LPCWSTR)malloc((pAttributes[i].ValueLen*2)+2);
		memset((VOID*)attrValue,0,(pAttributes[i].ValueLen*2)+2);
		memcpy((VOID*)attrValue,pAttributes[i].strValue,(pAttributes[i].ValueLen*2));
		m_Attributes.insert(std::map<string,string>::value_type(wstrtostr(attrName),wstrtostr(attrValue)));

		delete attrName;
		delete attrValue;
	}
	string currentElement = GetFullCurrentElementName();

	if(strcmp(currentElement.c_str(), "--weather--dayf--day") == 0)
	{
		currentForecast = new ForecastItem;

		currentForecast->nDayIndex = atoi(m_Attributes["d"].c_str());
		currentForecast->strDayofWeek = m_Attributes["t"];
		currentForecast->strDate = m_Attributes["dt"];
	}

	if(strcmp(currentElement.c_str(), "--weather--dayf--day--part") == 0)
	{
		currentTimeSegment = new TimeSegment;
	}

	if(strcmp(currentElement.c_str(), "--error--err") == 0)
	{
		m_bKeyIsValid = false;
	}

	return S_OK;
}

HRESULT  WeatherXML::ElementContent( const WCHAR *strData, UINT DataLen, BOOL More )
{
	LPCWSTR newString = (LPCWSTR)malloc((DataLen*2)+2);
	memset((VOID*)newString,0,(DataLen*2)+2);
	memcpy((VOID*)newString,strData,(DataLen*2));
	m_StringList.push_back(newString);
	
	m_CurrentElementData  = sprintfaW(L"%s%s",m_CurrentElementData.c_str(),newString);
	return S_OK;
}

HRESULT  WeatherXML::ElementEnd( const WCHAR *strName, UINT NameLen )
{
	parseCurrentConditions();
	parseForecastConditions();
	parseTimeSegments();
	parseGlobalVariables();
	parseLocaleVariables();

	m_CurrentElementData = L"";
	m_CurrentElementName.pop_back();
	
	m_Attributes.clear();
	return S_OK;
}

void WeatherXML::parseGlobalVariables()
{
	string currentElement = GetFullCurrentElementName();
	string strData = "";

	if(strcmp(currentElement.c_str(), "--weather--loc--dnam") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentGlobalConditions.strLocation = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--loc--tm") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentGlobalConditions.strTime = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--loc--lat") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentGlobalConditions.dLatitude = atof(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--loc--lon") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentGlobalConditions.dLongitude = atof(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--loc--sunr") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentGlobalConditions.strSunrise = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--loc--suns") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentGlobalConditions.strSunste = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--loc--zone") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentGlobalConditions.strTimeZone = strData;
	}
}

void WeatherXML::parseLocaleVariables()
{
	string currentElement = GetFullCurrentElementName();
	string strData = "";

	if(strcmp(currentElement.c_str(), "--weather--head--locale") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentLocaleInfo.strLocale = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--head--ut") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentLocaleInfo.strUnitTemp = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--head--ud") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentLocaleInfo.strUnitDistance = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--head--us") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentLocaleInfo.strUnitSpeed = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--head--up") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentLocaleInfo.strUnitPressure = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--head--ur") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentLocaleInfo.strUnitAmount = strData;
	}
}

void WeatherXML::parseCurrentConditions()
{
	string currentElement = GetFullCurrentElementName();
	string strData = "";

	if(strcmp(currentElement.c_str(), "--weather--cc--tmp") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.nCurrentTemp = atoi(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--flik") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.nFeelsLikeTemp = atoi(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--t") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.strCurrentCondition = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--icon") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.nConditionIcon = atoi(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--bar--r") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.nBarPressure = atof(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--bar--d") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.strBarChange = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--wind--s") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.nWindVelocity = atoi(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--wind--t") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.strWindDirection = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--hmid") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.nHumidity = atoi(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--vis") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.nVisibility = atof(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--uv--i") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.nUvIndex = atoi(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--uv--t") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.strUvDesc = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--dewp") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.nDewPoint = atoi(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--moon--icon") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.nMoonPhaseIcon = atoi(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--cc--moon--t") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		currentConditions.strMoonPhase = strData;
	}
}

void WeatherXML::parseForecastConditions()
{
	string currentElement = GetFullCurrentElementName();

	int nValue = 0;
	string strData = "";

	if(strcmp(currentElement.c_str(), "--weather--dayf--day--hi") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);

		if(strcmp(strData.c_str(), "N/A") != 0)
			nValue = atoi(strData.c_str());
		else
			nValue = -1;

		currentForecast->nHighTemp = nValue;
	}

	if(strcmp(currentElement.c_str(), "--weather--dayf--day--low") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);

		if(strcmp(strData.c_str(), "N/A") != 0)
			nValue = atoi(strData.c_str());
		else
			nValue = -1;

		currentForecast->nLowTemp = nValue;
	}

	if(strcmp(currentElement.c_str(), "--weather--dayf--day--sunr") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);

		currentForecast->strSunrise = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--dayf--day--suns") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);

		currentForecast->strSunset = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--dayf--day") == 0)
	{
		m_Forecast.push_back(*currentForecast);
		delete currentForecast;
	}
}

void WeatherXML::parseTimeSegments()
{
	string currentElement = GetFullCurrentElementName();
	int nValue = 0;
	string strData = "";

	if(strcmp(currentElement.c_str(), "--weather--dayf--day--part--icon") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);

		nValue = atoi(strData.c_str());

		currentTimeSegment->nForecastIcon = nValue;
	}

	if(strcmp(currentElement.c_str(), "--weather--dayf--day--part--t") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);

		currentTimeSegment->strForecastCondition = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--dayf--day--part--wind--s") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);
		
		currentTimeSegment->nWindVelocity = atoi(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--dayf--day--part--wind--t") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);

		currentTimeSegment->strWindDirection = strData;
	}

	if(strcmp(currentElement.c_str(), "--weather--dayf--day--part--ppcp") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);

		currentTimeSegment->nChanceofPercip = atoi(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--dayf--day--part--hmid") == 0)
	{
		strData = wstrtostr(m_CurrentElementData);

		currentTimeSegment->nHumidity = atoi(strData.c_str());
	}

	if(strcmp(currentElement.c_str(), "--weather--dayf--day--part") == 0)
	{
		currentForecast->Segments.push_back(*currentTimeSegment);
		delete currentTimeSegment;
	}
}