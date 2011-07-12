#pragma once
#include "stdafx.h"

#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../Debug/Debug.h"
#include "CoverXML.h"

HRESULT  CoverXML::StartDocument()
{

	m_CurrentElementData = L"";

	return S_OK;
}

VOID CoverXML::Error( HRESULT hError, CONST CHAR *strMessage )
{

} 

HRESULT  CoverXML::CDATABegin( )
{
	
	return S_OK;
}

HRESULT  CoverXML::CDATAData( CONST WCHAR *strCDATA, UINT CDATALen, BOOL bMore )
{
	
	LPCWSTR newString = (LPCWSTR)malloc((CDATALen*2)+2);
	memset((VOID*)newString,0,(CDATALen*2)+2);
	memcpy((VOID*)newString,strCDATA,(CDATALen*2));
	m_StringList.push_back(newString);
	m_CurrentElementData  = sprintfaW(L"%s%s",m_CurrentElementData.c_str(),newString);
	return S_OK;
}

HRESULT  CoverXML::CDATAEnd( )
{
	
	return S_OK;
}

HRESULT  CoverXML::EndDocument()
{
	int nSize = m_StringList.size();
	for(int x=0;x<nSize;x++)
	{
		LPCWSTR theStr = (LPCWSTR)m_StringList.at(x);
		delete theStr;
	}

	return S_OK;
}

HRESULT  CoverXML::ElementBegin( const WCHAR* strName, UINT NameLen,const ATG::XMLAttribute* pAttributes, UINT NumAttributes )
{
	LPCWSTR newString = (LPCWSTR)malloc((NameLen*2)+2);
	memset((VOID*)newString,0,(NameLen*2)+2);
	memcpy((VOID*)newString,strName,(NameLen*2));
	m_StringList.push_back(newString);
	m_CurrentElementName.push_back(newString);

	// Pass attribute names and values to a map that self deletes when the element has ended.
	int nCount = NumAttributes;

	for(int i = 0; i < nCount; i++)
	{
		LPCWSTR attrName = (LPCWSTR)malloc((pAttributes[i].NameLen*2)+2);
		memset((VOID*)attrName,0,(pAttributes[i].NameLen*2)+2);
		memcpy((VOID*)attrName,pAttributes[i].strName,(pAttributes[i].NameLen*2));

		LPCWSTR attrValue = (LPCWSTR)malloc((pAttributes[i].ValueLen*2)+2);
		memset((VOID*)attrValue,0,(pAttributes[i].ValueLen*2)+2);
		memcpy((VOID*)attrValue,pAttributes[i].strValue,(pAttributes[i].ValueLen*2));
		m_Attributes.insert(std::map<string,string>::value_type(wstrtostr(attrName),wstrtostr(attrValue)));

		delete attrName;
		delete attrValue;
	}
	string currentElement = GetFullCurrentElementName();

	if (strcmp(currentElement.c_str(), "--covers--title")==0)
	{
		CoverInfo CI;
		CI.TitleID = m_Attributes["id"];
		CI.Title = m_Attributes["name"];
		CI.MediaID = m_Attributes["mediaid"];
		CI.FileName = m_Attributes["filename"];
		CI.URL = m_Attributes["front"];
		CI.Type = m_Attributes["type"];
		Covers.push_back(CI);
	}
	return S_OK;
}

HRESULT  CoverXML::ElementContent( const WCHAR *strData, UINT DataLen, BOOL More )
{
	LPCWSTR newString = (LPCWSTR)malloc((DataLen*2)+2);
	memset((VOID*)newString,0,(DataLen*2)+2);
	memcpy((VOID*)newString,strData,(DataLen*2));
	m_StringList.push_back(newString);
	
	m_CurrentElementData  = sprintfaW(L"%s%s",m_CurrentElementData.c_str(),newString);
	return S_OK;
}

HRESULT  CoverXML::ElementEnd( const WCHAR *strName, UINT NameLen )
{
	string currentElement = GetFullCurrentElementName();

	m_CurrentElementData = L"";
	m_CurrentElementName.pop_back();
	
	m_Attributes.clear();
	return S_OK;
}