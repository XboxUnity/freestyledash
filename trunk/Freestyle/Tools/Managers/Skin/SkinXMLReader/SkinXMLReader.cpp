#pragma once
#include "stdafx.h"
#include "../../../Generic/Tools.h"
#include "../../../Debug/Debug.h"
#include "SkinXMLReader.h"
#include "../SkinManager.h"

SkinXMLReader::SkinXMLReader()
{	
	//	DebugMsg("Configurable Scene constructor");
}
void SkinXMLReader::SendSettingsValueToDebug()
{	
	for(std::map<string,string>::const_iterator it = m_SettingsMap->begin(); it != m_SettingsMap->end(); ++it)
	{
		DebugMsg("SkinXMLReader","%s : %s",it->first.c_str(),it->second.c_str());
	}
}

string SkinXMLReader::GetSetting(string SettingName,string defaultValue)
{
		
	string retVal = "";
		
	for(std::map<string,string>::const_iterator it = m_SettingsMap->begin(); it != m_SettingsMap->end(); ++it)
	{
		
		if(it->first.compare(SettingName) == 0)
		{
			retVal = it->second;
			break;
		}
	}

	if(retVal.length()==0)
	{
		return defaultValue;
	}
	else
	{
		return retVal;
	}
}

void SkinXMLReader::LoadSettings( string ClassName, string InstanceID )
{
	DebugMsg("SkinXMLReader","loading Configurable Scene");
	string instanceName = InstanceID;

	//DebugMsg("%sskin.xml does  exist",temppath.c_str());

	DebugMsg("SkinXMLReader","Loading Skin Settings");
	DebugMsg("SkinXMLReader","<%s>", ClassName.c_str());

	string check = instanceName;
	map<string, string> * temp = new map<string, string>;
	
	DebugMsg("SkinXMLReader", "<Instance id = %s >", check.c_str());

	HRESULT hr = SkinManager::getInstance().getSettingMap(temp, ClassName, check);
	if(hr==S_OK)
		DebugMsg("SkinXMLReader","Came Back Ok");
	
	if (temp->find("none") == temp->end())
	{
		m_SettingsMap = temp;
	}

	SettingsLoaded();
}