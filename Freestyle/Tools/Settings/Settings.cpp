#include "stdafx.h"
#include "Settings.h"
#include "../../Scenes/GameView/GameList/GamesList.h"
#include "../SQLite/FSDSql.h"
#include "../../Tools/Managers/Drives/DrivesManager.h"

SETTINGS::SETTINGS()
{
	DefaultSettings();

	//TODO : WILL CURRENTLY CREATE ONLY 1 FOLDER DEPTH.. IE : game:\\fsd2data\\content will not work
	if(!FileExistsA(m_DataPath))
	{
		DebugMsg("Settings","%s does not exist",m_DataPath.c_str());
		_mkdir(m_DataPath.c_str());
	}
	else
	{
		DebugMsg("Settings","%s does  exist",m_DataPath.c_str());
	}
	Read();

	// Locate the embedded font file in memory
	//DWORD dwStoredDigest;
	//VOID * pSectionData;
	//XGetModuleSection( GetModuleHandle(NULL), "FSDVer", &pSectionData, &dwStoredDigest);

	//dashVersionInfo.nMajor = ((VersionInfo*)pSectionData)->nMajor;
	//dashVersionInfo.nMinor = ((VersionInfo*)pSectionData)->nMinor;
	//dashVersionInfo.nRevsion = ((VersionInfo*)pSectionData)->nRevsion;
	//dashVersionInfo.nType = ((VersionInfo*)pSectionData)->nType;			// 0- undefined, 1- alpha, 2- beta, 3- release
}

void SETTINGS::getGameListSort(int type, int* sort, int* dir, int* view) {
	std::map<int, SortIds*>::iterator it;
	it = m_sortIds.find(type);
	if (it != m_sortIds.end()) {
		*sort = (*it).second->sortOrder;
		*dir = (*it).second->sortDirection;
		*view = (*it).second->subtitleView;
	}
	else {
		*sort = 0;
		*dir = 0;
		*view = 0;
	}
};

void SETTINGS::setGameListSort(int type, int sort, int dir, int view) {
	std::map<int, SortIds*>::iterator it;
	it = m_sortIds.find(type);
	if (it != m_sortIds.end()) {
		(*it).second->sortOrder = sort;
		(*it).second->sortDirection = dir;
		(*it).second->subtitleView = view;
	}
	else {
		m_sortIds.insert(make_pair(type, new SortIds((GAMELIST_SORTSTYLE)sort, (GAMELIST_SORTDIRECTION)dir, (GAMELIST_SUBTITLEVIEW)view)));
	}
	Write();
};

void SETTINGS::ResetToDefaultSettings()
{

	m_DisableAutoScan = 0;
	m_ResumeAssetDownload = 1;
	m_weatherLocation = "undefined";
	m_WeatherUnits = 0;

	m_MaxScreenshots = 15;
	m_EnableAdvancedMode = 0;

	m_DownloadIcon = 1;
	m_DownloadBoxart = 1;
	m_DownloadBanner = 1;
	m_DownloadBackground = 1;
	m_DownloadScreenshots = 1;
	m_DownloadVideo = 0;
	m_HttpPassword = "fsdhttp";
	m_HttpUsername = "fsdhttp";

	m_HTTPDisableSecurity = 0;

	m_glTransition = 0;

	m_Locale = "en-US";
	m_PingPatch = false;
	ScanPaths.clear();
	rssPaths.clear();
	m_NTPonStartup = 1;
	m_DataPath = "";
	m_UpdateOnBeta = 0;
	m_UpdateXexNotice = 0;
	m_UpdateSkinNotice = 0;
	m_UpdatePluginNotice = 0;
	m_InstallerState = 0;
	m_DataPathX = "";
	m_CurrentSkinPath = "";
	m_HorizOverscan = 0;
	m_VertOverscan = 0;
	m_HorizDisplacement = 0;
	m_VertDisplacement = 0;
	m_EnableLetterbox = 1;
	m_FanSpeed = 40;
	m_port = 21;
	m_ftpuser = "xbox";
	m_ftppass = "xbox";
	m_xboxip = "0.0.0.0";
	m_TempStatus = 1;
	m_debugport = 0;
	m_UseScreenshots = 1;
	m_UseBackgrounds = 1;
	m_SemiTransparentBackground = 0;
	m_UsePreviews = 1;
	m_glVisual = 0;
	m_Cels = 1;
	m_DiskStatus = 1;
	m_ParentalControl = 0;
	m_ShowIP = 1;
	m_ShowStatus =1;
	m_DateFormat = 0;
	m_TimeFormat = 0;
	m_LedType = 0;
	m_skin = "Default.xzp";
	m_XexVer = 2.0;
	m_showAvatar = 1;
	m_zipCode = "undefined";
	m_KaiSettingsUsername = "";
	m_KaiSettingsPassword = "";
	m_KaiSettingsIP = "Undefined";
	m_FsdUsername = "";
	m_FsdPassword = "";
	m_FsdUseLogin = FALSE;
	m_NotifyContactOnline = 1;
	m_NotifyContactOffline = 1;
	m_NotifyInviteToGame = 1;
	m_NotifyContactPM = 1;
	m_NotifyOpponentPM = 1;
	m_KaiSettingsRemember = 0;
	m_KaiSettingsAutoLogin = 0;
	m_MusicPath = "";
	m_VideoPath = "";
	m_OrigPath = "";
	m_360Path = "";	
	m_FtpServerOn = 1;
	m_HttpServerOn = 1;
	m_SambaClientOn = 0;
	m_SambaClientWorkgroup = "WORKGROUP";
	m_SambaServerOn = 0;
	m_hostname = "XBOX360";
	m_ShowStatusAtBoot = 0;

	m_CoverQuality = 0;

	// the default sort order for all types of GameLists
	for (int i = 0; i <= (int)CONTENT_LAST; i++) {
		m_sortIds.insert(make_pair(i, new SortIds(GAMELIST_SORTSTYLE_FIRST, GAMELIST_SORTDIRECTION_FORWARD, GAMELIST_SUBTITLEVIEW_FIRST)));
	}

	Write();
}

void SETTINGS::DefaultSettings()
{
	
	m_DisableAutoScan = 0;
	m_ResumeAssetDownload = 1;
	m_weatherLocation = "undefined";
	m_WeatherUnits = 0;

	m_EnableAdvancedMode = 0;
	m_MaxScreenshots = 15;

	m_DownloadIcon = 1;
	m_DownloadBoxart = 1;
	m_DownloadBanner = 1;
	m_DownloadBackground = 1;
	m_DownloadScreenshots = 1;
	m_DownloadVideo = 0;
	m_HttpPassword = "fsdhttp";
	m_HttpUsername = "fsdhttp";
	m_HTTPDisableSecurity = 0;

	m_glTransition = 0;

	m_Locale = "en-US";
	m_PingPatch = false;
	ScanPaths.clear();
	rssPaths.clear();
	m_NTPonStartup = 1;
	m_DataPath = "";
	m_UpdateOnBeta = 0;
	m_UpdateXexNotice = 0;
	m_UpdateSkinNotice = 0;
	m_UpdatePluginNotice = 0;
	m_InstallerState = 0;
	m_DataPathX = "";
	m_CurrentSkinPath = "";
	m_HorizOverscan = 0;
	m_VertOverscan = 0;
	m_HorizDisplacement = 0;
	m_VertDisplacement = 0;
	m_EnableLetterbox = 1;
	m_FanSpeed = 40;
	m_port = 21;
	m_ftpuser = "xbox";
	m_ftppass = "xbox";
	m_xboxip = "0.0.0.0";
	m_TempStatus = 1;
	m_debugport = 0;
	m_UseBackgrounds = 1;
	m_UseScreenshots = 1;
	m_SemiTransparentBackground = 0;
	m_UsePreviews = 1;
	m_glVisual = 0;
	m_Cels = 1;
	m_DiskStatus = 1;
	m_ParentalControl = 0;
	m_ShowIP = 1;
	m_ShowStatus =1;
	m_DateFormat = 0;
	m_TimeFormat = 0;
	m_LedType = 0;
	m_skin = "Default.xzp";
	m_XexVer = 2.0;
	m_showAvatar = 1;
	m_zipCode = "undefined";
	m_KaiSettingsUsername = "";
	m_KaiSettingsPassword = "";
	m_KaiSettingsIP = "Undefined";
	m_FsdUsername = "";
	m_FsdPassword = "";
	m_FsdUseLogin = FALSE;
	m_NotifyContactOnline = 1;
	m_NotifyContactOffline = 1;
	m_NotifyInviteToGame = 1;
	m_NotifyContactPM = 1;
	m_NotifyOpponentPM = 1;
	m_KaiSettingsRemember = 0;
	m_KaiSettingsAutoLogin = 0;
	m_MusicPath = "";
	m_VideoPath = "";
	m_OrigPath = "";
	m_360Path = "";	
	m_FtpServerOn = 1;
	m_HttpServerOn = 1;
	m_SambaClientOn = 0;
	m_SambaClientWorkgroup = "WORKGROUP";
	m_ShowStatusAtBoot = 0;
	m_SambaServerOn = 0;
	m_hostname = "XBOX360";

	m_CoverQuality = 0;

	// the default sort order for all types of GameLists
	for (int i = 0; i <= (int)CONTENT_LAST; i++) {
		m_sortIds.insert(make_pair(i, new SortIds(GAMELIST_SORTSTYLE_FIRST, GAMELIST_SORTDIRECTION_FORWARD, GAMELIST_SUBTITLEVIEW_FIRST)));
	}
}

void SETTINGS::Read()
{
	DebugMsg("Settings","Reading %s",(m_DataPath + "fsd2config.xml").c_str());


// First comes the SQL stuff

	bool gotScanPathsFromDB = false;

	// fetch scanpaths from the DB
	ScanPaths.clear();
	ScanPaths = FSDSql::getInstance().getScanPaths();
	if (ScanPaths.size() != 0)
		gotScanPathsFromDB = true;
	
	bool gotGlobalSettingsFromDB = FSDSql::getInstance().readGlobalSettings(m_VideoPath, m_MusicPath, m_360Path, m_OrigPath, m_zipCode, m_HorizOverscan,
		m_VertOverscan, m_HorizDisplacement, m_VertDisplacement, m_EnableLetterbox, m_port, m_ftpuser, m_ftppass, m_FanSpeed, m_LedType,
		m_TimeFormat, m_DateFormat, m_NTPonStartup, m_UpdateXexNotice, m_UpdateSkinNotice, m_UpdatePluginNotice, m_UpdateOnBeta, m_InstallerState,
		m_FsdUsername, m_FsdPassword, m_FsdUseLogin, m_Locale, m_UseBackgrounds, m_glVisual, m_TempStatus, m_DiskStatus, m_Cels, m_ShowIP, m_ShowStatus, m_showAvatar, m_UseScreenshots, 
		m_KaiSettingsAutoLogin, m_KaiSettingsRemember, m_NotifyContactOnline, m_NotifyContactOffline, m_NotifyInviteToGame, m_NotifyContactPM, m_NotifyOpponentPM, m_KaiSettingsIP,
		m_KaiSettingsUsername, m_KaiSettingsPassword, m_FtpServerOn, m_HttpServerOn, m_ShowStatusAtBoot,
		m_DownloadIcon, m_DownloadBoxart, m_DownloadBanner, m_DownloadBackground, m_DownloadVideo, m_DownloadScreenshots, 
		m_MaxScreenshots, m_HTTPDisableSecurity, m_HttpUsername, m_HttpPassword, m_glTransition, m_SambaClientOn,
		m_weatherLocation, m_ResumeAssetDownload, m_DisableAutoScan, m_WeatherUnits, m_SambaClientWorkgroup, m_SambaServerOn, m_hostname, m_EnableAdvancedMode, 
		m_CoverPoll, m_CoverPassword, m_CoverUsername, m_CoverCheck, m_CoverQuality );

	bool gotUserSettingsFromDB = false;
	gotUserSettingsFromDB = FSDSql::getInstance().readUserSettings(m_skin);

	if (m_port == 0) {
		m_port = 21;
		m_ftpuser = "xbox";
		m_ftppass = "xbox";
	}

// Now comes the XML stuff

/*	if(FileExistsA(m_DataPath + "fsd2config.xml"))
	{
		DebugMsg("Settings","File exist, proceed");
		XMLReader * xml = LoadConfigFile(m_DataPath + "fsd2config.xml");
		//	char temp[1024];
		DebugMsg("Settings","Loading Config");
		while (xml && xml->read())
		{

			if(StartAttribute(xml,"locale") && gotGlobalSettingsFromDB && m_Locale.compare("") == 0)
			{
				m_Locale = xml->getSubNodeData();  // only take this if the one in the database is empty
			}

			if (StartAttribute(xml,"path") && !gotScanPathsFromDB)
			{
				string Path = xml->getSubNodeData();
				string RetailTab = xml->getAttributeValue("retail");
				string DevkitTab = xml->getAttributeValue("devkit");

				ScanPath TmpScanPath;

				TmpScanPath.Path = Path;
				TmpScanPath.RetailTab = RetailTab;
				TmpScanPath.DevkitTab = DevkitTab;
				ScanPaths.push_back(TmpScanPath);

				DebugMsg("SETTINGS", "path: '%s', retail: '%s', devkit: '%s'", TmpScanPath.Path.c_str(), TmpScanPath.RetailTab.c_str(), TmpScanPath.DevkitTab.c_str());
			}

			if (StartAttribute(xml,"rss"))
			{
				//string path = xml->getSubNodeData();
				//rssPaths.push_back(path);
				//Create RssItems
				//RssReader::getInstance().AddRssFeed(path);

			}



			if (StartAttribute(xml,"display") && !gotGlobalSettingsFromDB)
			{
				m_HorizOverscan = xml->getAttributeValueAsInt("horizoverscan");
				m_VertOverscan = xml->getAttributeValueAsInt("vertoverscan");
				m_HorizDisplacement = xml->getAttributeValueAsInt("horizdisplacement");
				m_VertDisplacement = xml->getAttributeValueAsInt("vertdisplacement");
				m_EnableLetterbox = xml->getAttributeValueAsInt("letterbox");

			}

			if( StartAttribute(xml, "weather") && !gotGlobalSettingsFromDB)
			{
				m_zipCode = xml->getAttributeValue("location");
			}

			if (StartAttribute(xml,"gamelistopt"))
			{
				m_UseBackgrounds = xml->getAttributeValueAsInt("background");
				m_SemiTransparentBackground = xml->getAttributeValueAsInt("semitransparent");
				m_UsePreviews = xml->getAttributeValueAsInt("preview");
				m_glVisual = xml->getAttributeValueAsInt("glvisual");
			}

			if (StartAttribute(xml,"mainscreenopt"))
			{
				m_TempStatus = xml->getAttributeValueAsInt( "tempstatus");
				m_Cels = xml->getAttributeValueAsInt( "celsius");
				m_DiskStatus = xml->getAttributeValueAsInt( "diskstatus");
				m_ParentalControl = xml->getAttributeValueAsInt( "parent");
				m_ShowIP = xml->getAttributeValueAsInt( "showip");
				m_ShowStatus = xml->getAttributeValueAsInt( "showstatus");
				m_showAvatar = xml->getAttributeValueAsInt("showavatar");
			}


			if (StartAttribute(xml,"moreoptions") && !gotGlobalSettingsFromDB)
			{
				m_FanSpeed = xml->getAttributeValueAsInt( "fanspeed");

				m_LedType = xml->getAttributeValueAsInt("ledformat");
			}

			if (StartAttribute(xml,"timeformat") && !gotGlobalSettingsFromDB)
			{
				m_TimeFormat = xml->getAttributeValueAsInt("time");
				m_DateFormat = xml->getAttributeValueAsInt("date");
				m_NTPonStartup = xml->getAttributeValueAsInt("ntponstartup");
			}
			if (StartAttribute(xml,"skin") && !gotUserSettingsFromDB)
			{
				m_skin = xml->getSubNodeData();
			}
			if (StartAttribute(xml, "Version"))
			{
				m_XexVer = xml->getAttributeValueAsFloat("XEX");
			}
			if (StartAttribute(xml, "Media") && !gotGlobalSettingsFromDB)
			{
				m_MusicPath = xml->getAttributeValue("Music");
				m_VideoPath = xml->getAttributeValue("Video");
			}

			if (StartAttribute(xml, "DVDCopy") && !gotGlobalSettingsFromDB)
			{
				m_360Path = xml->getAttributeValue("360Path");
				m_OrigPath = xml->getAttributeValue("OrigPath");
			}

			/*<ftp xboxip="192.168.1.66" port="21" username="xbox" password="xbox" />
			<debug host="192.168.1.6" port="22597" />*/
/*			if (StartAttribute(xml,"ftp") && !gotGlobalSettingsFromDB)
			{
				m_xboxip = xml->getAttributeValue("xboxip");
				m_port = xml->getAttributeValueAsInt("port");
				m_ftpuser = xml->getAttributeValue("username");
				m_ftppass = xml->getAttributeValue("password");
			}

			if (StartAttribute(xml,"gamelistsort"))
			{
				int type = xml->getAttributeValueAsInt("type");
				int sort = xml->getAttributeValueAsInt("sort");
				int dir = xml->getAttributeValueAsInt("dir");
				int view = xml->getAttributeValueAsInt("view");

				std::map<int, SortIds*>::iterator it;
				it = m_sortIds.find(type);
				if (it != m_sortIds.end()) {
					(*it).second->sortOrder = sort;
					(*it).second->sortDirection = dir;
					(*it).second->subtitleView = view;
				}
			}

			if (StartAttribute(xml,"debug"))
			{
				m_debugip = xml->getAttributeValue("host");
				m_debugport = xml->getAttributeValueAsInt("port");
			}

			if( StartAttribute(xml, "autoupdater") && !gotGlobalSettingsFromDB)
			{
				m_UpdateXexNotice = xml->getAttributeValueAsInt("ondash");
				m_UpdateSkinNotice = xml->getAttributeValueAsInt("onskin");
				m_UpdatePluginNotice = xml->getAttributeValueAsInt("onplugin");
				m_UpdateOnBeta = xml->getAttributeValueAsInt("onbeta");
				m_InstallerState = xml->getAttributeValueAsInt("installer");
			}
			if (StartAttribute(xml,"KaiNotices"))
			{
				m_NotifyContactOnline = xml->getAttributeValueAsInt("contactonline");
				m_NotifyContactOffline = xml->getAttributeValueAsInt("contactoffline");
				m_NotifyInviteToGame = xml->getAttributeValueAsInt("invitetogame");
				m_NotifyContactPM = xml->getAttributeValueAsInt("contactpm");
				m_NotifyOpponentPM = xml->getAttributeValueAsInt("opponentpm");
			}

			if (StartAttribute(xml, "kaisettings"))
			{
				m_KaiSettingsRemember = xml->getAttributeValueAsInt("remember");
				m_KaiSettingsAutoLogin = xml->getAttributeValueAsInt("autologin");
				m_KaiSettingsUsername = xml->getAttributeValue("username");
				m_KaiSettingsPassword = xml->getAttributeValue("password");
				m_KaiSettingsIP = xml->getAttributeValue("serverip");
			}
		}
	}
	else
	{
		DebugMsg("Settings", "Default Settings Loaded- no config file found");
	}*/
}

ULONGLONG SETTINGS::InternalTabNameToDatabaseTabId(string name) {
	// this routine is utter crap, and it should be removed as soon as we are using TabIds everywhere, from the database, instead of TabNames
	if (!name.compare("360")) 
		return 1;
	if (!name.compare("XBLA")) 
		return 2;
	if (!name.compare("HOMEBREW")) 
		return 3;
	if (!name.compare("EMULATOR")) 
		return 4;
	if (!name.compare("XBOX1")) 
		return 5;

	return 0;
}

const ScanPath* SETTINGS::getScanPathByScanPathId(ULONGLONG scanPathId) {
	vector<ScanPath>::iterator sp_itr;
	for (sp_itr = ScanPaths.begin() ; sp_itr != ScanPaths.end() ; sp_itr++)
	{
		ScanPath sp = *sp_itr;
		if (sp.PathId == scanPathId)
			return &(*sp_itr);
	}
	return NULL;
}


string SETTINGS::DatabaseTabIdToInternalTabName(ULONGLONG tabId) {
	// this routine is utter crap, and it should be removed as soon as we are using TabIds everywhere, from the database, instead of TabNames
	switch(tabId) {
		case 1:
			return "XBLA";
		case 2:
			return "Xbox 360";
		case 3:
			return "Xbox Classic";
		case 4:
			return "Homebrew";
		case 5:
			return "Emulator";
	}
	return "Automatic";
}

void SETTINGS::Write()
{
	DebugMsg("Settings","DoSaveSettings");
	
	// Here's the SQL stuff
	FSDSql::getInstance().writeGlobalSettings(m_VideoPath, m_MusicPath, m_360Path, m_OrigPath, m_zipCode, m_HorizOverscan,
		m_VertOverscan, m_HorizDisplacement, m_VertDisplacement, m_EnableLetterbox, m_port, m_ftpuser, m_ftppass, m_FanSpeed, m_LedType,
		m_TimeFormat, m_DateFormat, m_NTPonStartup, m_UpdateXexNotice, m_UpdateSkinNotice, m_UpdatePluginNotice, m_UpdateOnBeta, m_InstallerState, 
		m_FsdUsername, m_FsdPassword, m_FsdUseLogin, m_Locale, m_UseBackgrounds, m_glVisual, m_TempStatus, m_DiskStatus, m_Cels, m_ShowIP, m_ShowStatus, m_showAvatar, m_UseScreenshots,
		m_KaiSettingsAutoLogin, m_KaiSettingsRemember, m_NotifyContactOnline, m_NotifyContactOffline, m_NotifyInviteToGame, m_NotifyContactPM, m_NotifyOpponentPM, m_KaiSettingsIP,
		m_KaiSettingsUsername, m_KaiSettingsPassword, m_FtpServerOn, m_HttpServerOn,m_ShowStatusAtBoot,
		m_DownloadIcon, m_DownloadBoxart, m_DownloadBanner, m_DownloadBackground, m_DownloadVideo, m_DownloadScreenshots,
		m_MaxScreenshots, m_HTTPDisableSecurity, m_HttpUsername, m_HttpPassword, m_glTransition, m_SambaClientOn,
		m_weatherLocation, m_ResumeAssetDownload, m_DisableAutoScan, m_WeatherUnits, m_SambaClientWorkgroup, m_SambaServerOn, m_hostname, m_EnableAdvancedMode,
		m_CoverPoll, m_CoverPassword, m_CoverUsername, m_CoverCheck, m_CoverQuality );

	FSDSql::getInstance().writeUserSettings(m_skin);

	vector<ScanPath>::iterator sp_itr;
	for (sp_itr = ScanPaths.begin() ; sp_itr != ScanPaths.end() ; sp_itr++)
	{
		ScanPath sp = *sp_itr;
		string root = sp.Path.substr(0, sp.Path.find_first_of(":")+1);
		string path = sp.Path.substr(sp.Path.find_first_of(":")+1);

		Drive* d = DrivesManager::getInstance().getDriveByMountPoint(root);
		if (d != NULL) {
			string devId = d->getSerialStr();
			ULONGLONG bAdded = FSDSql::getInstance().addScanPath(path, devId, InternalTabNameToDatabaseTabId(sp.RetailTab),
				InternalTabNameToDatabaseTabId(sp.DevkitTab), sp.ScanDepth);
			if (!bAdded) {
				ScanPath* existingPath = FSDSql::getInstance().getScanPath(path, devId);
				// that path exists already, so have to call modifyScanPath...but that means deleting all content for that scanpath 
				// (because it might be from a deeper depth, for example)
				// and then we should go off and scan for content again...				
				delete existingPath;
			}
		}
	}

	// Here's the XML stuff
//	string settingsxml;
//	settingsxml.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
//	settingsxml.append("<config>\n");
//	settingsxml.append("\t<locale>");
//	settingsxml.append(m_Locale);
//	settingsxml.append("</locale>\n");

//	settingsxml.append("\t\t<!--\tPaths are in the database now, you can't set them here. Use the Settings page in FSD.-->\n");

	/*  Paths are in the Database now
	settingsxml.append("\t<paths>\n");
	settingsxml.append("\t\t<!--\tRetail and devkit attributes are used to configure the tab on which\n");
	settingsxml.append("\t\t\tthe title is displayed if it is a retail or devkit executable.\n");
	settingsxml.append("\t\t\tOptions are: 360, HOMEBREW, EMULATOR and XBOX1 (can be left blank).\n");
	settingsxml.append("\t\t\tExample: <path retail='360' devkit='EMULATOR'>hdd1:\\emulators</path> -->\n");

	
	for (sp_itr = ScanPaths.begin() ; sp_itr != ScanPaths.end() ; sp_itr++)
	{
		settingsxml.append(sprintfa("\t\t<path retail='%s' devkit='%s'>%s</path>\n", (*sp_itr).RetailTab.c_str(), (*sp_itr).DevkitTab.c_str(), (*sp_itr).Path.c_str()));
	}

	settingsxml.append("\t</paths>\n");
	*/

	/*if (StartAttribute(xml,"rss"))
	{
	string path = xml->getSubNodeData();
	rssPaths.push_back(path);
	//Create RssItems
	RssReader::getInstance().AddRssFeed(path);

	}*/
//	settingsxml.append("\t<feeds>\n");
//	vector<string>::iterator itr;
//	for (itr = rssPaths.begin() ; itr != rssPaths.end() ; itr++)
//	{
//		settingsxml.append(sprintfa("\t\t\t<rss>%s</rss>\n",(*itr).c_str()));
//	}
//	settingsxml.append("\t</feeds>\n");
//	settingsxml.append(sprintfa("\t<skin>%s</skin>\n",(m_skin).c_str()));
//	settingsxml.append(sprintfa("\t<Media Video='%s' Music='%s' Music MUST be in GAME:\\ />\n", m_VideoPath.c_str(), m_MusicPath.c_str()));
//	settingsxml.append(sprintfa("\t<DVDCopy 360Path='%s' OrigPath='%s'/>\n", m_360Path.c_str(), m_OrigPath.c_str()));
	/*	settingsxml.append(sprintfa(L"\t<SkinPath>%s</SkinPath>\n",m_CurrentSkinPath.c_str()));
	settingsxml.append(sprintfa(L"\t<DataPath>%s</DataPath>\n",m_DataPath.c_str()));*/
//	settingsxml.append(sprintfa("\t<weather location='%s' />\n", m_zipCode.c_str()));
//	settingsxml.append(sprintfa("\t<display horizoverscan='%d' vertoverscan='%d' horizdisplacement='%d' vertdisplacement='%d' letterbox='%d'/>\n",m_HorizOverscan,m_VertOverscan, m_HorizDisplacement, m_VertDisplacement, m_EnableLetterbox));
//	settingsxml.append(sprintfa("\t<ftp xboxip='%s' port='%d' username='%s' password='%s' />\n",(m_xboxip).c_str(),m_port,(m_ftpuser).c_str(),(m_ftppass).c_str()));
	//settingsxml.append(sprintfa("\t<debug host='%s' port='%d' />\n",cl.debugip.c_str(),cl.debugport));
	//settingsxml.append(sprintfa("\t<gamelistopt background='%d' preview='%d' semitransparent='%d' glvisual='%d' />\n",m_UseBackgrounds,m_UsePreviews,m_SemiTransparentBackground, m_glVisual));
	//settingsxml.append(sprintfa("\t<mainscreenopt tempstatus='%d' diskstatus='%d' celsius='%d' parent='%d' showip='%d' showstatus='%d' showavatar='%d' />\n",m_TempStatus,m_DiskStatus,m_Cels,m_ParentalControl,m_ShowIP,m_ShowStatus, m_showAvatar));
//	settingsxml.append(sprintfa("\t<moreoptions fanspeed='%d' ledformat='%d' />\n",m_FanSpeed,m_LedType));
//	settingsxml.append(sprintfa("\t<timeformat time='%d' date='%d' ntponstartup='%d' />\n",m_TimeFormat,m_DateFormat,m_NTPonStartup));
//	settingsxml.append(sprintfa("\t<autoupdater ondash='%d' onskin='%d' onplugin='%d' onbeta='%d' installer='%d' />\n", m_UpdateXexNotice, m_UpdateSkinNotice, m_UpdatePluginNotice, m_UpdateOnBeta, m_InstallerState));


//	std::map<int, SortIds*>::iterator it;
//	for (it = m_sortIds.begin(); it != m_sortIds.end() ; it++) {
//		settingsxml.append(sprintfa("\t<gamelistsort type='%d' sort='%d' dir='%d' view='%d' />\n", (*it).first, (*it).second->sortOrder, (*it).second->sortDirection, (*it).second->subtitleView));
//	}
	
//	settingsxml.append(sprintfa("\t<Version XEX='%d' />\n", m_XexVer));
//	settingsxml.append("</config>\n");

	if(!FileExists(m_DataPath))
	{
		mkdir(m_DataPath.c_str());
	}
//	DebugMsg("Settings","Saving settings to %s",(m_DataPath + "fsd2config.xml").c_str());
//	StringToFile(settingsxml,(m_DataPath + "fsd2config.xml").c_str());
}


void SETTINGS::setDataPath(string DataPath, bool writeData)
{
	m_DataPath = DataPath;

	Read();
	if(writeData)
	{
		Write();
	}
}

