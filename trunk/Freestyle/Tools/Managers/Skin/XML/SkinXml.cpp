#pragma once
#include "stdafx.h"

#include "../../../Generic/tools.h"
#include "../../../Generic/xboxtools.h"
#include "../../../Debug/Debug.h"
#include "SkinXml.h"

HRESULT  SkinXml::StartDocument()
{

	m_CurrentElementData = L"";
	INCount = 0;
	return S_OK;
}

VOID SkinXml::Error( HRESULT hError, CONST CHAR *strMessage )
{

} 

HRESULT  SkinXml::CDATABegin( )
{
	
	return S_OK;
}

HRESULT  SkinXml::CDATAData( CONST WCHAR *strCDATA, UINT CDATALen, BOOL bMore )
{
	
	LPCWSTR newString = (LPCWSTR)malloc((CDATALen*2)+2);
	memset((VOID*)newString,0,(CDATALen*2)+2);
	memcpy((VOID*)newString,strCDATA,(CDATALen*2));
	m_StringList.push_back(newString);
	m_CurrentElementData  = sprintfaW(L"%s%s",m_CurrentElementData.c_str(),newString);
	return S_OK;
}

HRESULT  SkinXml::CDATAEnd( )
{
	
	return S_OK;
}

HRESULT  SkinXml::EndDocument()
{
	int nSize = m_StringList.size();
	for(int x=0;x<nSize;x++)
	{
		LPCWSTR theStr = (LPCWSTR)m_StringList.at(x);
		delete theStr;
	}

	//nSize = m_Items.size();
	//nSize = m_Forecast.size();
	//int nItem = m_Forecast.at(3).nDayIndex;
	//string strDay = m_Forecast.at(3).strDayofWeek;
	//DebugMsg("SkinXml", "m_Forecast Size:  %d", nSize);

	return S_OK;
}

HRESULT  SkinXml::ElementBegin( const WCHAR* strName, UINT NameLen,const ATG::XMLAttribute* pAttributes, UINT NumAttributes )
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

	if(strcmp(make_lowercaseA(currentElement).c_str(), "--skin--fonts--font") == 0)
	{
		FontInfo tmpInfo;

		tmpInfo.FontFile = m_Attributes["file"].c_str();
		tmpInfo.FontName = m_Attributes["name"].c_str();
		if (strcmp(make_lowercaseA(m_Attributes["default"]).c_str(), "true")==0)
		{
			tmpInfo.FontDefault = true;
		} else {
			tmpInfo.FontDefault = false;
		}

		Fonts.push_back(tmpInfo);
	}
	getInstanceNames();

	return S_OK;
}

HRESULT  SkinXml::ElementContent( const WCHAR *strData, UINT DataLen, BOOL More )
{
	LPCWSTR newString = (LPCWSTR)malloc((DataLen*2)+2);
	memset((VOID*)newString,0,(DataLen*2)+2);
	memcpy((VOID*)newString,strData,(DataLen*2));
	m_StringList.push_back(newString);
	
	m_CurrentElementData  = sprintfaW(L"%s%s",m_CurrentElementData.c_str(),newString);
	return S_OK;
}

HRESULT  SkinXml::ElementEnd( const WCHAR *strName, UINT NameLen )
{
	//string currentElement = GetFullCurrentElementName();
	parseSkinXml();

	m_CurrentElementData = L"";
	m_CurrentElementName.pop_back();
	
	m_Attributes.clear();
	return S_OK;
}

void SkinXml::parseSkinXml()
{
	string currentElement = GetFullCurrentElementName();
	string strData = "";
	vector<string> Settings;
	StringSplit(currentElement, "--", &Settings);
	unsigned int size = Settings.size()-1;
	if (size == 2)
	{
		if (strcmp(make_lowercaseA(Settings.at(size)).c_str(), "displayfps")==0)
		{
			strData = wstrtostr(m_CurrentElementData).c_str();
			if (strcmp(make_lowercaseA(strData).c_str(),"true") == 0)
			{
				DisplayFPS = true;
			} else {
				DisplayFPS = false;
			}
		} else if (strcmp(make_lowercaseA(Settings.at(size)).c_str(), "displayprojecttitle")==0)
		{
			strData = wstrtostr(m_CurrentElementData).c_str();
			if (strcmp(make_lowercaseA(strData).c_str(),"true") == 0)
			{
				DisplayProjectTitle = true;
			} else {
				DisplayProjectTitle = false;
			}
		} else if (strcmp(make_lowercaseA(Settings.at(size)).c_str(), "displaycpurender")==0)
		{	
			strData = wstrtostr(m_CurrentElementData).c_str();
			if (strcmp(make_lowercaseA(strData).c_str(),"true") == 0)
			{
				DisplayCPURender = true;
			} else {
				DisplayCPURender = false;
			}
		} else if (strcmp(make_lowercaseA(Settings.at(size)).c_str(), "skin")==0)
		{
			strData = wstrtostr(m_CurrentElementData).c_str();
			SkinName = strData;
			MaxVer = atoi(m_Attributes["Max"].c_str());
			MinVer = atoi(m_Attributes["Min"].c_str());
		}  else if (strcmp(make_lowercaseA(Settings.at(size)).c_str(), "author")==0)
		{
			strData = wstrtostr(m_CurrentElementData).c_str();
			SkinAuthor = strData;
		} else if (strcmp(make_lowercaseA(Settings.at(size)).c_str(), "version")==0)
		{
			strData = wstrtostr(m_CurrentElementData).c_str();
			SkinVersion = strData;
		} else if (strcmp(make_lowercaseA(Settings.at(size)).c_str(), "displayfreemem")==0)
		{
			strData = wstrtostr(m_CurrentElementData).c_str();
			if (strcmp(make_lowercaseA(strData).c_str(),"true") == 0)
			{
				DisplayFreeMEM = true;
			} else {
				DisplayFreeMEM = false;
			}
		} else {
			if (strcmp(make_lowercaseA(Settings.at(size)).c_str(), "font")!=0)
			{
				m_XmlInfo.m_SceneInfo.Element[Settings.at(size).c_str()] = m_XmlInfo.m_InstanceInfo;
				m_XmlInfo.m_InstanceInfo.Element.clear();
			}
		}
	} else if (size == 3)
	{
		m_XmlInfo.m_InstanceInfo.Element[Instance.c_str()] = m_XmlInfo.m_SettingMap;
		m_XmlInfo.m_SettingMap.Element.clear();
		Instance = "";
	} else if (size == 4)
	{
		strData = wstrtostr(m_CurrentElementData).c_str();
		m_XmlInfo.m_SettingMap.Element[Settings.at(size).c_str()] = strData;
	}
}

void SkinXml::getInstanceNames()
{
	string currentElement = GetFullCurrentElementName();
	string strData = "";
	
	vector<string> Settings;

	StringSplit(currentElement, "--", &Settings);
	unsigned int size = Settings.size()-1;
	if (strcmp(Settings.at(size).c_str(), "Instance")==0)
	{
		Instance = m_Attributes["id"].c_str();
	}
}