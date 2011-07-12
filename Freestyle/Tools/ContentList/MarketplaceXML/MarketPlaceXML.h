#pragma once
//#include <xtl.h>
//#include <direct.h>
#include <AtgXmlParser.h>
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../Debug/Debug.h"
//#include <vector>
class MarketPlaceXML : public ATG::ISAXCallback
{
private :

	string getGenreFromId( int nId );
	int nInternalSSCounter;

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
	bool gotBanner;
	bool gotBackground;
	bool gotBoxArt;
	bool gotScreenshots;
	bool gotPreview;
	bool gotDescription;
	bool gotTitle;
	bool gotPublisher;
	bool gotDeveloper;
	bool gotGenre;
	bool gotRating;
	bool gotRaters;
	string m_CurrentImageMediaId;
	string m_CurrentImageMediaInstanceId;
	wstring m_Description;
	wstring m_Title;
	wstring m_Publisher;
	wstring m_Developer;
	wstring m_CustomGenre;
	wstring m_Rating;
	wstring m_Raters;
	string m_BannerUrl;
	string m_BackgroundUrl;
	string m_BoxartUrl;
	vector<string> m_ScreenshotsUrl;
	vector<string> m_GenreList;
	string m_PreviewUrls;


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