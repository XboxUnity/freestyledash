#include "stdafx.h"


#include "../../../Generic/tools.h"
#include "../../../Generic/xboxtools.h"
#include "../../../Debug/Debug.h"
#include "RssXML.h"

HRESULT  RssXML::StartDocument()
{
	gotTitle = false;
	m_CurrentElementData = L"";
	return S_OK;
}
VOID RssXML::Error( HRESULT hError, CONST CHAR *strMessage ){

} 
HRESULT  RssXML::CDATABegin( ){return S_OK;}
HRESULT  RssXML::CDATAData( CONST WCHAR *strCDATA, UINT CDATALen, BOOL bMore ){
	
	LPCWSTR newString = (LPCWSTR)malloc((CDATALen*2)+2);
	memset((VOID*)newString,0,(CDATALen*2)+2);
	memcpy((VOID*)newString,strCDATA,(CDATALen*2));
	m_StringList.push_back(newString);
	m_CurrentElementData  = sprintfaW(L"%s%s",m_CurrentElementData.c_str(),newString);
	return S_OK;}
HRESULT  RssXML::CDATAEnd( ){return S_OK;}
HRESULT  RssXML::EndDocument()
{
	int nSize = m_StringList.size();
	for(int x=0;x<nSize;x++)
	{
		LPCWSTR theStr = (LPCWSTR)m_StringList.at(x);
		delete theStr;
	}
	return S_OK;
}
HRESULT  RssXML::ElementBegin( const WCHAR* strName, UINT NameLen,const ATG::XMLAttribute* pAttributes, UINT NumAttributes )
{

	LPCWSTR newString = (LPCWSTR)malloc((NameLen*2)+2);
	memset((VOID*)newString,0,(NameLen*2)+2);
	memcpy((VOID*)newString,strName,(NameLen*2));
	
	m_StringList.push_back(newString);
	m_CurrentElementName.push_back(newString);
	if(strcmp(GetFullCurrentElementName().c_str(), "--rss--channel--item")==0)
	{
		m_rssTemp = new RSS_FEED_ITEM;
		m_rssTemp->szRssDate = "N/A";
		m_rssTemp->szRssDescription = "N/A";
		m_rssTemp->szRssTitle = "N/A";
	}
//	DebugMsg("MarketPlaceXML","Got element %d %d %s",NameLen,elementName.size(),wstrtostr(elementName).c_str());

	return S_OK;
}
HRESULT  RssXML::ElementContent( const WCHAR *strData, UINT DataLen, BOOL More )
{
	LPCWSTR newString = (LPCWSTR)malloc((DataLen*2)+2);
	memset((VOID*)newString,0,(DataLen*2)+2);
	memcpy((VOID*)newString,strData,(DataLen*2));
	m_StringList.push_back(newString);
	m_CurrentElementData  = sprintfaW(L"%s%s",m_CurrentElementData.c_str(),newString);
	//DebugMsg("MarketPlaceXML","Got data");
	return S_OK;
}
HRESULT  RssXML::ElementEnd( const WCHAR *strName, UINT NameLen )
{

	string currentElement = GetFullCurrentElementName();

	if(strcmp(currentElement.c_str(),"--rss--channel--item") == 0)
	{
		// This is the end of an item, let's push our data to our vector
		m_rssItems.push_back(*m_rssTemp);
		if(m_rssTemp != NULL)
			delete m_rssTemp;
	}
	if(strcmp(currentElement.c_str(),"--rss--channel--title")==0)
	{
		gotTitle = true;
		m_Title = wstrtostr(m_CurrentElementData);
	}
	if(strcmp(currentElement.c_str(),"--feed--title")==0)
	{
		gotTitle = true;
		m_Title = wstrtostr(m_CurrentElementData);
	}
	if(strcmp(currentElement.c_str(),"--rss--channel--item--title")==0)
	{
		m_rssTemp->szRssTitle = wstrtostr(m_CurrentElementData);
	}
	if(strcmp(currentElement.c_str(),"--rss--channel--item--pubDate")==0)
	{
	
		m_rssTemp->szRssDate = wstrtostr(m_CurrentElementData);
	}
	if(strcmp(currentElement.c_str(),"--rss--channel--item--description")==0)
	{
		m_rssTemp->szRssDescription = wstrtostr(m_CurrentElementData);
	}

	m_CurrentElementData = L"";
	m_CurrentElementName.pop_back();
	return S_OK;
}
