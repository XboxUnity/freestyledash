#pragma once

#include "../../Generic/tools.h"
#include "../../HTTP/HttpItem.h"
#include "../../HTTP/HTTPDownloader.h"
#include "XML/WeatherXML.h"

typedef struct
{
	vector<ForecastItem> m_Forecast;
	CurrentItem currentConditions;
	CurrentItemGlobal currentGlobalConditions;
	CurrentLocaleInfo currentLocaleInfo;
} WeatherInfo;


class iWeatherDisplayer{
public :
	virtual void setWeather(WeatherInfo inf) = 0;
};

// Weather Channel Base URL / keys
static const char baseWeatherPathAlt_A[] = "http://xoap.weather.com/weather/local/";
static const char baseWeatherPathAlt_B[] = "?cc=*&dayf=5&link=xoap&prod=xoap&par=%s&key=%s";
static const char baseWeatherPathAlt_Metric[] = "&unit=m";

static const string partnerID[] = { "1231353582", "1231353053", "1231352490", "1231210803", "1231211492", 
									"1231211776", "1231212046", "1231212275", "1231212544", "1231352127",
									"1231372808", "1231373108", "1231373563", "1231374209", "1231374782",
									"1231375036", "1231375262", "1231376163", "1231376939", "1231377304" };

static const string licenseKey[] = { "d753d431a206eb37", "1942aa4018a283fa", "580f7af3f3ef0b39", "4c0fe87d077d1115", "5aa228a2c7571aec", 
									 "012d70d474028bd6", "6394b3a038821f28", "0d9d4f85c2556ef9", "ddf6587ef4972198", "2da0fdb2e1d2e461",
									 "2197362ad97a40d5", "a54a130a6867f5a6", "28eb7e20cf489dfb", "62b3d27d816d0b6e", "9e17a5d5da9be0b8",
									 "c00b4ea929ec597b", "111af7072b656486", "31afe6fda631e2ae", "7b88e897e1d741bd", "024ffb39fb950b24" };

static const int nMaxAPIInfo = 19;

class WeatherFeed : public iHttpItemRequester
{
public:
	static WeatherFeed& getInstance()
	{
		static WeatherFeed singleton;
		return singleton;
	}

	void add(iWeatherDisplayer& ref);
	void remove(iWeatherDisplayer& ref);

	void UpdateInfo(int errorIndex = -1);
	void DownloadCompleted(HttpItem* itm);
	float GetFarenheight(float Celcius);
	float GetCelcius(float Farenheight);

	struct WeatherAPIInfo
	{
		string m_PartnerID;
		string m_LicenceKey;
	} m_ApiInfo[nMaxAPIInfo + 1];

private:	
	WeatherFeed() 
	{
		//Generate list of api info
		for(int i=0; i < nMaxAPIInfo + 1; i++)
		{
			m_ApiInfo[i].m_PartnerID = partnerID[i];
			m_ApiInfo[i].m_LicenceKey = licenseKey[i];
		}
	}
	~WeatherFeed() {}

	std::map<iWeatherDisplayer* const,iWeatherDisplayer* const> _observers;
	typedef std::map<iWeatherDisplayer* const, iWeatherDisplayer* const> item;

	void setWeather( WeatherInfo inf );

	DWORD m_lastTickUpdated;
	string weatherLocation;

	WeatherFeed(const WeatherFeed&);		            // Prevent copy-construction
	WeatherFeed& operator=(const WeatherFeed&);		    // Prevent assignment

};