#include <direct.h>
#include "stdafx.h"

#include "../../../Application/FreeStyleUIApp.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../XML/xmlreader.h"
#include "../../Debug/Debug.h"
#include "../../Settings/Settings.h"
#include "../XZP/XZPManager.h"
#include "../Version/VersionManager.h"
#include "../Drives/DrivesManager.h"
#include "SkinManager.h"
#include "XML/SkinXml.h"


string SkinManager::getCurrentSkin()
{
	return CurrentSkin.SkinName;
}

map<string, skinitem> SkinManager::getAvailableSkins()
{
	return m_skinList;
}

void SkinManager::setSkin( CFreestyleUIApp&  g_uiApp,string firstScene = "Main.xur", bool firstLoad )
{
	loadSkin(g_uiApp,firstScene, firstLoad );			
}
	
void SkinManager::setScene(string sceneName, HXUIOBJ curScene, bool stayVisible, string objectRef, void * pvInitData )
{
	loadScene(sceneName, curScene, stayVisible, objectRef, pvInitData);
}

string SkinManager::getCurrentScenePath()
{
	if (CurrentSkin.isCompressed)
	{
		return CurrentSkin.XZPPath;
	} else {
		return CurrentSkin.SkinPath;
	}
}

bool SkinManager::getDisplayFPS()
{
	return CurrentSkin.DisplayFPS;
}

bool SkinManager::getDisplayCPURender()
{
	return CurrentSkin.DisplayCPURender;
}

bool SkinManager::getDisplayProjectTitle()
{
	return CurrentSkin.DisplayProjectTitle;
}

bool SkinManager::getDisplayFreeMem()
{
	return CurrentSkin.DisplayFreeMEM;
}

string SkinManager::getFavoriteIconPath()
{
	return CurrentSkin.FavoriteIconPath;
}

/*void SkinManager::ReadSkinXML(skinitem* skin)
{
	ReadSkinSettings(skin);
}
*/

/*void SkinManager::WriteSkinXML(string Setting, string Value)
{
	Write(Setting, Value);
}
*/

bool SkinManager::isCurrentCompressed()
{
	return CurrentSkin.isCompressed;
}

string SkinManager::getCurrentxmlPath()
{
	return CurrentSkin.xmlpath;
}

string SkinManager::getCurrentSkinFileName()
{
	return CurrentSkin.SkinFileName;
}

HRESULT SkinManager::getSettingMap(map<string, string>* temp, string sceneName, string instanceId)
{
	string tempS;

	CurrentSkin.m_SceneInfo.Count = CurrentSkin.m_SceneInfo.Element.find(sceneName.c_str());
	if (CurrentSkin.m_SceneInfo.Count != CurrentSkin.m_SceneInfo.Element.end())
	{
		CurrentSkin.m_SceneInfo.Count->second.Count = CurrentSkin.m_SceneInfo.Count->second.Element.find(instanceId.c_str());
		if (CurrentSkin.m_SceneInfo.Count->second.Count != CurrentSkin.m_SceneInfo.Count->second.Element.end())
		{
			*temp = CurrentSkin.m_SceneInfo.Count->second.Count->second.Element;

		} else {
			map<string, string> temp2;
			temp2["none"] = "none";
			temp = &temp2;
		}
	} else {
		map<string, string> temp2;
		temp2["none"] = "none";
		temp = &temp2;
	}

	return S_OK;
}

void SkinManager::DefaultSettings()
{
	CurrentSkin.SkinFileName = SETTINGS::getInstance().getSkin();
	//m_XexVer = (int)SETTINGS::getInstance().getXexVer();
	CurrentSkin.MinVer = m_XexVer;
	CurrentSkin.MaxVer = m_XexVer;
	DebugMsg("SkinManager","Try to load skin : %s",CurrentSkin.SkinFileName.c_str());
	m_BaseSkinPath = SETTINGS::getInstance().getCurrentSkinPath();
	CurrentSkin.Author = "";
	CurrentSkin.Version = "";
	CurrentSkin.DisplayFPS = "false";
	CurrentSkin.DisplayProjectTitle = "false";
	CurrentSkin.DisplayCPURender = "false";
	CurrentSkin.DisplayFreeMEM = "false";
	CurrentSkin.FavoriteIconPath = "ylwstar.png";
	CurrentSkin.HasDefaultFont = false;

	if (m_BaseSkinPath == "")
	{
		m_BaseSkinPath = "game:\\Skins\\";
		SETTINGS::getInstance().setCurrentSkinPath(m_BaseSkinPath);
	}


	
	//DebugMsg("SkinManager", "Loading Compressed Skins");
	//m_skinList = XZPManager::getInstance().SkinInfo();
	//DebugMsg("SkinManager", "Loading  Skins");
	getSkinList();
	//DebugMsg("SkinManager","Set Active Skin");
	setActiveSkin();
}

void SkinManager::setActiveSkin()
{
	int done = 0;
	DebugMsg("SkinManager","Setting active Skin");
	std::map<string, skinitem>::iterator itr = m_skinList.find(CurrentSkin.SkinFileName);
	//DebugMsg("SkinManager","Skin Search Found:  %s", itr->second.SkinName.c_str());

	if (itr != m_skinList.end())
	{
		CurrentSkin = itr->second;
		//DebugMsg("SkinManager","Current Skin is %s", CurrentSkin.SkinName.c_str());
		done = 1;
	} else {
		DebugMsg("SkinManager","Current Skin not Found");
		itr = m_skinList.begin();
		int nXexVer = VersionManager::getInstance().getFSDDashVersion().Version.dwMajor;
		for (;itr != m_skinList.end(); itr++)
		{
			DebugMsg("SkinManager", "Name: %s   MinVer:  %d    MaxVer:  %d    XeXVer:  %d", itr->second.SkinName.c_str(), itr->second.MinVer, itr->second.MaxVer, nXexVer);
			if ((strcmp(itr->second.SkinName.c_str(), "") == 0) || itr->second.MinVer > nXexVer || itr->second.MaxVer < nXexVer)
			{
				DebugMsg("SkinManager","Incompatible Skin. Skipping %s.", itr->second.SkinName.c_str());
			} else {
				done = 1;
				CurrentSkin = itr->second;
				SETTINGS::getInstance().setSkin(CurrentSkin.SkinFileName);
				break;
			}
		}
	}
	if (done != 1)
	{
		DebugMsg("SkinManager","no valid skin found");
		XNotifyQueueUICustom(L"No Valid Skins Found.");
	}
}

string SkinManager::getCurrentFullSkinPath()
{
	return CurrentSkin.FullSkinPath;
}

string SkinManager::getCurrentSkinPath()
{
	return CurrentSkin.SkinPath;
}

string SkinManager::getCurrentXZPSkinPath()
{
	return CurrentSkin.XZPPath;
}

void SkinManager::getSkinList()
{
	/* Finding Skin.xzp files for processing. */
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd;
		/* Now finding and validating skin folders*/
	searchcmd = m_BaseSkinPath + "*";
	searchcmd = str_replaceallA(searchcmd,"\\\\","\\");
	DebugMsg("SkinManager","Scanning %s for skins",searchcmd.c_str());

	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			//DebugMsg("Confirming Skin", m_BaseSkinPath.c_str(), findFileData.cFileName);
			string searchxur = sprintfaA("%s%s\\skin.xur", m_BaseSkinPath.c_str(), findFileData.cFileName);
			if (FileExists(searchxur))
			{
				skinitem AddSkin;
				string szExecutablePath = ParseExecutablePath(DrivesManager::getInstance().getExecutablePath()) + "\\";
				szExecutablePath = str_replaceallA(m_BaseSkinPath, "game:\\", szExecutablePath);
				szExecutablePath = str_replaceallA(szExecutablePath, "\\\\", "\\");
				AddSkin.SkinPath = sprintfaA("%s%s\\", m_BaseSkinPath.c_str() , findFileData.cFileName);
				AddSkin.xmlpath = sprintfaA("%s%s\\", m_BaseSkinPath.c_str() , findFileData.cFileName);
				AddSkin.FullSkinPath = make_lowercaseA(sprintfaA("%s%s", szExecutablePath.c_str(), findFileData.cFileName)) + "\\";
				AddSkin.PreviewPath = sprintfaA("file://%s%s\\preview.png", m_BaseSkinPath.c_str() , findFileData.cFileName);
				AddSkin.IconPath = sprintfaA("file://%s%s\\devIcon.png", m_BaseSkinPath.c_str() , findFileData.cFileName);
				AddSkin.SkinFileName = findFileData.cFileName;
				AddSkin.isCompressed = false;
				ReadSkinSettings(&AddSkin);
				if (AddSkin.SkinName == "")
				{
					AddSkin.SkinName = findFileData.cFileName;
				}
				m_skinList[AddSkin.SkinFileName] = AddSkin;

			} else if (FileExt(findFileData.cFileName) == "xzp")
			{
				skinitem AddSkin;
				string szExecutablePath = ParseExecutablePath(DrivesManager::getInstance().getExecutablePath()) + "\\";
				szExecutablePath = str_replaceallA(m_BaseSkinPath, "game:\\", szExecutablePath);
				szExecutablePath = str_replaceallA(szExecutablePath, "\\\\", "\\");
				AddSkin.SkinPath = sprintfaA("%s", m_BaseSkinPath.c_str());
				AddSkin.SkinFileName = sprintfaA("%s", findFileData.cFileName);
				AddSkin.XZPPath = sprintfaA("%s%s#", m_BaseSkinPath.c_str(), findFileData.cFileName);
				AddSkin.FullSkinPath = make_lowercaseA(sprintfaA("%s%s", szExecutablePath.c_str(), findFileData.cFileName));
				AddSkin.xmlpath = sprintfaA("%sSkins\\%s\\", SETTINGS::getInstance().getDataPath().c_str(), findFileData.cFileName);
				AddSkin.PreviewPath = sprintfaA("file://%s%s#preview.png", m_BaseSkinPath.c_str(), findFileData.cFileName);
				AddSkin.IconPath = sprintfaA("file://%s%s#devIcon.png", m_BaseSkinPath.c_str(), findFileData.cFileName);
				AddSkin.isCompressed = true;
				ExtractSkinInfo(&AddSkin);
				ReadSkinSettings(&AddSkin);
				if (AddSkin.SkinName == "")
				{
					string skinName = findFileData.cFileName;
					skinName = TrimRightStr(skinName, ".xzp");
					skinName = skinName + " Compressed";
					AddSkin.SkinName = skinName;
				}
				m_skinList[AddSkin.SkinFileName] = AddSkin;
			}
		} while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
	}
	//DebugMsg("SkinManager","Finished listing skin");
}

void SkinManager::loadSkin( CFreestyleUIApp&  g_uiApp , string firstScene, bool firstLoad )
{
	
	HRESULT hr;
	string path;

	if(firstLoad)
	{
		ExtractTempSkinInfo();
	}		

	if (CurrentSkin.isCompressed)
	{
		path = CurrentSkin.XZPPath;
	} else {
		path = CurrentSkin.SkinPath;
	}

	RegisterFonts(g_uiApp, path);

	string skinload;
	string sceneload;
	skinload = "file://" + path + "skin.xur";
	sceneload = "file://" + path;


	//DebugMsg("SkinManager","loading %s as default skin", skinload.c_str());
	hr = g_uiApp.LoadSkin( strtowstr(skinload).c_str() );
	if (hr != S_OK)
	{
		DebugMsg("SkinManager", "Skin Loaded with HRESULT : %08x");
		return;
	}
	//DebugMsg("SkinManager","loading %s %s as default scene", sceneload.c_str(),firstScene.c_str());
	hr = g_uiApp.LoadFirstScene( strtowstr(sceneload).c_str(), strtowstr(firstScene).c_str() );
	if (hr != S_OK)
	{
		DebugMsg("SkinManager", "Scene Loaded with HRESULT : %08x");
	}
}

void SkinManager::RegisterFonts(CFreestyleUIApp&  g_uiApp, string path)
{
	HRESULT hr;

	vector<FontInfo>::iterator itr;
	for(itr = CurrentSkin.Fonts.begin(); itr != CurrentSkin.Fonts.end(); itr++)
	{
		string FontPath = "file://" + path + (*itr).FontFile;

		if((*itr).FontDefault)  
		{  
			DebugMsg("SkinManager","Loading default font %s as %s",FontPath.c_str(),(*itr).FontName.c_str());  
			hr = g_uiApp.RegisterDefaultTypeface(strtowstr((*itr).FontName).c_str(), strtowstr(FontPath).c_str());  
			CurrentSkin.HasDefaultFont = true;
		}  
		else  
		{  
			DebugMsg("SkinManager","Loading no default font %s as %s",FontPath.c_str(),(*itr).FontName.c_str());  
			hr = g_uiApp.RegisterTypeface( strtowstr((*itr).FontName).c_str(), strtowstr(FontPath).c_str());  
		}  
		if(hr!=S_OK)
		{
			DebugMsg("SkinManager","Couldnt load font...");  
		}
	}
}

void SkinManager::ExtractTempSkinInfo()
{
	std::map<string, skinitem>::iterator itr = m_skinList.begin();
	for (; itr != m_skinList.end(); itr++)
	{
		if (itr->second.isCompressed)
		{
			// Check to see if Data Path exists
			string datapath = "game:\\Cache\\";
			SETTINGS::getInstance().setDataPath(datapath, false);
			HRESULT hr;
			if (!FileExists(datapath)){
				DebugMsg("SkinManager", "Creating %s folder", datapath.c_str());
				_mkdir(datapath.c_str());
			}

			// Check to see if Data Path exists
			string skinpath = datapath + "Skins\\";
			if (!FileExists(skinpath)){
				DebugMsg("SkinManager", "Creating %s folder", skinpath.c_str());
				_mkdir(skinpath.c_str());
			}

			// Check to see if xmlpath exists "skins\\default.xzp\\"
			itr->second.xmlpath = datapath + itr->second.xmlpath;
			string xmlPath = itr->second.xmlpath;
			if (!FileExists(xmlPath)){
				DebugMsg("SkinManager", "Creating %s folder", xmlPath.c_str());
				_mkdir(xmlPath.c_str());
			}

			// Check to see if skin.xml exists, if not, extract it
			string strbuff = xmlPath + "skin.xml";
			if (!FileExists(strbuff)){
				DebugMsg("SkinManager", "File %s doesn't exist. Extracting", strbuff.c_str());
				hr = XZPManager::getInstance().ExtractXML(itr->second.SkinPath, itr->second.SkinFileName, "skin.xml");
				if (hr == S_OK)
				{
					DebugMsg("SkinManager", "Extracted %s Successfully", strbuff.c_str());
				} else {
					DebugMsg("SkinManager", "Failed to extract %s", strbuff.c_str());
				}
			}
		}
	}
	string file = SETTINGS::getInstance().getDataPath() + "Skins\\" + CurrentSkin.SkinFileName + "\\skin.xml";
	
	SkinXml xml;
	ATG::XMLParser parser;
	parser.RegisterSAXCallbackInterface(&xml);
	parser.ParseXMLFile(file.c_str());
	CurrentSkin.Fonts = xml.Fonts;
}

void SkinManager::ExtractSkinInfo(skinitem* Skin)
{
	if (Skin->isCompressed)
	{
		// Check to see if Data Path exists
		string datapath = SETTINGS::getInstance().getDataPath() + "Skins\\";
		HRESULT hr;
		if (!FileExists(datapath)){
			DebugMsg("SkinManager", "Creating %s folder", datapath.c_str());
			_mkdir(datapath.c_str());
		}
		// Check to see if xmlpath exists "skins\\default.xzp\\"
		if (!FileExists(Skin->xmlpath)){
			DebugMsg("SkinManager", "Creating %s folder", Skin->xmlpath.c_str());
			_mkdir(Skin->xmlpath.c_str());
		}
		// Check to see if Settings folder exists
		string tempcfg = Skin->xmlpath + "Settings\\";
		if(!FileExists(tempcfg)){
		DebugMsg("SkinManager", "Creating %s folder", tempcfg.c_str());
		_mkdir(tempcfg.c_str());
		}

		// Check to see if Menu Settings Exists, if not, extract it
		string strbuff = Skin->xmlpath + "Settings\\MenuSettings.xml";
		if (!FileExists(Skin->xmlpath + "Settings\\MenuSettings.xml"))
		{
			DebugMsg("SkinManager", "File %s doesn't exist. Extracting", strbuff.c_str());
			hr = XZPManager::getInstance().ExtractXML(Skin->SkinPath, Skin->SkinFileName, "Settings\\MenuSettings.xml");
			if (hr == S_OK)
			{
				DebugMsg("SkinManager", "Extracted %s Successfully", strbuff.c_str());
			} else {
				DebugMsg("SkinManager", "Failed to extract %s", strbuff.c_str());
			}
		}

		// Check to see if SkinSettings Exists, if not, extract it
		strbuff = Skin->xmlpath + "Settings\\SkinSettings.xml";
		if (!FileExists(Skin->xmlpath + "Settings\\SkinSettings.xml"))
		{
			DebugMsg("SkinManager", "File %s doesn't exist. Extracting", strbuff.c_str());
			hr = XZPManager::getInstance().ExtractXML(Skin->SkinPath, Skin->SkinFileName, "Settings\\SkinSettings.xml");
			if (hr == S_OK)
			{
				DebugMsg("SkinManager", "Extracted %s Successfully", strbuff.c_str());
			} else {
				DebugMsg("SkinManager", "Failed to extract %s", strbuff.c_str());
			}
		}

		// Check to verify that XML Folder Exists
		string tempxml = Skin->xmlpath + "XML\\";
		if (!FileExists(tempxml))
		{
			DebugMsg("SkinManager", "Creating %s folder", tempxml.c_str());
			_mkdir(tempxml.c_str());
		}

		// Look for BkgListA and extract that
		strbuff = Skin->xmlpath + "XML\\BkgListA.xml";
		if (!FileExists(Skin->xmlpath + "XML\\BkgListA.xml")){
			DebugMsg("SkinManager", "File %s doesn't exist. Extracting", strbuff.c_str());
			hr = XZPManager::getInstance().ExtractXML(Skin->SkinPath, Skin->SkinFileName, "XML\\BkgListA.xml");
			if (hr == S_OK)
			{
				DebugMsg("SkinManager", "Extracted %s Successfully", strbuff.c_str());
			} else {
				DebugMsg("SkinManager", "Failed to extract %s", strbuff.c_str());
			}
		}
		// Look for BkgListB and extract that
		strbuff = Skin->xmlpath + "XML\\BkgListB.xml";
		if (!FileExists(Skin->xmlpath + "XML\\BkgListB.xml")){
			DebugMsg("SkinManager", "File %s doesn't exist. Extracting", strbuff.c_str());
			hr = XZPManager::getInstance().ExtractXML(Skin->SkinPath, Skin->SkinFileName, "XML\\BkgListB.xml");
			if (hr == S_OK)
			{
				DebugMsg("SkinManager", "Extracted %s Successfully", strbuff.c_str());
			} else {
				DebugMsg("SkinManager", "Failed to extract %s", strbuff.c_str());
			}
		}
			// Look for VisualList and extract that
		strbuff = Skin->xmlpath + "XML\\VisualList.xml";
		if (!FileExists(Skin->xmlpath + "XML\\VisualList.xml")){
			DebugMsg("SkinManager", "File %s doesn't exist. Extracting", strbuff.c_str());
			hr = XZPManager::getInstance().ExtractXML(Skin->SkinPath, Skin->SkinFileName, "XML\\VisualList.xml");
			if (hr == S_OK)
			{
				DebugMsg("SkinManager", "Extracted %s Successfully", strbuff.c_str());
			} else {
				DebugMsg("SkinManager", "Failed to extract %s", strbuff.c_str());
			}
		}
		XMLReader * xml = LoadConfigFile(strbuff);
		// Extract the visual files as well
		DebugMsg("SkinManager","Extracting Visual XML files");
		while (xml && xml->read())
		{
			if(StartAttribute(xml, "visual"))
			{
				DebugMsg("SkinManager", "Reading VisualList.xml");
				strbuff = xml->getAttributeValue("path");
				DebugMsg("SkinManager", "Path: %s", strbuff.c_str());
				tempxml = Skin->xmlpath + strbuff;
				DebugMsg("SkinManager", "TempPath: %s", tempxml.c_str());
				if (!FileExists(tempxml)){
					DebugMsg("SkinManager", "File %s doesn't exist. Extracting", strbuff.c_str());
					hr = XZPManager::getInstance().ExtractXML(Skin->SkinPath, Skin->SkinFileName, strbuff);
					if (hr == S_OK)
					{
						DebugMsg("SkinManager", "Extracted %s Successfully", strbuff.c_str());
					} else {
						DebugMsg("SkinManager", "Failed to extract %s", strbuff.c_str());
					}
				}
			}
		}
	}
}


void SkinManager::loadScene(string sceneName, HXUIOBJ curScene, bool stayVisible, string objectRef, void * pvInitData)
{
	HXUIOBJ hScene;
	
	string path;
	if (CurrentSkin.isCompressed)
	{
		path = "file://" + CurrentSkin.XZPPath;
	} else {
		path = "file://" + CurrentSkin.SkinPath;
	}
	CXuiScene::SceneCreate( strtowstr(path).c_str(), strtowstr(sceneName).c_str(), &hScene, pvInitData);
	
	if(objectRef == "")
		objectRef = sceneName;
		
	getXurObjectHandle[objectRef] = hScene;

	DebugMsg("SkinManager", "SceneObjectMap- Scene Name:  %s,  HXUIOBJ:  %08X", objectRef.c_str(), hScene);
	XuiSceneNavigateForward(curScene, stayVisible, hScene, XUSER_INDEX_ANY);
}

void SkinManager::ReadSkinSettings(skinitem* Skin)
{
	
	if (Skin->isCompressed)
	{
		WCHAR * fileBuffer;
		UINT fileLength;

		string filePath = Skin->SkinPath + Skin->SkinFileName;

		HRESULT retVal = NULL;
		retVal = XZPManager::getInstance().XZPOpenMemory(filePath, "skin.xml", (BYTE**)&fileBuffer, &fileLength);
		if(retVal != S_OK){
			DebugMsg("SkinManager", "XZP Resource failed to extract from archive");
		}
	
		SkinXml xml;
		ATG::XMLParser parser;
		parser.RegisterSAXCallbackInterface(&xml);
		parser.ParseXMLBuffer((const CHAR*)fileBuffer, fileLength);

		DebugMsg("SkinManager", "Setting Variables Compressed");
		Skin->DisplayCPURender = xml.DisplayCPURender;
		Skin->DisplayFPS = xml.DisplayFPS;
		Skin->DisplayFreeMEM = xml.DisplayFreeMEM;
		Skin->DisplayProjectTitle = xml.DisplayProjectTitle;
		Skin->Author = xml.SkinAuthor;
		Skin->Version = xml.SkinVersion;
		Skin->SkinName = xml.SkinName;
		Skin->MinVer = xml.MinVer;
		Skin->MaxVer = xml.MaxVer;
		Skin->m_SceneInfo = xml.m_XmlInfo.m_SceneInfo;
		Skin->Fonts = xml.Fonts;

	} else {
		string file;

		file = Skin->xmlpath + "skin.xml";

		//DebugMsg("SkinManager","Reading %s",file.c_str());
		if(!FileExistsA(file))
		{
			DebugMsg("SkinManager","%s does not exist",file.c_str());
		}
		else
		{
			SkinXml xml;
			ATG::XMLParser parser;
			parser.RegisterSAXCallbackInterface(&xml);
			parser.ParseXMLFile(file.c_str());

			DebugMsg("SkinManager", "Setting Variables");
			Skin->DisplayCPURender = xml.DisplayCPURender;
			Skin->DisplayFPS = xml.DisplayFPS;
			Skin->DisplayFreeMEM = xml.DisplayFreeMEM;
			Skin->DisplayProjectTitle = xml.DisplayProjectTitle;
			Skin->Author = xml.SkinAuthor;
			Skin->Version = xml.SkinVersion;
			Skin->SkinName = xml.SkinName;
			Skin->MinVer = xml.MinVer;
			Skin->MaxVer = xml.MaxVer;
			Skin->m_SceneInfo = xml.m_XmlInfo.m_SceneInfo;
			Skin->Fonts = xml.Fonts;
		}
	}
}


/*
void SkinManager::Write(string Setting, string Value)
{
	string file;
	file = CurrentSkin.xmlpath + "skin.xml";
	
	string result;
	string startstring = sprintfaA("<%s>", Setting.c_str());
	string endstring = sprintfaA("</%s>", Setting.c_str());
	FileToStringA(result, file);
	size_t StartPos = result.find(startstring);
	StartPos = StartPos + startstring.length();
	size_t EndPos = result.find(endstring);
	size_t pos = EndPos - StartPos;
	if (StartPos != startstring.length())
	{
		result.replace(StartPos, pos, Value.c_str());
		DebugMsg("SkinManager","Writing %s value at %d location", Value.c_str(), StartPos);
		StringToFileA(result, file);
	} else {
		DebugMsg("SkinManager","%s setting not found in skin.xml", Setting);	
	}		
}
*/

void SkinManager::WriteToXML(string RelativePath, string Setting, string Value)
{
	string file;

	file = CurrentSkin.xmlpath + RelativePath;

	string result;
	string startstring = sprintfaA("<%s>", Setting.c_str());
	string endstring = sprintfaA("</%s>", Setting.c_str());
	FileToStringA(result, file);
	size_t StartPos = result.find(startstring);
	StartPos = StartPos + startstring.length();
	size_t EndPos = result.find(endstring);
	size_t pos = EndPos - StartPos;
	if (StartPos != startstring.length())
	{
		result.replace(StartPos, pos, Value.c_str());
		DebugMsg("SkinManager","Writing %s value at %d location", Value.c_str(), StartPos);
		StringToFileA(result, file);
	} else {
		DebugMsg("SkinManager","%s setting not found in skin.xml", Setting);	
	}		
}

