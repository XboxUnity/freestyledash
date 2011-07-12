#pragma once

#include "../../../Tools/Generic/Tools.h"
#include "../../../Tools/Debug/Debug.h"

class ConfigurableScene :public CXuiTabSceneImpl
{
private:
	std::map<string,string> * m_SettingsMap;
	
public :
	ConfigurableScene();
	void SendSettingsValueToDebug();
	void LoadSettings();
	void LoadSettings(string sceneName, CXuiElement scene);

	string GetSetting(string SettingName,string defaultValue);

	virtual void SettingsLoaded()
	{

	}
};