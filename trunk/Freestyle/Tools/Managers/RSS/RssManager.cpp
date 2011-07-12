#include "stdafx.h"

#include "RssManager.h"
#include "../../Settings/Settings.h"
#include "../../Managers/Skin/SkinManager.h"
#include "../../Debug/Debug.h"

using namespace std;

RssManager::RssManager()
{
	// Load saved settings from XML
	LoadSettingsFromXML();
}

void RssManager::LoadSettingsFromXML()
{
	string tempPath;

	tempPath = SkinManager::getInstance().getCurrentxmlPath();

	if(!FileExistsA(tempPath + "Settings\\SkinSettings.xml"))
	{
		if(nDebugFlag != SHOW_DEBUG)
			DebugMsg("RssManager", "%sskin.xml does not exist",tempPath.c_str());
		return;
	}

	string file = tempPath + "Settings\\SkinSettings.xml";
	XMLReader * xml = LoadConfigFile(file);

	string key,val;

	while (!EndAttribute(xml, "rss"))
	{
		key = xml->getNodeName();
		key = xml->getNodeData();
			
		if (StartAttribute(xml, key.c_str()))
		{
			val = xml->getSubNodeData();
			if(nDebugFlag != SHOW_DEBUG)
				DebugMsg("RssManager", "Setting: %s, Value: %s", key.c_str(), val.c_str());

			if(strcmp(key.c_str(), "FeedTitleColor")==0)
				feedTitleColor = val;
			
			if(strcmp(key.c_str(), "ItemTitleColor")==0)
				itemTitleColor = val;

			if(strcmp(key.c_str(), "ItemBodyColor")==0)
				itemBodyColor = val;

			if(strcmp(key.c_str(), "ItemDateColor")==0)
				itemDateColor = val;

			if(strcmp(key.c_str(), "FeedTitleFontSize")==0)
				feedTitleFontSize = val;

			if(strcmp(key.c_str(), "ItemFontSize")==0)
				itemFontSize = val;

		}
	}
}