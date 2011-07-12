#pragma once

#include "../../Generic/tools.h"
#include "../../HTTP/HttpItem.h"
#include "../../HTTP/HTTPDownloader.h"
#include "XML/WeatherSearchXML.h"


class iWeatherLocation{
public :
	virtual void setWeatherLocation(vector<LocationSearch> m_currentLocation) = 0;
};

// Weather Channel Base URL / keys
static const char baseSearchPath[] = "http://xoap.weather.com/search/search?where=";
static const char baseWeatherPath[] = "http://xoap.weather.com/weather/local/";
static const char baseParnerIDKey[] = "&link=xoap&prod=xoap&par=1231212544&key=ddf6587ef4972198";
static const char baseRequestTag[] = "?cc=*&dayf=5";

class WeatherSearch : public iHttpItemRequester
{
public:
	static WeatherSearch& getInstance()
	{
		static WeatherSearch singleton;
		return singleton;
	}

	void add(iWeatherLocation& ref);
	void remove(iWeatherLocation& ref);

	void UpdateInfo(string m_search);
	void DownloadCompleted(HttpItem* itm);
	float GetFarenheight(float Celcius);
	float GetCelcius(float Farenheight);

private:	
	WeatherSearch() {}
	~WeatherSearch() {}

	std::map<iWeatherLocation* const,iWeatherLocation* const> _observers;
	typedef std::map<iWeatherLocation* const, iWeatherLocation* const> item;

	void setWeatherLocation(vector<LocationSearch> m_currentLocation);

	DWORD m_lastTickUpdated;
	string weatherLocation;

	WeatherSearch(const WeatherSearch&);		            // Prevent copy-construction
	WeatherSearch& operator=(const WeatherSearch&);		    // Prevent assignment

};