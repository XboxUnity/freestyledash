#pragma once
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../Debug/Debug.h"

class MarketPlaceASX : public ATG::ISAXCallback
{
private :
	//SAX
	vector<LPCWSTR> m_CurrentElementName;
	vector<LPCWSTR> m_StringList;
	map<string,string> m_Attributes;
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

	string szASXVersion;
	bool hasValidVersion;
	string szWMVUrl;
	bool hasWMVUrl;


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