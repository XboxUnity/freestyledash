#pragma once

#include "../../../Generic/Tools.h"
#include "../../../Debug/Debug.h"

class SkinXMLReader
{
private:
	std::map<string,string> * m_SettingsMap;
	
public :
	SkinXMLReader();
	void SendSettingsValueToDebug();
	void LoadSettings(string ClassName, string InstanceID);

	string GetSetting(string SettingName,string defaultValue);

	virtual void SettingsLoaded()
	{

	}
};