#include "stdafx.h"

#include <AtgXmlParser.h>
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../Debug/Debug.h"
#include "MarketPlaceXML.h"

HRESULT  MarketPlaceXML::StartDocument()
{
	gotBanner = false;
	gotBackground = false;
	gotBoxArt = false;
	gotScreenshots = false;
	gotPreview = false;
	gotDescription = false;
	gotTitle = false;
	gotPublisher = false;
	gotDeveloper = false;
	gotGenre = false;
	gotRating = false;
	gotRaters = false;
	m_CurrentElementData = L"";
	m_CurrentImageMediaId = "";
	m_CurrentImageMediaInstanceId = "";
	m_GenreList.clear();
	nInternalSSCounter = 0;
	return S_OK;
}
VOID MarketPlaceXML::Error( HRESULT hError, CONST CHAR *strMessage ){

} 
HRESULT  MarketPlaceXML::CDATABegin( ){return S_OK;}
HRESULT  MarketPlaceXML::CDATAData( CONST WCHAR *strCDATA, UINT CDATALen, BOOL bMore ){return S_OK;}
HRESULT  MarketPlaceXML::CDATAEnd( ){return S_OK;}
HRESULT  MarketPlaceXML::EndDocument()
{
	int nSize = m_StringList.size();
	for(int x=0;x<nSize;x++)
	{
		LPCWSTR theStr = (LPCWSTR)m_StringList.at(x);
		delete theStr;
	}
	return S_OK;
}
HRESULT  MarketPlaceXML::ElementBegin( const WCHAR* strName, UINT NameLen,const ATG::XMLAttribute* pAttributes, UINT NumAttributes )
{
	LPCWSTR newString = (LPCWSTR)malloc((NameLen*2)+2);
	memset((VOID*)newString,0,(NameLen*2)+2);
	memcpy((VOID*)newString,strName,(NameLen*2));
	
	m_StringList.push_back(newString);
	m_CurrentElementName.push_back(newString);
//	DebugMsg("MarketPlaceXML","Got element %d %d %s",NameLen,elementName.size(),wstrtostr(elementName).c_str());

	return S_OK;
}
HRESULT  MarketPlaceXML::ElementContent( const WCHAR *strData, UINT DataLen, BOOL More )
{
	LPCWSTR newString = (LPCWSTR)malloc((DataLen*2)+2);
	memset((VOID*)newString,0,(DataLen*2)+2);
	memcpy((VOID*)newString,strData,(DataLen*2));
	m_StringList.push_back(newString);
	m_CurrentElementData  = sprintfaW(L"%s%s",m_CurrentElementData.c_str(),newString);
	//DebugMsg("MarketPlaceXML","Got data");
	return S_OK;
}
HRESULT  MarketPlaceXML::ElementEnd( const WCHAR *strName, UINT NameLen )
{

	string currentElement = GetFullCurrentElementName();

	//if(strcmp(currentElement.c_str(),"--feed--entry--live:media--live:description")==0)
	if(strcmp(currentElement.c_str(),"--feed--entry--live:media--live:reducedDescription")==0)
	{
		string d = wstrtostr(m_CurrentElementData);
		m_Description = FromUtf8(d);
		if (m_Description != L"") gotDescription = true;
	}
	if(strcmp(currentElement.c_str(),"--feed--entry--live:media--live:fullTitle")==0)
	{
		string t = wstrtostr(m_CurrentElementData);
		m_Title = FromUtf8(t);
		if (m_Title != L"") gotTitle = true;
		//DebugMsg("MarketPlaceXMl","Title : %s",m_Title.c_str());
	}
	if(strcmp(currentElement.c_str(),"--feed--entry--live:media--live:publisher")==0)
	{
		string p = wstrtostr(m_CurrentElementData);
		m_Publisher = FromUtf8(p);
		if (m_Publisher != L"") gotPublisher = true;
		//DebugMsg("MarketPlaceXMl","Publisher : %s",m_Publisher.c_str());
	}
	if(strcmp(currentElement.c_str(),"--feed--entry--live:media--live:developer")==0)
	{
		string d = wstrtostr(m_CurrentElementData);
		m_Developer = FromUtf8(d);
		if (m_Developer != L"") gotDeveloper = true;
		//DebugMsg("MarketPlaceXMl","Developer : %s",m_Developer.c_str());
	}
	if(strcmp(currentElement.c_str(),"--feed--entry--live:categories--live:category--live:categoryId")==0)
	{
		int id = atoi(wstrtostr(m_CurrentElementData).c_str());
		string genre = getGenreFromId(id);
		
		if(genre.length() > 0) 
			m_GenreList.push_back(genre);
		
		if (m_GenreList.size() > 0) gotGenre = true;
	}
	if(strcmp(currentElement.c_str(),"--feed--entry--live:media--live:ratingAggregate")==0)
	{
		m_Rating = m_CurrentElementData;
		if (m_Rating != L"") gotRating = true;
		//DebugMsg("MarketPlaceXMl","RatingAggregate : %s",wstrtostr(m_CurrentElementData).c_str());
	}
	if(strcmp(currentElement.c_str(),"--feed--entry--live:media--live:numberOfRatings")==0)
	{
		m_Raters = m_CurrentElementData;
		if (m_Raters != L"") gotRaters = true;
		//DebugMsg("MarketPlaceXMl","numberOfRatings : %s",wstrtostr(m_CurrentElementData).c_str());
	}
	if(currentElement.find("live:image--live:imageMediaId") != string::npos)
	{
		m_CurrentImageMediaId = wstrtostr(m_CurrentElementData);
	//	if (m_CustomGenre != "") gotGenre = true;
	//	DebugMsg("MarketPlaceXMl","CustomGenre : %s",m_CustomGenre.c_str());
	}
	if(currentElement.find("live:image--live:imageMediaInstanceId") != string::npos)
	{
		m_CurrentImageMediaInstanceId = wstrtostr(m_CurrentElementData);
	//	if (m_CustomGenre != "") gotGenre = true;
	//	DebugMsg("MarketPlaceXMl","CustomGenre : %s",m_CustomGenre.c_str());
	}
	if(strcmp(currentElement.c_str(),"--feed--entry--live:images--live:image--live:fileUrl")==0)
	{

		// convert the current url to a string for parsing
		string m_urlData = wstrtostr(m_CurrentElementData);

		if(m_urlData.find("banner.png") != string::npos) {
			m_BannerUrl = wstrtostr(m_CurrentElementData);
			if (m_BannerUrl != "") gotBanner = true;
		}
		if(m_urlData.find("boxartlg.jpg") != string::npos) {
			m_BoxartUrl = wstrtostr(m_CurrentElementData);
			if (m_BoxartUrl != "") gotBackground = true;
		}

		if(m_urlData.find("background.jpg") != string::npos) {
			m_BackgroundUrl = wstrtostr(m_CurrentElementData);
			if (m_BackgroundUrl != "") gotBoxArt = true;
		}

		// Legacy Support - This is left in here to support any xmls received that have
		// not gone through teh transition in the xbox marketplace
		if(strpos(m_CurrentImageMediaId.c_str(),"4000-8000-000000008004")!= -1)
		{
			m_BannerUrl = wstrtostr(m_CurrentElementData);
			if (m_BannerUrl != "") gotBanner = true;
		}
		if(strpos(m_CurrentImageMediaId.c_str(),"4002-8000-000000000003")!= -1)
		{
			m_BackgroundUrl = wstrtostr(m_CurrentElementData);
			if (m_BackgroundUrl != "") gotBackground = true;
		}
		if(strpos(m_CurrentImageMediaId.c_str(),"4002-8000-000000000002")!= -1)
		{
			m_BoxartUrl = wstrtostr(m_CurrentElementData);
			if (m_BoxartUrl != "") gotBoxArt = true;
		}
	}
	
	if(strcmp(currentElement.c_str(),"--feed--entry--live:previewInstances--live:videoPreviewInstance--live:files--live:file--live:fileUrl")==0)
	{
		if(!gotPreview)
		{
			m_PreviewUrls = wstrtostr(m_CurrentElementData);
			if (m_PreviewUrls != "") gotPreview = true;
		}
		m_CurrentImageMediaId = wstrtostr(m_CurrentElementData);
	
	}
	
	if(strcmp(currentElement.c_str(),"--feed--entry--live:slideShows--live:slideShow--live:image--live:fileUrl")==0)
	{
		
		string screenshotUrl = wstrtostr(m_CurrentElementData);
		if(strpos(m_CurrentImageMediaId.c_str(), "0000-4003-8000-") != -1)
		{
			// Increment our screenshot counter		
			nInternalSSCounter++;
			// If the counter is passed the first one, we can start including them
			if(nInternalSSCounter > 1)
			{
				gotScreenshots = true;
				m_ScreenshotsUrl.push_back(screenshotUrl);
			}
		} else {
			if(screenshotUrl.find("66acd000-77fe-1000-9115-d802") != string::npos)
			{
				gotScreenshots = true;
				m_ScreenshotsUrl.push_back(screenshotUrl);
			}
		}
	}
	m_CurrentElementData = L"";
	m_CurrentElementName.pop_back();
	return S_OK;
}
	/*switch(currentElement.c_str())
	{
	default :
		//Unhandled yet
		break;
	}
	wstring lastElement = GetLastElement();
	if(wcscmp(lastElement.c_str(),strName) == 0)
	{
		m_CurrentElementName.pop_back();
	}
//	DebugMsg("MarketPlaceXML","Element End");
	m_CurrentElementData = L"";
	return S_OK;
}*/


string MarketPlaceXML::getGenreFromId( int nId )
{
	string genre = "";
	switch (nId)
	{
	case 3001: 
		genre = "Other"; 
		break;
	case 3002: 
		genre = "Action & Adventure"; 
		break;
	case 3005: 
		genre = "Family"; 
		break;
	case 3006: 
		genre = "Fighting"; 
		break;
	case 3007:
		genre = "Music";
		break;
	case 3008:
		genre = "Platformer";
		break;
	case 3009:
		genre = "Racing & Flying";
		break;
	case 3010:
		genre = "Role Playing";
		break;
	case 3011:
		genre = "Shooter";
		break;
	case 3012:
		genre = "Strategy & Simulation";
		break;
	case 3013:
		genre = "Sports & Recreation";
		break;
	case 3018:
		genre = "Card & Board";
		break;
	case 3019:
		genre = "Classics";
		break;
	case 3022:
		genre = "Puzzle & Trivia";
		break;
	};

	return genre;
}