#pragma once

#include "../../../Tools/Generic/Tools.h"
#include "../../../Tools/Debug/Debug.h"

class ConfigurableList :public CXuiListImpl
{
private:
	std::map<string,string> * m_SettingsMap;
	
public :
	ConfigurableList();
	void SendSettingsValueToDebug();
	void LoadSettings();
	void LoadSettings(string sceneName, CXuiElement scene);

	string GetSetting(string SettingName,string defaultValue);

	virtual void SettingsLoaded()
	{

	}
};