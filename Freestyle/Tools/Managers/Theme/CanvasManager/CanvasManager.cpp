#include "stdafx.h"

#include "CanvasManager.h"
#include "../../../Settings/Settings.h"
#include "../../../Managers/Skin/SkinManager.h"
#include "../../../Debug/Debug.h"

using namespace std;

CanvasManager::CanvasManager()
{
	// Constructor for Visual Manager

	// Load Internal Maps from XML
	LoadBkgAFromXML();
	LoadBkgBFromXML();

	// Load saved settings from XML
	LoadSettingsFromFile();
}

/*Class Construction ToDo

---------Functions-----------
-SaveCurrentSettings to XML
*/

void CanvasManager::add(iThemeCanvasObserver& ref)
{
	_observers.insert(item::value_type(&ref,&ref));
}
void CanvasManager::remove(iThemeCanvasObserver& ref)
{
	_observers.erase(&ref);
}


void CanvasManager::_notifyBackgroundAChange( void )
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "CanvasManager: Background Change Notified");

	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
		it->first->handleBackgroundAChange();
}

void CanvasManager::_notifyBackgroundBChange( void )
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "CanvasManager: Background Change Notified");

	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
		it->first->handleBackgroundBChange();
}

void CanvasManager::_notifyPrefixChange( void )
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "CanvasManager: Prefix Change Notified");

	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
		it->first->handlePrefixChange();
}

void CanvasManager::_notifyAnimationChange( void )
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "Canvas Manager: Animation Change Notified");

	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
		it->first->handleAnimationChange();
}

BackgroundElement CanvasManager::getBackgroundA( int nIndex, VMOptions nFlag = VM_OPTION_INDEX)
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "Canvas Manager: Requesting BackgroundA[%d] with Flag: '%d'", nIndex, nFlag);

	int nSize = getBackgroundACount();

	if(nIndex > nSize - 1)
		return m_activeBkgA;

	switch (nFlag)
	{
	case VM_OPTION_INDEX:
		return m_BackgroundAList[nIndex];

	case VM_OPTION_LOADED:
		return m_loadedBkgA;

	case VM_OPTION_ACTIVE:
		break;
	}
	
	return m_activeBkgA;
}

BackgroundElement CanvasManager::getBackgroundB(int nIndex, VMOptions nFlag = VM_OPTION_INDEX)
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "Canvas Manager: Requesting BackgroundB[%d] with Flag: '%d'", nIndex, nFlag);

	int nSize = getBackgroundBCount();

	if(nIndex > nSize - 1)
		return m_activeBkgB;

	switch (nFlag)
	{
	case VM_OPTION_INDEX:
		return m_BackgroundBList[nIndex];

	case VM_OPTION_LOADED:
		return m_loadedBkgB;

	case VM_OPTION_ACTIVE:
		break;
	}
	
	return m_activeBkgB;
}

PrefixElement CanvasManager::getPrefix(int nIndex, VMOptions nFlag = VM_OPTION_INDEX)
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "Canvas Manager: Requesting Prefix[%d] with Flag: '%d'", nIndex, nFlag);

	int nSize = getPrefixCount();

	if(nIndex > nSize - 1)
		return m_activePrefix;

	switch (nFlag)
	{
	case VM_OPTION_INDEX:
		return m_PrefixList[nIndex];

	case VM_OPTION_LOADED:
		return m_loadedPrefix;

	case VM_OPTION_ACTIVE:
		break;
	}
	
	return m_activePrefix;
}

AnimationElement CanvasManager::getAnimation(VMOptions nFlag = VM_OPTION_ACTIVE)
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "Canvas Manager: Requesting Animation with Flag: '%d'", nFlag);

	switch (nFlag)
	{
	case VM_OPTION_LOADED:
		return m_loadedAnim;

	case VM_OPTION_ACTIVE:
		break;
	}
	
	return m_activeAnim;
}

void CanvasManager::setBackgroundA(BackgroundElement bkgElement, VMOptions nFlag = VM_OPTION_ACTIVE)
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "Canvas Manager: Storing BackgroundA with Flag: '%d'", nFlag);

	switch (nFlag)
	{
	case VM_OPTION_LOADED:
		m_loadedBkgA = bkgElement;
		break;

	case VM_OPTION_ACTIVE:
		m_activeBkgA = bkgElement;
		_notifyBackgroundAChange();
		break;
	}
}

void CanvasManager::setBackgroundA(int nElementIndex, VMOptions nFlag = VM_OPTION_ACTIVE)
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "Canvas Manager: Storing BackgroundA with Flag: '%d'", nFlag);

	int nSize = getBackgroundACount();

	if(nElementIndex > nSize - 1)
		return;

	switch (nFlag)
	{
	case VM_OPTION_LOADED:
		m_loadedBkgA = m_BackgroundAList[nElementIndex];
		break;
	case VM_OPTION_ACTIVE:
		m_activeBkgA = m_BackgroundAList[nElementIndex];
		_notifyBackgroundAChange();
		break;
	}

}

void CanvasManager::setBackgroundB(BackgroundElement bkgElement, VMOptions nFlag = VM_OPTION_ACTIVE)
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "Canvas Manager: Storing BackgroundB with Flag: '%d'", nFlag);

	switch (nFlag)
	{
	case VM_OPTION_LOADED:
		m_loadedBkgB = bkgElement;
		break;

	case VM_OPTION_ACTIVE:
		m_activeBkgB = bkgElement;
		_notifyBackgroundBChange();
		break;
	}


}

void CanvasManager::setBackgroundB(int nElementIndex, VMOptions nFlag = VM_OPTION_ACTIVE)
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "Canvas Manager: Storing BackgroundB with Flag: '%d'", nFlag);

	int nSize = getBackgroundBCount();

	if(nElementIndex > nSize - 1)
		return;

	switch (nFlag)
	{
	case VM_OPTION_LOADED:
		m_loadedBkgB = m_BackgroundBList[nElementIndex];
		break;
	case VM_OPTION_ACTIVE:
		m_activeBkgB = m_BackgroundBList[nElementIndex];
		_notifyBackgroundBChange();
		break;
	}
}

void CanvasManager::setPrefix(PrefixElement prefixElement, VMOptions nFlag = VM_OPTION_ACTIVE)
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "Canvas Manager: Storing Prefix with Flag: '%d'", nFlag);

	switch (nFlag)
	{
	case VM_OPTION_LOADED:
		m_loadedPrefix = prefixElement;
		break;

	case VM_OPTION_ACTIVE:
		m_activePrefix = prefixElement;
		_notifyPrefixChange();
		break;
	}


}

void CanvasManager::setPrefix(int nElementIndex, VMOptions nFlag = VM_OPTION_ACTIVE)
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "Canvas Manager: Storing Prefix with Flag: '%d'", nFlag);

	int nSize = getPrefixCount();

	if(nElementIndex > nSize - 1)
		return;

	switch (nFlag)
	{
	case VM_OPTION_LOADED:
		m_loadedPrefix = m_PrefixList[nElementIndex];
		break;
	case VM_OPTION_ACTIVE:
		m_activePrefix = m_PrefixList[nElementIndex];
		_notifyPrefixChange();
		break;
	}


}

void CanvasManager::setAnimation(AnimationElement animElement, VMOptions nFlag = VM_OPTION_ACTIVE)
{
	if(nDebugFlag != SHOW_DEBUG)
		DebugMsg("CanvasManager", "Canvas Manager: Storing Animation Elements with Flag: '%d'", nFlag);

	switch (nFlag)
	{
	case VM_OPTION_LOADED:
		m_loadedAnim = animElement;
		break;

	case VM_OPTION_ACTIVE:
		m_activeAnim = animElement;
		_notifyAnimationChange();
		break;
	}


}

void CanvasManager::setTemporaryBkgA(int nIndex)
{
	int nSize = getBackgroundACount();

	if(nIndex > nSize - 1)
		return;

	m_activeBkgA = m_BackgroundAList[nIndex];

	_notifyBackgroundAChange();
}

void CanvasManager::setTemporaryBkgA(BackgroundElement bkgElement)
{
	m_activeBkgA = bkgElement;

	_notifyBackgroundAChange();
}

void CanvasManager::setTemporaryBkgA(string path)
{
	BackgroundElement bkgItem;
	bkgItem.strBackgroundPath = path;

	m_activeBkgA = bkgItem;
	
	_notifyBackgroundAChange();
}

void CanvasManager::setTemporaryBkgB(int nIndex)
{
	int nSize = getBackgroundBCount();

	if(nIndex > nSize - 1)
		return;

	m_activeBkgB = m_BackgroundBList[nIndex];

	_notifyBackgroundBChange();
}

void CanvasManager::setTemporaryBkgB(BackgroundElement bkgElement)
{
	m_activeBkgB = bkgElement;

	_notifyBackgroundBChange();
}

void CanvasManager::setTemporaryBkgB(string path)
{
	BackgroundElement bkgItem;
	bkgItem.strBackgroundPath = path;

	m_activeBkgB = bkgItem;
	
	_notifyBackgroundBChange();
}

void CanvasManager::revertBkgA( )
{
	m_activeBkgA = m_loadedBkgA;
	
	_notifyBackgroundAChange();
}

void CanvasManager::revertBkgB( )
{
	m_activeBkgB = m_loadedBkgB;

	_notifyBackgroundBChange();
}

void CanvasManager::LoadBkgAFromXML()
{
	string tempPath;

	tempPath = SkinManager::getInstance().getCurrentxmlPath();

	if(!FileExistsA(tempPath + "XML\\BkgListA.xml"))
	{
		if(nDebugFlag != SHOW_DEBUG)
			DebugMsg("CanvasManager", "%BkgListA.xml does not exist",tempPath.c_str());
	}
	else
	{
		string file = tempPath + "XML\\BkgListA.xml";
		XMLReader * xml = LoadConfigFile(file);
		
		BackgroundElement bkgItem;
		string backgroundName;
		int bkgCounter = 0;

		while (!EndAttribute(xml, "backgroundlist"))
		{
			if (StartAttribute(xml, "Background"))
			{
				backgroundName = xml->getAttributeValue("name");
				
				if(nDebugFlag != SHOW_DEBUG)
					DebugMsg("CanvasManager", "Loading Settings for %s", backgroundName.c_str());
				
				bkgItem.strBackgroundName = backgroundName;

				string key, val;
				while (!EndAttribute(xml, "Background"))
				{
					key = xml->getNodeName();
					key = xml->getNodeData();
					
					if (StartAttribute(xml, key.c_str()))
					{
						val = xml->getSubNodeData();
						if(nDebugFlag != SHOW_DEBUG)
							DebugMsg("CanvasManager", "Setting: %s, Value: %s", key.c_str(), val.c_str());

						if(strcmp(key.c_str(), "FILEPATH")==0)
							bkgItem.strBackgroundPath = val;

						if(strcmp(key.c_str(), "PREVIEWPATH")==0)
							bkgItem.strPreviewPath = val;

						bkgItem.nIndex = bkgCounter;
					}
				}
				m_BackgroundAList.insert(map<int,BackgroundElement>::value_type(bkgCounter, bkgItem));
				bkgCounter++;
			}
		}
	} 
}

void CanvasManager::LoadBkgBFromXML()
{
	string tempPath;
	
	tempPath= SkinManager::getInstance().getCurrentxmlPath();
	
	if(!FileExistsA(tempPath + "XML\\BkgListB.xml"))
	{
		if(nDebugFlag != SHOW_DEBUG)
			DebugMsg("CanvasManager", "%BkgListB.xml does not exist",tempPath.c_str());
	}
	else
	{
		string file = tempPath + "XML\\BkgListB.xml";
		XMLReader * xml = LoadConfigFile(file);
		
		BackgroundElement bkgItem;
		string backgroundName;
		int bkgCounter = 0;

		while (!EndAttribute(xml, "backgroundlist"))
		{
			if (StartAttribute(xml, "Background"))
			{
				backgroundName = xml->getAttributeValue("name");
				
				if(nDebugFlag != SHOW_DEBUG)
					DebugMsg("CanvasManager", "Loading Settings for %s", backgroundName.c_str());
				
				bkgItem.strBackgroundName = backgroundName;

				string key, val;
				while (!EndAttribute(xml, "Background"))
				{
					key = xml->getNodeName();
					key = xml->getNodeData();
					
					if (StartAttribute(xml, key.c_str()))
					{
						val = xml->getSubNodeData();
						if(nDebugFlag != SHOW_DEBUG)
							DebugMsg("CanvasManager", "Setting: %s, Value: %s", key.c_str(), val.c_str());

						if(strcmp(key.c_str(), "FILEPATH")==0)
							bkgItem.strBackgroundPath = val;

						if(strcmp(key.c_str(), "PREVIEWPATH")==0)
							bkgItem.strPreviewPath = val;

						bkgItem.nIndex = bkgCounter;
					}
				}
				m_BackgroundBList.insert(map<int,BackgroundElement>::value_type(bkgCounter, bkgItem));
				bkgCounter++;
			}
		}
	} 
}

void CanvasManager::LoadSettingsFromFile()
{
	string tempPath;

	tempPath= SkinManager::getInstance().getCurrentxmlPath();

	if(!FileExistsA(tempPath + "Settings\\SkinSettings.xml"))
	{
		if(nDebugFlag != SHOW_DEBUG)
			DebugMsg("CanvasManager", "%SkinSettings.xml does not exist",tempPath.c_str());
		return;
	}

	string file = tempPath + "Settings\\SkinSettings.xml";
	XMLReader * xml = LoadConfigFile(file);
	
	string bkgIndexA, bkgIndexB;
	string prefixIndex;
	string strAnim[MAX_ANIM_ITEMS];
	string strBuffer; 

	string key,val;

	while (!EndAttribute(xml, "canvasmanager"))
	{
		key = xml->getNodeName();
		key = xml->getNodeData();
			
		if (StartAttribute(xml, key.c_str()))
		{
			val = xml->getSubNodeData();
			if(nDebugFlag != SHOW_DEBUG)
				DebugMsg("CanvasManager", "Setting: %s, Value: %s", key.c_str(), val.c_str());

			if(strcmp(key.c_str(), "LOADBKGA")==0)
				bkgIndexA = val;
			
			if(strcmp(key.c_str(), "LOADBKGB")==0)
				bkgIndexB = val;

			if(strcmp(key.c_str(), "LOADPREFIX")==0)
				prefixIndex = val;

			for(int i = 0; i < MAX_ANIM_ITEMS; i++)
			{
				strBuffer = sprintfaA("LOADANIM%d", (i+1));
			
				if(strcmp(key.c_str(), strBuffer.c_str())==0)
					strAnim[i] = val;
			}
		}
	}

	bool bAnim[4] = {true, true, true, true};

	for(int i = 0; i < MAX_ANIM_ITEMS; i++)
	{
		if(strcmp(strAnim[i].c_str(), "TRUE") == 0)
			bAnim[i] = true;
		else
			bAnim[i] = false;
	}

	int nIndex[3] = {-1, -1, -1};

	nIndex[0] = atoi(bkgIndexA.c_str());
	nIndex[1] = atoi(bkgIndexB.c_str());
	nIndex[2] = atoi(prefixIndex.c_str());

	m_loadedBkgA = getBackgroundA(nIndex[0], VM_OPTION_INDEX);
	m_loadedBkgB = getBackgroundB(nIndex[1], VM_OPTION_INDEX);
	m_loadedPrefix = getPrefix(nIndex[2], VM_OPTION_INDEX);

	for(int i = 0; i < MAX_ANIM_ITEMS; i++)
	{
		m_loadedAnim.animItem[i] = bAnim[i];
	}
}

void CanvasManager::SaveSettingsToFile()
{

	string strBkgAIndex = sprintfaA("%d", getBackgroundA(0, VM_OPTION_ACTIVE).nIndex);
	string strBkgBIndex = sprintfaA("%d", getBackgroundB(0, VM_OPTION_ACTIVE).nIndex);

	string strAnimState[MAX_ANIM_ITEMS];
	string strAnimName[MAX_ANIM_ITEMS];

	SkinManager::getInstance().WriteToXML("Settings\\SkinSettings.xml", "LOADBKGA", strBkgAIndex);
	SkinManager::getInstance().WriteToXML("Settings\\SkinSettings.xml", "LOADBKGB", strBkgBIndex);
	
	AnimationElement animItem = getAnimation(VM_OPTION_ACTIVE);

	for(int i = 0; i < MAX_ANIM_ITEMS; i++)
	{
		
		if(animItem.animItem[i])
			strAnimState[i] = "TRUE";
		else
			strAnimState[i] = "FALSE";

		strAnimName[i] = sprintfaA("LOADANIM%d", (i+1));

		SkinManager::getInstance().WriteToXML("Settings\\SkinSettings.xml", strAnimName[i], strAnimState[i]);
	}

}