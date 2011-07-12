#pragma once

#include <AtgXmlParser.h>
#include "../../../Generic/tools.h"
#include "../../../Generic/xboxtools.h"
#include "../../../Debug/Debug.h"

typedef struct _RSS_FEED_ITEM {
	string szRssTitle;
	string szRssDate;
	string szRssDescription;
} RSS_FEED_ITEM;

class RssXML : public ATG::ISAXCallback
{
private :



	//SAX
	vector<LPCWSTR> m_CurrentElementName;
	vector<LPCWSTR> m_StringList;
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
		//DebugMsg("MarketPlaceXML","%d",m_CurrentElementName.size());
		int nSize = m_CurrentElementName.size();
		for(int x=0;x<nSize;x++)
		{
			
			result = sprintfa("%s--%s",result.c_str(),wstrtostr(m_CurrentElementName.at(x)).c_str());
	
		//	result = result+("|");
		//	DebugMsg("MarketPlaceXML","%s",wstrtostr(m_CurrentElementName.at(x)).c_str());

			//result = result + (wstrtostr(m_CurrentElementName.at(x)));
		}
	
		return result;
	}
public :
	
	bool gotTitle;

	
	string m_Title;
	vector<RSS_FEED_ITEM> m_rssItems;
	RSS_FEED_ITEM * m_rssTemp;

	HRESULT  StartDocument();
    HRESULT  EndDocument();
	HRESULT  CDATABegin( );
    HRESULT  CDATAData( CONST WCHAR *strCDATA, UINT CDATALen, BOOL bMore );
    HRESULT  CDATAEnd( );

    VOID Error( HRESULT hError, CONST CHAR *strMessage ) ;

	HRESULT  ElementContent( CONST WCHAR *strData, UINT DataLen, BOOL More );
	HRESULT  ElementBegin( CONST WCHAR* strName, UINT NameLen,CONST ATG::XMLAttribute* pAttributes, UINT NumAttributes );
    HRESULT  ElementEnd( CONST WCHAR *strName, UINT NameLen );

};