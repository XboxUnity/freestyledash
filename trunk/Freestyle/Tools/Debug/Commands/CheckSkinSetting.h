#include "stdafx.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../Settings/Settings.h"
#include "../../Managers/Skin/SkinManager.h"
#include "../../FileBrowser/FileBrowser.h"



class CheckSkinXMLSetting:public DebugCommand
{
public :

	CheckSkinXMLSetting()
	{
		m_CommandName = "CheckSkinXMLSetting";
	}

	void Perform(string parameters)
	{
		HRESULT hr;
		string sceneName = "";
		string instanceID = "";
		int startpos = parameters.find(" ");

		map<string, string> m_Settings;

		if(startpos != -1)
		{
			sceneName = parameters.substr(0,startpos);
			instanceID = parameters.substr(startpos +1);

			DebugMsg("CheckSkinXMLSetting","SceneName %s   InstanceID %s", sceneName.c_str(), instanceID.c_str());

			hr = SkinManager::getInstance().getSettingMap(&m_Settings, sceneName, instanceID);
		}
		else
		{
			DebugMsg("CheckSkinXMLSetting","Please Enter a SceneName and Instance Id    ex. CheckSkinXMLSetting ScnProfile ProfileScene");
		}

		if(hr == S_OK)
			DebugMsg("CheckSkinXMLSetting","Got Setting List - Size %d", m_Settings.size());
		else
			DebugMsg("CheckSkinXMLSetting","List Failed");

		map<string, string>::iterator itr;

		for(itr = m_Settings.begin(); itr != m_Settings.end(); itr++)
		{
			DebugMsg("CheckSkinXMLSetting", "GAMELISTSETTINGS:  %s - %s", (*itr).first.c_str(),(*itr).second.c_str());
		}
	}
};