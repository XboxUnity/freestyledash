#pragma once
#include "stdafx.h"

#include "../../Generic/tools.h"
#include "../../Settings/Settings.h"
#include "../../HTTP/HttpItem.h"
#include "../../HTTP/HTTPDownloader.h"
#include "WeatherSearch.h"

void WeatherSearch::UpdateInfo(string m_search)
{
	if(strcmp(m_search.c_str(), "undefined")!=0)
	{
		string weatherURL = baseSearchPath + m_search;

		HttpItem* itm = HTTPDownloader::getInstance().CreateHTTPItem(weatherURL.c_str());
		itm->setTag("WeatherLocationFeed");
		itm->setRequester(this);
		itm->SetPriority(HTTP_PRIORITY_HIGH);
		HTTPDownloader::getInstance().AddToQueue(itm);
	}
}

void WeatherSearch::setWeatherLocation( vector<LocationSearch> m_currentLocation )
{
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
		it->first->setWeatherLocation(m_currentLocation);
}

void WeatherSearch::add(iWeatherLocation &ref)
{
	_observers.insert(item::value_type(&ref,&ref));
}

void WeatherSearch::remove(iWeatherLocation &ref)
{
	_observers.erase(&ref);
}

void WeatherSearch::DownloadCompleted(HttpItem* itm)
{
	m_lastTickUpdated = GetTickCount();

	DebugMsg("WeatherLocationFeed", "%s", itm->getTag().c_str());

	if(strcmp(itm->getTag().c_str(), "WeatherLocationFeed")==0)
	{
		if(itm->getResponseCode() == 200)
		{
			MemoryBuffer& buffer = itm->GetMemoryBuffer();

			if(buffer.GetDataLength() > 0)
			{
				DebugMsg("WeatherLocationFeed", "File Found");
				//Parse XML
				WeatherSearchXML xml;
				ATG::XMLParser parser;
				parser.RegisterSAXCallbackInterface(&xml);
				parser.ParseXMLBuffer((CONST CHAR*)buffer.GetData(), (UINT)buffer.GetDataLength());

				setWeatherLocation(xml.getLocationItem());
			}
		}
	}
}