#pragma once
#include "stdafx.h"
#include "../../../Tools/Generic/Tools.h"
#include "../../../Tools/Debug/Debug.h"
#include "ConfigurableScene.h"
#include "../../../Tools/Managers/Skin/SkinManager.h"

ConfigurableScene::ConfigurableScene()
{	
	//	DebugMsg("Configurable Scene constructor");
}
void ConfigurableScene::SendSettingsValueToDebug()
{	
	for(std::map<string,string>::const_iterator it = m_SettingsMap->begin(); it != m_SettingsMap->end(); ++it)
	{
		DebugMsg("ConfigurableScn","%s : %s",it->first.c_str(),it->second.c_str());
	}
}

string ConfigurableScene::GetSetting(string SettingName,string defaultValue)
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

void ConfigurableScene::LoadSettings()
{
	
	CXuiTextElement m_Settings;
	HRESULT hr = GetChildById( L"Settings", &m_Settings );
	if(hr==S_OK)
	{
		//DebugMsg("Loading settings");
		  
		string setting =wstrtostr( m_Settings.GetText());
		//DebugMsg("%s",setting.c_str());
		vector<string> settings;
	
		
		StringSplit(setting,",",&settings);
		//DebugMsg("%d - %d",settings.size(),setting.npos);
		for(unsigned int x = 0;x<settings.size();x++)
		{
			string currentSetting = settings.at(x);
			//  DebugMsg("Parsing %s",currentSetting.c_str());
			vector<string> keyVal;
			StringSplit(currentSetting,":",&keyVal);
			string key = keyVal.at(0);
			string val = keyVal.at(1);
			m_SettingsMap->insert(std::map<string,string>::value_type(key,val));
		}
	 }
	 else
	 {
		 //DebugMsg("No settings label");
	 }
	 SettingsLoaded();
}
void ConfigurableScene::LoadSettings( string sceneName, CXuiElement scene )
{
	DebugMsg("ConfigurableScene","loading Configurable Scene");
	string instanceName;

	//DebugMsg("%sskin.xml does  exist",temppath.c_str());

	DebugMsg("ConfigurableScene","Loading Skin Settings");
	DebugMsg("ConfigurableScene","<%s>", sceneName.c_str());

	LPCWSTR ID;
	scene.GetId(&ID);
	string check = wstrtostr(ID);
	map<string, string> * temp = new map<string, string>;
	
	DebugMsg("ConfigurableScene", "<Instance id = %s >", check.c_str());
	HRESULT hr = SkinManager::getInstance().getSettingMap(temp, sceneName, check);
	if(hr==S_OK)
		DebugMsg("ConfigurableScene","Came Back Ok");
	
	if (temp->find("none") == temp->end())
	{
		m_SettingsMap = temp;
	}

	SettingsLoaded();
}