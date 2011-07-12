#pragma once
#include <AtgXmlParser.h>
#include "../../../Generic/tools.h"
#include "../../../Generic/xboxtools.h"
#include "../../../Debug/Debug.h"


//static const char baseImageURLAlt[] = "http://i.imwx.com/v.20100202.0/web/common/wxicons/52/";

class LocationSearch
{
public:
	string strName;
	string strID;
};

class WeatherSearchXML : public ATG::ISAXCallback
{
private :

	//SAX Implementation
	vector<LPCWSTR> m_CurrentElementName;
	vector<LPCWSTR> m_StringList;
	map<string,string> m_Attributes;

	//Grouping of subitems
	vector<LocationSearch> m_currentLocation;
	LocationSearch * currentLocation;

	void parseLocationConditions();

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

	vector<LocationSearch> getLocationItem()
	{
		return m_currentLocation;
	}

    VOID Error( HRESULT hError, CONST CHAR *strMessage ) ;

	HRESULT  ElementContent( CONST WCHAR *strData, UINT DataLen, BOOL More );
	HRESULT  ElementBegin( CONST WCHAR* strName, UINT NameLen,CONST ATG::XMLAttribute* pAttributes, UINT NumAttributes );
    HRESULT  ElementEnd( CONST WCHAR *strName, UINT NameLen );

};