#include "stdafx.h"

#include <AtgXmlParser.h>
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../Debug/Debug.h"
#include "MarketPlaceOfferXML.h"

HRESULT  MarketPlaceOfferXML::StartDocument()
{
	gotReleaseDate = false;
	m_CurrentElementData = L"";
	return S_OK;
}

VOID MarketPlaceOfferXML::Error( HRESULT hError, CONST CHAR *strMessage )
{
} 

HRESULT  MarketPlaceOfferXML::CDATABegin( ){return S_OK;}
HRESULT  MarketPlaceOfferXML::CDATAData( CONST WCHAR *strCDATA, UINT CDATALen, BOOL bMore ){return S_OK;}
HRESULT  MarketPlaceOfferXML::CDATAEnd( ){return S_OK;}
HRESULT  MarketPlaceOfferXML::EndDocument()
{
	int nSize = m_StringList.size();
	for(int x=0;x<nSize;x++)
	{
		LPCWSTR theStr = (LPCWSTR)m_StringList.at(x);
		delete theStr;
	}
	return S_OK;
}

HRESULT  MarketPlaceOfferXML::ElementBegin( const WCHAR* strName, UINT NameLen,const ATG::XMLAttribute* pAttributes, UINT NumAttributes )
{
	LPCWSTR newString = (LPCWSTR)malloc((NameLen*2)+2);
	memset((VOID*)newString,0,(NameLen*2)+2);
	memcpy((VOID*)newString,strName,(NameLen*2));
	
	m_StringList.push_back(newString);
	m_CurrentElementName.push_back(newString);
	return S_OK;
}

HRESULT  MarketPlaceOfferXML::ElementContent( const WCHAR *strData, UINT DataLen, BOOL More )
{
	LPCWSTR newString = (LPCWSTR)malloc((DataLen*2)+2);
	memset((VOID*)newString,0,(DataLen*2)+2);
	memcpy((VOID*)newString,strData,(DataLen*2));
	m_StringList.push_back(newString);
	m_CurrentElementData  = sprintfaW(L"%s%s",m_CurrentElementData.c_str(),newString);
	//DebugMsg("MarketPlaceXML","Got data");
	return S_OK;
}

HRESULT MarketPlaceOfferXML::ElementEnd( const WCHAR *strName, UINT NameLen )
{
	string currentElement = GetFullCurrentElementName();

	if(strcmp(currentElement.c_str(),"--feed--entry--live:offer--live:offerInstances--live:offerInstance--live:startDateTime")==0)
	{
		m_ReleaseDate = m_CurrentElementData;
		m_ReleaseDate = m_ReleaseDate.substr(0,10);		// chop the time off of the datestamp
		if (m_ReleaseDate != L"") gotReleaseDate = true;
		//DebugMsg("MarketPlaceXMl","releaseDate : %s",m_ReleaseDate.c_str());
	}
	
	m_CurrentElementData = L"";
	m_CurrentElementName.pop_back();
	return S_OK;
}
