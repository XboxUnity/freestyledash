#pragma once
#include <stdafx.h>
#include "../../../Settings/Settings.h"
#include "../../../Managers/Skin/SkinManager.h"
#include "../../../Debug/Debug.h"
#include "TabManager.h"
#include "../../../Managers/XZP/XZPManager.h"

using namespace std;

TabManager::TabManager()
{
	doneLoading = false;
	LoadXML_MenuSettings();
	LoadXML_VisualList();
	doneLoading = true;
	m_ActiveMenuMap = m_PreviewMenuMap = m_LoadedMenuMap;

}

void TabManager::add(iThemeTabObserver& ref)
{
	_observers.insert(item::value_type(&ref,&ref));
}
void TabManager::remove(iThemeTabObserver& ref)
{
	_observers.erase(&ref);
}

void TabManager::_notifyContentTabChange( int nParent, int nChild, int nFlag )
{

	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
		it->first->handleContentTabChange(nParent, nChild, nFlag );

}

void TabManager::_notifyMenuRefresh( int nFlag )
{

	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
		it->first->handleMenuRefresh(nFlag);

}

int TabManager::getParentCount( int nFlag )
{
	int nSize = 0;

	switch (nFlag)
	{
	case TM_MENU_LOADED:
		nSize = m_LoadedMenuMap.size();
		break;
	case TM_MENU_PREVIEW:
		nSize = m_PreviewMenuMap.size();
		break;
	case TM_MENU_ACTIVE:
		nSize = m_ActiveMenuMap.size();
		break;
	};

	return nSize;
}

int TabManager::getChildCount( int nParent, int nFlag )
{
	int nParentSize = getParentCount( nFlag );
	int nSize = 0;

	if(nParent > nParentSize - 1)
		return nSize;

	switch (nFlag)
	{
	case TM_MENU_LOADED:
		nSize = m_LoadedMenuMap[nParent].m_TabElement.size();
		break;
	case TM_MENU_PREVIEW:
		nSize = m_PreviewMenuMap[nParent].m_TabElement.size();
		break;
	case TM_MENU_ACTIVE:
		nSize = m_ActiveMenuMap[nParent].m_TabElement.size();
		break;
	};

	return nSize;
}

int TabManager::getVisualCount( void )
{
	int nSize = m_VisualList.size();

	return nSize;
}

int TabManager::setAlternateText( int nParent, int nChild, string szText, int nFlag)
{
	TabDataPackage tabRequest;

	tabRequest = requestTabDataPackage(nParent, nChild, nFlag);
	tabRequest.ContentDisplayText = szText;

	submitTabDataPackage(nParent, nChild, tabRequest, nFlag);

	return 1;
}

int TabManager::setDefaultText( int nParent, int nChild, int nFlag)
{

	int nParentSize = m_LoadedMenuMap.size();
	if(nParent > nParentSize - 1)
		return 2;

	int nChildSize = m_LoadedMenuMap[nParent].m_TabElement.size();
	if(nChild > nChildSize - 1)
		return 2;

	string defaultText = m_LoadedMenuMap[nParent].m_TabElement[nChild].TabDisplayText;

	setAlternateText(nParent, nChild, defaultText, nFlag);
	return 1;
}

int TabManager::getAssetCount( int nVisual, int nFlag )
{
	int nSize = 0;
	int nVisualSize = getVisualCount();

	if(nVisual > nVisualSize - 1)
		return nSize;

	switch (nFlag)
	{
	case TM_ASSET_BKG:
		nSize = m_VisualList[nVisual].m_BkgAsset.size();
		break;
	case TM_ASSET_ICON:
		nSize = m_VisualList[nVisual].m_IconAsset.size();
		break;
	case TM_ASSET_REFLECT:
		nSize = m_VisualList[nVisual].m_ReflectAsset.size();
		break;
	};

	return nSize;
}
void TabManager::copyMenuMap( int srcMenuMap, int destMenuMap, bool bRefreshMenu )
{
	map<int, TMMenuElement> m_srcMenuMap;
	
	switch (srcMenuMap)
	{
	case TM_MENU_LOADED:
		m_srcMenuMap = m_LoadedMenuMap;
		break;
	case TM_MENU_PREVIEW:
		m_srcMenuMap = m_PreviewMenuMap;
		break;
	case TM_MENU_ACTIVE:
		m_srcMenuMap = m_ActiveMenuMap;
		break;
	};

	switch (destMenuMap)
	{
	case TM_MENU_LOADED:
		m_LoadedMenuMap = m_srcMenuMap;
		break;
	case TM_MENU_PREVIEW:
		m_PreviewMenuMap = m_srcMenuMap;
		break;
	case TM_MENU_ACTIVE:
		m_ActiveMenuMap = m_srcMenuMap;
		break;
	};

	if(bRefreshMenu)
		_notifyMenuRefresh(destMenuMap);
}

void TabManager::forceMenuRefresh( int nFlag )
{
	_notifyMenuRefresh(nFlag);
}

string TabManager::getTabDisplayText( int nParent, int nChild, int nFlag )
{
	int nParentSize = getParentCount(nFlag);
	if(nParent > nParentSize - 1)
		nParent = nParentSize - 1;

	int nChildSize = getChildCount(nParent, nFlag);
	if(nChild > nChildSize - 1)
		nChild = nChildSize - 1;

	string strBuffer = "";

	switch (nFlag)
	{
	case TM_MENU_LOADED:
		strBuffer = m_LoadedMenuMap[nParent].m_TabElement[nChild].TabDisplayText;
		break;
	case TM_MENU_PREVIEW:
		strBuffer = m_PreviewMenuMap[nParent].m_TabElement[nChild].TabDisplayText;
		break;
	case TM_MENU_ACTIVE:
		strBuffer = m_ActiveMenuMap[nParent].m_TabElement[nChild].TabDisplayText;
		break;
	};

	return strBuffer;
}

VisualData TabManager::getVisualData( int nVisual )
{
	VisualData visualRequest;

	int nVisualSize = getVisualCount();
	if(nVisual > nVisualSize - 1)
		nVisual = nVisualSize - 1;

	visualRequest.nBkgListSize = getAssetCount(nVisual, TM_ASSET_BKG);
	visualRequest.nIconListSize = getAssetCount(nVisual, TM_ASSET_ICON);
	visualRequest.nReflectListSize = getAssetCount(nVisual, TM_ASSET_REFLECT);
	visualRequest.PreviewPath = m_VisualList[nVisual].PreviewPath;
	visualRequest.VisualDescription = m_VisualList[nVisual].VisualDescription;
	visualRequest.VisualName = m_VisualList[nVisual].VisualName;

	return visualRequest;
}

AssetData TabManager::getAssetData( int nVisual, int nIndex, int nFlag)
{
	AssetData assetRequest;
	
	int nVisualSize = getVisualCount();
	if(nVisual > nVisualSize - 1)
		nVisual = nVisualSize - 1;

	int nAssetSize = getAssetCount(nVisual, nFlag);
	if(nIndex > nAssetSize - 1)
		nIndex = nAssetSize - 1;

	switch (nFlag)
	{
	case TM_ASSET_BKG:
		assetRequest = m_VisualList[nVisual].m_BkgAsset[nIndex];
		break;
	case TM_ASSET_ICON:
		assetRequest = m_VisualList[nVisual].m_IconAsset[nIndex];
		break;
	case TM_ASSET_REFLECT:
		assetRequest = m_VisualList[nVisual].m_ReflectAsset[nIndex];
		break;
	};

	return assetRequest;
}

TabDataPackage TabManager::requestTabDataPackage( int nParent, int nChild, int nFlag )
{
	map<int, TMMenuElement> m_TempMenuMap;
	TabDataPackage tabRequest;

	switch (nFlag)
	{
	case TM_MENU_LOADED:
		m_TempMenuMap = m_LoadedMenuMap;
		break;
	case TM_MENU_PREVIEW:
		m_TempMenuMap = m_PreviewMenuMap;
		break;
	case TM_MENU_ACTIVE:
		m_TempMenuMap = m_ActiveMenuMap;
		break;
	};

	int nParentSize = getParentCount(nFlag);
	
	if(nParent > nParentSize - 1)
		nParent = nParentSize - 1;

	int nChildSize = getChildCount(nParent, nFlag);

	if(nChild > nChildSize - 1)
		nChild = nChildSize - 1;

	tabRequest.ContentAnimState = m_TempMenuMap[nParent].m_TabElement[nChild].AnimationState;
	tabRequest.ContentBkgIndex = m_TempMenuMap[nParent].m_TabElement[nChild].BackgroundIndex;
	tabRequest.ContentBkgVisual = m_TempMenuMap[nParent].m_TabElement[nChild].BackgroundVisual;
	tabRequest.ContentDisplayText = m_TempMenuMap[nParent].m_TabElement[nChild].TabDisplayText;
	tabRequest.ContentGradientState = m_TempMenuMap[nParent].m_TabElement[nChild].GradientState;
	tabRequest.ContentIconIndex = m_TempMenuMap[nParent].m_TabElement[nChild].IconIndex;
	tabRequest.ContentIconVisual = m_TempMenuMap[nParent].m_TabElement[nChild].IconVisual;
	tabRequest.ContentIconState = m_TempMenuMap[nParent].m_TabElement[nChild].IconState;
	tabRequest.ContentReflectIndex = m_TempMenuMap[nParent].m_TabElement[nChild].ReflectionIndex;
	tabRequest.ContentReflectVisual = m_TempMenuMap[nParent].m_TabElement[nChild].ReflectionVisual;
	tabRequest.ContentMainTextColor = m_TempMenuMap[nParent].m_TabElement[nChild].MainTextColor;
	tabRequest.ContentReflectTextColor = m_TempMenuMap[nParent].m_TabElement[nChild].ReflectionTextColor;

	return tabRequest;
}

void TabManager::submitTabDataPackage( int nParent, int nChild, TabDataPackage tabData, int nFlag)
{
	map<int, TMMenuElement> m_TempMenuMap;
	
	switch (nFlag)
	{
	case TM_MENU_LOADED:
		m_TempMenuMap = m_LoadedMenuMap;
		break;
	case TM_MENU_PREVIEW:
		m_TempMenuMap = m_PreviewMenuMap;
		break;
	case TM_MENU_ACTIVE:
		m_TempMenuMap = m_ActiveMenuMap;
		break;
	};

	int nParentSize = m_TempMenuMap.size();
	if(nParent > nParentSize -1)
		nParent = nParentSize - 1;
	
	int nChildSize = m_TempMenuMap[nParent].m_TabElement.size();
	if(nChild > nChildSize - 1)
		nChild = nChildSize - 1;

	m_TempMenuMap[nParent].m_TabElement[nChild].MenuIndex = nParent;
	m_TempMenuMap[nParent].m_TabElement[nChild].TabIndex = nChild;
	m_TempMenuMap[nParent].m_TabElement[nChild].AnimationState = tabData.ContentAnimState;
	m_TempMenuMap[nParent].m_TabElement[nChild].BackgroundIndex = tabData.ContentBkgIndex;
	m_TempMenuMap[nParent].m_TabElement[nChild].BackgroundVisual = tabData.ContentBkgVisual;
	m_TempMenuMap[nParent].m_TabElement[nChild].GradientState = tabData.ContentGradientState;
	m_TempMenuMap[nParent].m_TabElement[nChild].IconIndex = tabData.ContentIconIndex;
	m_TempMenuMap[nParent].m_TabElement[nChild].IconVisual = tabData.ContentIconVisual;
	m_TempMenuMap[nParent].m_TabElement[nChild].IconState = tabData.ContentIconState;
	m_TempMenuMap[nParent].m_TabElement[nChild].ReflectionIndex = tabData.ContentReflectIndex;
	m_TempMenuMap[nParent].m_TabElement[nChild].ReflectionVisual = tabData.ContentReflectVisual;
	m_TempMenuMap[nParent].m_TabElement[nChild].MainTextColor = tabData.ContentMainTextColor;
	m_TempMenuMap[nParent].m_TabElement[nChild].ReflectionTextColor = tabData.ContentReflectTextColor;
	m_TempMenuMap[nParent].m_TabElement[nChild].TabDisplayText = tabData.ContentDisplayText;
	
	switch (nFlag)
	{
	case TM_MENU_LOADED:
		m_LoadedMenuMap = m_TempMenuMap;
		break;
	case TM_MENU_PREVIEW:
		m_PreviewMenuMap = m_TempMenuMap;
		break;
	case TM_MENU_ACTIVE:
		m_ActiveMenuMap = m_TempMenuMap;
		break;
	};

	_notifyContentTabChange(nParent, nChild, nFlag);
}



void TabManager::SaveXML_MenuSettings( int nFlag )
{
	map<int, TMMenuElement> m_SaveMenuMap;
	
	switch (nFlag)
	{
	case TM_MENU_LOADED:
		m_SaveMenuMap = m_LoadedMenuMap;
		break;
	case TM_MENU_PREVIEW:
		m_SaveMenuMap = m_PreviewMenuMap;
		break;
	case TM_MENU_ACTIVE:
		m_SaveMenuMap = m_ActiveMenuMap;
		break;
	};

	string strSettings = "";

	strSettings.append("<menusettings>\n");
	int menuSize = m_SaveMenuMap.size();
	int tabSize = 0;

	for(int i = 0; i < menuSize; i++)
	{
		strSettings.append(sprintfa("\t<menu id='%d' tabs='%d' text='%s' color='0x%08X' />\n", m_SaveMenuMap[i].MenuIndex, m_SaveMenuMap[i].TabElementCount, m_SaveMenuMap[i].MenuDisplayText.c_str(), m_SaveMenuMap[i].MenuTextColor));
	}
	strSettings.append("</menusettings>\n");
	strSettings.append("<tabsettings>\n");

	for(int i = 0; i < menuSize; i++)
	{
		tabSize = m_SaveMenuMap[i].m_TabElement.size();
		for(int x = 0; x < tabSize; x++)
		{
			strSettings.append(sprintfa("\t<tab id='%d' parentid='%d' text='%s'>\n", m_SaveMenuMap[i].m_TabElement[x].TabIndex, m_SaveMenuMap[i].m_TabElement[x].MenuIndex, m_SaveMenuMap[i].m_TabElement[x].TabDisplayText.c_str()));
			strSettings.append(sprintfa("\t\t<bkgvisual>%d</bkgvisual>\n", m_SaveMenuMap[i].m_TabElement[x].BackgroundVisual));
			strSettings.append(sprintfa("\t\t<bkgindex>%d</bkgindex>\n", m_SaveMenuMap[i].m_TabElement[x].BackgroundIndex));
			strSettings.append(sprintfa("\t\t<iconvisual>%d</iconvisual>\n", m_SaveMenuMap[i].m_TabElement[x].IconVisual));
			strSettings.append(sprintfa("\t\t<iconindex>%d</iconindex>\n", m_SaveMenuMap[i].m_TabElement[x].IconIndex));
			strSettings.append(sprintfa("\t\t<reflectvisual>%d</reflectvisual>\n", m_SaveMenuMap[i].m_TabElement[x].ReflectionVisual));
			strSettings.append(sprintfa("\t\t<reflectindex>%d</reflectindex>\n", m_SaveMenuMap[i].m_TabElement[x].ReflectionIndex));
			strSettings.append(sprintfa("\t\t<animstate>%d</animstate>\n", m_SaveMenuMap[i].m_TabElement[x].AnimationState));
			strSettings.append(sprintfa("\t\t<gradientstate>%d</gradientstate>\n", m_SaveMenuMap[i].m_TabElement[x].GradientState));
			strSettings.append(sprintfa("\t\t<iconstate>%d</iconstate>\n", m_SaveMenuMap[i].m_TabElement[x].IconState));
			strSettings.append(sprintfa("\t\t<mtextcolor>0x%08X</mtextcolor>\n", m_SaveMenuMap[i].m_TabElement[x].MainTextColor));
			strSettings.append(sprintfa("\t\t<rtextcolor>0x%08X</rtextcolor>\n", m_SaveMenuMap[i].m_TabElement[x].ReflectionTextColor));
			strSettings.append("\t</tab>\n");
		}
	}
	strSettings.append("</tabsettings>");
	
	string skinPath;

	skinPath = SkinManager::getInstance().getCurrentxmlPath();
	
	
	// Save settings to MenuSettings.xml
	StringToFile(strSettings, skinPath + "Settings\\MenuSettings.xml");

}

void TabManager::LoadXML_MenuSettings( void )
{
	string skinPath;
	
	skinPath = SkinManager::getInstance().getCurrentxmlPath();
	
	if(!FileExistsA(skinPath + "Settings\\MenuSettings.xml")) {
		DebugMsg("TabManager", "\\Settings\\MenuSettings does not exists");
	}
	else
	{
		string filePath = skinPath + "Settings\\MenuSettings.xml";
		XMLReader * xml = LoadConfigFile(filePath);
		DebugMsg("TabManager", "Loading config from %s",filePath.c_str());
		string strBuffer = "";
		int nValue = 0, nCounter = 0;
		DWORD dwValue = 0;

		TMMenuElement menuElement;
	

		// Loop through XML file and find all the menu and tab elements
		while (!EndAttribute(xml, "menusettings"))
		{
			if(StartAttribute(xml, "menu"))
			{
				nValue = xml->getAttributeValueAsInt("id");
				menuElement.MenuIndex = nValue;
				nValue = xml->getAttributeValueAsInt("tabs");
				menuElement.TabElementCount = nValue;
				strBuffer = xml->getAttributeValue("text");
				menuElement.MenuDisplayText = strBuffer;
				strBuffer = xml->getAttributeValue("color");
				dwValue = strtoul(strBuffer.c_str(), NULL, 16);
				menuElement.MenuTextColor = dwValue;
				//DebugMsg("TabManagerInsert","%d,%d,%d,%s,%s,0x%08x",nCounter,menuElement.MenuIndex,menuElement.TabElementCount,menuElement.MenuDisplayText.c_str(),strBuffer.c_str(),dwValue);
				m_LoadedMenuMap.insert(map<int, TMMenuElement>::value_type(nCounter, menuElement));
				nCounter++;
				
			}	
		}

		xml->restart();

		int nParentID = 0, nSize = 0;
		string key, val;

		while (!EndAttribute(xml, "tabsettings"))
		{
			if(StartAttribute(xml, "tab"))
			{
				TMTabElement  tabElement;
				nValue = xml->getAttributeValueAsInt("id");
				tabElement.TabIndex = nValue;
				nValue = xml->getAttributeValueAsInt("parentid");
				tabElement.MenuIndex = nValue;
			
				strBuffer = xml->getAttributeValue("text");
				tabElement.TabDisplayText = strBuffer;

				while(!EndAttribute(xml, "tab"))
				{
					key = xml->getNodeName();
					key = xml->getNodeData();
				
					if(StartAttribute(xml, key.c_str()))
					{
						val = xml->getSubNodeData();
						
						if(strcmp(make_lowercaseA(key).c_str(), "bkgvisual")==0)
						{
								
							tabElement.BackgroundVisual = atoi(val.c_str());
							if(tabElement.BackgroundVisual  == 0)
							{
								DebugMsg("TabManager","Tab loaded as blue!");
							}
						}

						if(strcmp(make_lowercaseA(key).c_str(), "bkgindex")==0)
							tabElement.BackgroundIndex = atoi(val.c_str());

						if(strcmp(make_lowercaseA(key).c_str(), "iconvisual")==0)
							tabElement.IconVisual = atoi(val.c_str());

						if(strcmp(make_lowercaseA(key).c_str(), "iconindex")==0)
							tabElement.IconIndex = atoi(val.c_str());

						if(strcmp(make_lowercaseA(key).c_str(), "reflectvisual")==0)
							tabElement.ReflectionVisual = atoi(val.c_str());

						if(strcmp(make_lowercaseA(key).c_str(), "reflectindex")==0)
							tabElement.ReflectionIndex = atoi(val.c_str());

						if(strcmp(make_lowercaseA(key).c_str(), "animstate")==0)
							tabElement.AnimationState = (val=="0") ? false : true;

						if(strcmp(make_lowercaseA(key).c_str(), "gradientstate")==0)
							tabElement.GradientState = (val=="0") ? false : true;

						if(strcmp(make_lowercaseA(key).c_str(), "iconstate")==0)
							tabElement.IconState = (val=="0") ? false : true;

						if(strcmp(make_lowercaseA(key).c_str(), "mtextcolor")==0)
							tabElement.MainTextColor = strtoul(val.c_str(), NULL, 16);

						if(strcmp(make_lowercaseA(key).c_str(), "rtextcolor")==0)
							tabElement.ReflectionTextColor = strtoul(val.c_str(), NULL, 16);
					}
				}
				
			
				nSize = m_LoadedMenuMap.size();
				nParentID = tabElement.MenuIndex;

				if(nParentID < nSize)
				{
					m_LoadedMenuMap[nParentID].m_TabElement.insert(map<int, TMTabElement>::value_type(tabElement.TabIndex, tabElement));
				}
			
			 }
		}
	}
	
}

void TabManager::LoadXML_VisualList( void )
{
	string skinPath;
	
	skinPath = SkinManager::getInstance().getCurrentxmlPath();
	
	if(!FileExistsA(skinPath + "XML\\VisualList.xml")) {
		DebugMsg("TabManager", "XML\\VisualList.xml does not exist");

	} else
	{
		string filePath = skinPath + "XML\\VisualList.xml";
		XMLReader * xml = LoadConfigFile(filePath);

		string strBuffer = "";
		int nCounter = 0;

		map<int, string> m_PathList;
		// Loop through XML file and find all the menu and tab elements
		while (!EndAttribute(xml, "visuallist"))
		{
			if(StartAttribute(xml, "visual"))
			{
				strBuffer = skinPath + xml->getAttributeValue("path");
				m_PathList.insert(map<int, string>::value_type(nCounter, strBuffer));
				nCounter++;	
			}	
		}
		
		int nSize = m_PathList.size();

		for(int i =  0; i < nSize; i++)
		{
			LoadXML_VisualData(i, m_PathList[i]);
		}
	}
}

void TabManager::LoadXML_VisualData( int nIndex, string filePath )
{

	if(!FileExistsA(filePath))
	{
		DebugMsg("TabManager", "%s Does Not Exist", filePath.c_str());
	}
	else
	{
		XMLReader * xml = LoadConfigFile(filePath);
		int nCounter = 0;
		string key, val;

		while(!EndAttribute(xml, "visual"))
		{
			key = xml->getNodeName();
			key = xml->getNodeData();

			if(StartAttribute(xml, key.c_str()))
			{
				val = xml->getSubNodeData();

				if(strcmp(make_lowercaseA(key).c_str(), "visualname")==0)
					m_VisualList[nIndex].VisualName = val;

				if(strcmp(make_lowercaseA(key).c_str(), "visualdesc")==0)
					m_VisualList[nIndex].VisualDescription = val;

				if(strcmp(make_lowercaseA(key).c_str(), "previewpath")==0)
					m_VisualList[nIndex].PreviewPath = val;

			}
		}

		xml->restart();

		string strBuffer = "";
		AssetData iconAsset;
		nCounter = 0;

		
		while(!EndAttribute(xml, "iconlist"))
		{
			if(StartAttribute(xml, "icon"))
			{
				strBuffer = xml->getAttributeValue("name");

				iconAsset.Index = nCounter;
				iconAsset.Visual = nIndex;
				iconAsset.Name = strBuffer;

				string key, val;
				while(!EndAttribute(xml, "icon"))
				{
					key = xml->getNodeName();
					key = xml->getNodeData();

					if(StartAttribute(xml, key.c_str()))
					{
						val = xml->getSubNodeData();

						if(strcmp(make_lowercaseA(key).c_str(), "iconpath")==0)
							iconAsset.ImagePath = val;
					}
				}
				m_VisualList[nIndex].m_IconAsset.insert(map<int, AssetData>::value_type(nCounter, iconAsset));
				nCounter++;
			}
		}
		xml->restart();

		strBuffer = "";
		AssetData bkgAsset;
		nCounter = 0;

		
		while(!EndAttribute(xml, "bkglist"))
		{
			if(StartAttribute(xml, "bkg"))
			{
				strBuffer = xml->getAttributeValue("name");

				bkgAsset.Index = nCounter;
				bkgAsset.Visual = nIndex;
				bkgAsset.Name = strBuffer;

				string key, val;
				while(!EndAttribute(xml, "bkg"))
				{
					key = xml->getNodeName();
					key = xml->getNodeData();

					if(StartAttribute(xml, key.c_str()))
					{
						val = xml->getSubNodeData();

						if(strcmp(make_lowercaseA(key).c_str(), "bkgpath")==0)
							bkgAsset.ImagePath = val;
					}
				}
				m_VisualList[nIndex].m_BkgAsset.insert(map<int, AssetData>::value_type(nCounter, bkgAsset));
				nCounter++;
			}
		}

		strBuffer = "";
		AssetData reflectAsset;
		nCounter = 0;

		
		while(!EndAttribute(xml, "reflectionlist"))
		{
			if(StartAttribute(xml, "reflection"))
			{
				strBuffer = xml->getAttributeValue("name");

				reflectAsset.Index = nCounter;
				reflectAsset.Visual = nIndex;
				reflectAsset.Name = strBuffer;

				string key, val;
				while(!EndAttribute(xml, "reflection"))
				{
					key = xml->getNodeName();
					key = xml->getNodeData();

					if(StartAttribute(xml, key.c_str()))
					{
						val = xml->getSubNodeData();

						if(strcmp(make_lowercaseA(key).c_str(), "reflectionpath")==0)
							reflectAsset.ImagePath = val;
					}
				}
				m_VisualList[nIndex].m_ReflectAsset.insert(map<int, AssetData>::value_type(nCounter, reflectAsset));
				nCounter++;
			}
		}
	}
}