#pragma once
#include "stdafx.h"

#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../Debug/Debug.h"
#include "KaiVectorXML.h"

HRESULT  KaiVectorXML::StartDocument()
{

	m_CurrentElementData = L"";
	return S_OK;
}

VOID KaiVectorXML::Error( HRESULT hError, CONST CHAR *strMessage )
{

} 

HRESULT  KaiVectorXML::CDATABegin( )
{
	
	return S_OK;
}

HRESULT  KaiVectorXML::CDATAData( CONST WCHAR *strCDATA, UINT CDATALen, BOOL bMore )
{
	
	LPCWSTR newString = (LPCWSTR)malloc((CDATALen*2)+2);
	memset((VOID*)newString,0,(CDATALen*2)+2);
	memcpy((VOID*)newString,strCDATA,(CDATALen*2));
	m_StringList.push_back(newString);
	m_CurrentElementData  = sprintfaW(L"%s%s",m_CurrentElementData.c_str(),newString);
	return S_OK;
}

HRESULT  KaiVectorXML::CDATAEnd( )
{
	
	return S_OK;
}

HRESULT  KaiVectorXML::EndDocument()
{
	int nSize = m_StringList.size();
	for(int x=0;x<nSize;x++)
	{
		LPCWSTR theStr = (LPCWSTR)m_StringList.at(x);
		delete theStr;
	}

	return S_OK;
}

HRESULT  KaiVectorXML::ElementBegin( const WCHAR* strName, UINT NameLen,const ATG::XMLAttribute* pAttributes, UINT NumAttributes )
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

	return S_OK;
}

HRESULT  KaiVectorXML::ElementContent( const WCHAR *strData, UINT DataLen, BOOL More )
{
	LPCWSTR newString = (LPCWSTR)malloc((DataLen*2)+2);
	memset((VOID*)newString,0,(DataLen*2)+2);
	memcpy((VOID*)newString,strData,(DataLen*2));
	m_StringList.push_back(newString);
	
	m_CurrentElementData  = sprintfaW(L"%s%s",m_CurrentElementData.c_str(),newString);
	return S_OK;
}

HRESULT  KaiVectorXML::ElementEnd( const WCHAR *strName, UINT NameLen )
{
	string currentElement = GetFullCurrentElementName();
	string strData = "";
	if(strcmp(currentElement.c_str(), "--version") == 0)
	{
		m_nVersion = atoi(wstrtostr(m_CurrentElementData).c_str());
		DebugMsg("KaiVectorXML", "KaiVectorXML Version:  %d", m_nVersion);
	}
	if(strcmp(currentElement.c_str(), "--titles--title--id") == 0)
	{
		dwCurrentTitleId = strtoul(wstrtostr(m_CurrentElementData).c_str(), NULL, 16);
	}
	if(strcmp(currentElement.c_str(), "--titles--title--vector") == 0)
	{
		strCurrentVector = wstrtostr(m_CurrentElementData);
		//strCurrentVector = str_replaceallA(strCurrentVector, "%26", "&");
	}
	if(strcmp(currentElement.c_str(), "--titles--title") == 0)
	{
		mTitleMap->insert(map<DWORD, string>::value_type(dwCurrentTitleId, strCurrentVector));
		mVectorMap->insert(map<string, DWORD>::value_type(strCurrentVector, dwCurrentTitleId));
		dwCurrentTitleId = 0;
		strCurrentVector = "";
	}

	m_CurrentElementData = L"";
	m_CurrentElementName.pop_back();
	
	m_Attributes.clear();
	return S_OK;
}