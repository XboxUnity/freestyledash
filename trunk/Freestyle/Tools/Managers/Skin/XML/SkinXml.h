#pragma once
#include <AtgXmlParser.h>
#include "../../../Generic/tools.h"
#include "../../../Generic/xboxtools.h"
#include "../../../Debug/Debug.h"


//static const char baseImageURLAlt[] = "http://i.imwx.com/v.20100202.0/web/common/wxicons/52/";

struct SettingMap {
	map<string, string> Element;
	map<string, string>::iterator Count;
};
struct InstanceInfo {
	map<string, SettingMap> Element;
	map<string, SettingMap>::iterator Count;
};
struct SceneInfo {
	map<string, InstanceInfo> Element;
	map<string, InstanceInfo>::iterator Count;
};
struct XmlInfo {
	SettingMap m_SettingMap;
	InstanceInfo m_InstanceInfo;
	SceneInfo m_SceneInfo;
};

struct FontInfo {
	string FontName;
	string FontFile;
	bool FontDefault;
};


class SkinXml : public ATG::ISAXCallback
{
private :

	//SAX Implementation
	vector<LPCWSTR> m_CurrentElementName;
	vector<LPCWSTR> m_StringList;
	map<string,string> m_Attributes;

	InstanceInfo * instance;
	SettingMap * setting;

	//Grouping of subitems
	vector<string> InstanceName;
	string Instance;
	unsigned int INCount;

	void parseSkinXml();
	void getInstanceNames();

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

	vector<FontInfo> Fonts;
	string SkinName;
	string SkinAuthor;
	string SkinVersion;
	int MaxVer;
	int MinVer;
	bool DisplayFPS;
	bool DisplayProjectTitle;
	bool DisplayCPURender;
	bool DisplayFreeMEM;
	XmlInfo m_XmlInfo;


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