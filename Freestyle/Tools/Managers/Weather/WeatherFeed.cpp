#pragma once
#include "stdafx.h"

#include "../../Generic/tools.h"
#include "../../Settings/Settings.h"
#include "../../HTTP/HttpItem.h"
#include "../../HTTP/HTTPDownloader.h"
#include "WeatherFeed.h"

void WeatherFeed::UpdateInfo(int errorIndex)
{
	weatherLocation = SETTINGS::getInstance().getWeatherLocation();
	if(strcmp(weatherLocation.c_str(), "undefined")!=0)
	{
		int nIndex;

		do //Check that u dont try to use the same invalid key again
		{
			nIndex = rand() % nMaxAPIInfo;
		}
		while(nIndex == errorIndex);
		
		string weatherAPIpath = sprintfaA(baseWeatherPathAlt_B, m_ApiInfo[nIndex].m_PartnerID.c_str() , m_ApiInfo[nIndex].m_LicenceKey.c_str() );
		string weatherURL = "";

		if(SETTINGS::getInstance().getWeatherUnits()) //Use Metric Units
		{
			weatherURL = baseWeatherPathAlt_A + weatherLocation + weatherAPIpath + baseWeatherPathAlt_Metric;
		}
		else
		{
			weatherURL = baseWeatherPathAlt_A + weatherLocation + weatherAPIpath;
		}

		HttpItem* itm = HTTPDownloader::getInstance().CreateHTTPItem(weatherURL.c_str());
		itm->setTag("WeatherFeed");
		itm->setTag2((DWORD)nIndex);
		itm->setRequester(this);
		itm->SetPriority(HTTP_PRIORITY_HIGH);
		HTTPDownloader::getInstance().AddToQueue(itm);
	}
}

void WeatherFeed::setWeather( WeatherInfo inf )
{
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
		it->first->setWeather(inf);
}

void WeatherFeed::add(iWeatherDisplayer &ref)
{
	_observers.insert(item::value_type(&ref,&ref));
}

void WeatherFeed::remove(iWeatherDisplayer &ref)
{
	_observers.erase(&ref);
}

void WeatherFeed::DownloadCompleted(HttpItem* itm)
{
	m_lastTickUpdated = GetTickCount();

	DebugMsg("WeatherFeed", "%s", itm->getTag().c_str());
	if(strcmp(itm->getTag().c_str(), "WeatherFeed")==0)
	{
		if(itm->getResponseCode() == 200)
		{
			MemoryBuffer& buffer = itm->GetMemoryBuffer();
			
			if(buffer.GetDataLength() > 0)
			{
				DebugMsg("WeatherFeed", "File Found");
				//Parse XML
				WeatherXML xml;
				ATG::XMLParser parser;
				parser.RegisterSAXCallbackInterface(&xml);
				parser.ParseXMLBuffer((CONST CHAR*)buffer.GetData(), (UINT)buffer.GetDataLength());

				if(xml.isKeyValid())
				{
					WeatherInfo inf;

					inf.currentConditions = xml.getCurrentConditions();
					inf.m_Forecast = xml.getForecastItem();
					inf.currentGlobalConditions = xml.getCurrentGlobalConditions();
					inf.currentLocaleInfo = xml.getCurrentLocaleInfo();

					setWeather(inf);
				}
				else//Key Error Redownload Info
				{
					DebugMsg("WeatherFeed","Key Index %x is Invalid trying new key",itm->getTag2());
					UpdateInfo((int)itm->getTag2());
				}
			}
		}
	}
}

float WeatherFeed::GetFarenheight(float Celcius)
{
	float retVal = 0.0f;

	retVal = ((9.0f/5.0f) * Celcius) + 32.0f;
	return retVal;
}

float WeatherFeed::GetCelcius(float Farenheight)
{
	float retVal = 0.0f;
	
	retVal = ((5.0f/9.0f) * (Farenheight - 32.0f));
	return retVal;
}