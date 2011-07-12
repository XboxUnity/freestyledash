#include "stdafx.h"

#include "AutoUpdater.h"
#include "../PKZip/PkZip.h"
#include "../Managers/Version/VersionManager.h"
#include "../Managers/Drives/DrivesManager.h"
#include "../../Application/FreestyleApp.h"
#include "../Encryption/MD5/md5.h"

using namespace std;
	
AutoUpdater::AutoUpdater()
{
	DebugMsg("AutoUpdater", "AutoUpdater Initialized");
	m_StateNotifyUser = false;
	m_StateDownloading = false;

	bErrorOccurred = false;

	// Check For Updates
}

void AutoUpdater::add(iUpdateDisplayer& ref)
{
	_observers.insert(item::value_type(&ref,&ref));
}

void AutoUpdater::remove(iUpdateDisplayer& ref)
{
	_observers.erase(&ref);
}

void AutoUpdater::_notifyStatusUpdate(string szStatus)
{
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
		it->first->handleStatusUpdate( szStatus );
}

void AutoUpdater::_notifyLoadingComplete( void )
{
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
		it->first->handleLoadingComplete();
}

void AutoUpdater::_notifyDownloadFinished( int nItem )
{
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
		it->first->handleDownloadFinished( nItem );
}

HttpItem* AutoUpdater::getAutoupdateXeXHttpItem(string fileName)
{
	HttpItem* retVal = NULL;
	string dataPath = getAutoupdaterTempXexPath(true);
	DebugMsg("Content","Getting Xex from %s",fileName.c_str());
	retVal = HTTPDownloader::getInstance().CreateHTTPItem((CHAR*)fileName.c_str(), (CHAR*)dataPath.c_str());
	retVal->setTag("UpdatedXEX");
	retVal->SetPriority(HTTP_PRIORITY_HIGH);
	return retVal;
}

HttpItem* AutoUpdater::getAutoupdateChangeLogHttpItem(string fileName)
{
	HttpItem * retVal = NULL;
	string dataPath = getAutoupdaterTempChangeLogPath(true);
	DebugMsg("Content","Getting ChangeLog from %s", fileName.c_str());
	retVal = HTTPDownloader::getInstance().CreateHTTPItem((CHAR*)fileName.c_str(), (CHAR*)dataPath.c_str());
	retVal->setTag("UpdatedLOG");
	retVal->SetPriority(HTTP_PRIORITY_HIGH);
	return retVal;
}

HttpItem* AutoUpdater::getAutoupdateSkinHttpItem(string fileName)
{
	HttpItem* retVal = NULL;
	string dataPath = getAutoupdaterTempSkinPath(true);
	DebugMsg("Content","Getting Skin from %s",fileName.c_str());
	retVal = HTTPDownloader::getInstance().CreateHTTPItem((CHAR*)fileName.c_str(), (CHAR*)dataPath.c_str());
	retVal->setTag("UpdatedSKIN");
	retVal->SetPriority(HTTP_PRIORITY_HIGH);

	return retVal;
}

HttpItem* AutoUpdater::getAutoupdatePluginHttpItem(string fileName)
{
	HttpItem* retVal = NULL;
	string dataPath = getAutoupdaterTempPluginPath(true);
	DebugMsg("Content","Getting Skin from %s",fileName.c_str());
	retVal = HTTPDownloader::getInstance().CreateHTTPItem((CHAR*)fileName.c_str(), (CHAR*)dataPath.c_str());
	retVal->setTag("UpdatedPLUGIN");
	retVal->SetPriority(HTTP_PRIORITY_HIGH);

	return retVal;
}

HttpItem* AutoUpdater::getAutoupdateXmlHttpItem()
{
	return NULL;
}

string AutoUpdater::getAutoupdaterTempChangeLogPath(bool erase)
{
	string dataPath = SETTINGS::getInstance().getDataPath();
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"TEMP");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(!FileExistsA(dataPath))
	{
		//  DebugMsg("Creating %s",dataPath.c_str());
		_mkdir(dataPath.c_str());
	}
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"UpdateChangeLog.txt");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(FileExistsA(dataPath) && erase)
	{
		unlink(dataPath.c_str());
	}
	return dataPath;
}

string AutoUpdater::getAutoupdaterTempXexVerPath(bool erase)
{
	string dataPath = SETTINGS::getInstance().getDataPath();
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"TEMP");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(!FileExistsA(dataPath))
	{
		//  DebugMsg("Creating %s",dataPath.c_str());
		_mkdir(dataPath.c_str());
	}
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"NewDashVersion.ver");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(FileExistsA(dataPath) && erase)
	{
		unlink(dataPath.c_str());
	}
	return dataPath;
}
string AutoUpdater::getAutoupdaterTempXexPath(bool erase)
{
	string dataPath = SETTINGS::getInstance().getDataPath();
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"TEMP");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(!FileExistsA(dataPath))
	{
		//  DebugMsg("Creating %s",dataPath.c_str());
		_mkdir(dataPath.c_str());
	}
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"NewDashVersion.bin");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(FileExistsA(dataPath) && erase)
	{
		unlink(dataPath.c_str());
	}
	return dataPath;

}
string AutoUpdater::getAutoupdaterTempSkinVerPath(bool erase)
{
	string dataPath = SETTINGS::getInstance().getDataPath();
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"TEMP");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(!FileExistsA(dataPath))
	{
		//  DebugMsg("Creating %s",dataPath.c_str());
		_mkdir(dataPath.c_str());
	}
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"NewSkinVersion.ver");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(FileExistsA(dataPath) && erase)
	{
		unlink(dataPath.c_str());
	}
	return dataPath;

}

string AutoUpdater::getAutoupdaterTempSkinPath(bool erase)
{
	string dataPath = SETTINGS::getInstance().getDataPath();
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"TEMP");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(!FileExistsA(dataPath))
	{
		//  DebugMsg("Creating %s",dataPath.c_str());
		_mkdir(dataPath.c_str());
	}
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"NewSkinVersion.bin");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(FileExistsA(dataPath) && erase)
	{
		unlink(dataPath.c_str());
	}
	return dataPath;

}

string AutoUpdater::getAutoupdaterTempPluginVerPath(bool erase)
{
	string dataPath = SETTINGS::getInstance().getDataPath();
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"TEMP");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(!FileExistsA(dataPath))
	{
		//  DebugMsg("Creating %s",dataPath.c_str());
		_mkdir(dataPath.c_str());
	}
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"NewPluginVersion.ver");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(FileExistsA(dataPath) && erase)
	{
		unlink(dataPath.c_str());
	}
	return dataPath;

}	

string AutoUpdater::getAutoupdaterTempPluginPath(bool erase)
{
	string dataPath = SETTINGS::getInstance().getDataPath();
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"TEMP");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(!FileExistsA(dataPath))
	{
		//  DebugMsg("Creating %s",dataPath.c_str());
		_mkdir(dataPath.c_str());
	}
	dataPath = sprintfaA("%s\\%s",dataPath.c_str(),"NewPluginVersion.bin");
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(FileExistsA(dataPath) && erase)
	{
		unlink(dataPath.c_str());
	}
	return dataPath;

}	

string AutoUpdater::getAutoupdaterXmlPath()
{
	return "";
}


HRESULT AutoUpdater::DownloadQueuedItems( ListGroup dashQueue, ListGroup skinQueue, ListGroup pluginQueue)
{
	DownloadPacket * dlpack;

	if(m_dlRequest.size() > 0)
	{
		_notifyStatusUpdate("Updates are Currently Being Downloaded");
		return S_FALSE;
	}

	XNotifyQueueUICustom(L"Updates have been added to the download queue.");

	// Loop through the dash downloads and add to queue for download
	int nDashCount = dashQueue.nListItemCount;

	for(int i = 0; i < nDashCount; i++)
	{
		dlpack = new DownloadPacket;
		int nCurrentMapIdx = dashQueue.listItems.at(i).nMapIndex;
		string szFileName = xmlDoc.xmlDescriptors[nCurrentMapIdx].szFileName;
		HttpItem * itm = getAutoupdateXeXHttpItem(szFileName);
		itm->setRequester(this);
		HTTPDownloader::getInstance().AddToQueue(itm);
		dlpack->nListIndex = dashQueue.listItems.at(i).nListIndex;
		dlpack->nMapIndex = nCurrentMapIdx;
		m_dlRequest.insert(map<HttpItem*, DownloadPacket>::value_type(itm, *dlpack));
		delete dlpack;
	}

	int nSkinCount = skinQueue.nListItemCount;
	
	for(int i = 0; i < nSkinCount; i++)
	{
		dlpack = new DownloadPacket;
		int nCurrentMapIdx = skinQueue.listItems.at(i).nMapIndex;
		string szFileName = xmlDoc.xmlDescriptors[nCurrentMapIdx].szFileName;
		HttpItem * itm = getAutoupdateSkinHttpItem(szFileName);
		itm->setRequester(this);
		HTTPDownloader::getInstance().AddToQueue(itm);
		dlpack->nListIndex = skinQueue.listItems.at(i).nListIndex;
		dlpack->nMapIndex = nCurrentMapIdx;
		m_dlRequest.insert(map<HttpItem*, DownloadPacket>::value_type(itm, *dlpack));
		delete dlpack;
	}

	int nPluginCount = pluginQueue.nListItemCount;

	for(int i = 0; i < nPluginCount; i++)
	{
		dlpack = new DownloadPacket;
		int nCurrentMapIdx = pluginQueue.listItems.at(i).nMapIndex;
		string szFileName = xmlDoc.xmlDescriptors[nCurrentMapIdx].szFileName;
		HttpItem * itm = getAutoupdatePluginHttpItem(szFileName);
		itm->setRequester(this);
		HTTPDownloader::getInstance().AddToQueue(itm);
		dlpack->nListIndex = pluginQueue.listItems.at(i).nListIndex;
		dlpack->nMapIndex = nCurrentMapIdx;
		m_dlRequest.insert(map<HttpItem*, DownloadPacket>::value_type(itm, *dlpack));
		delete dlpack;
	}

	m_StateDownloading = true;
	_notifyStatusUpdate("Downloads have been added to the download queue.");

	return S_OK;
}

HRESULT AutoUpdater::DownloadCurrentXML( void )
{	
	if(!SETTINGS::getInstance().getDataPath().empty())
	{
		//Add update xml to download queue for version checking
		HttpItem * xmlItem = getAutoupdateXmlHttpItem();
		xmlItem->setRequester(this);
		HTTPDownloader::getInstance().AddToQueue(xmlItem);
		return S_OK;
	}
	return S_FALSE;
}

HRESULT AutoUpdater::CheckForUpdates( bool notifyUser )
{
	if (!SETTINGS::getInstance().getDataPath().empty())
	{
		m_StateNotifyUser = notifyUser;
		// Add update xml to download queue for version checking

		_notifyStatusUpdate("Requesting Update Information");

		HttpItem* xmlItem = getAutoupdateXmlHttpItem();
		xmlItem->setRequester(this);
		HTTPDownloader::getInstance().AddToQueue(xmlItem);
		return S_OK;
	}

	return S_FALSE;
}

HRESULT AutoUpdater::ReadSettings( void )
{
	// Fill variables for future use on first load
	bUpdatedXexNotice = SETTINGS::getInstance().getUpdateXexNotice() == 1 ? true : false;
	bUpdatedSkinNotice = SETTINGS::getInstance().getUpdateSkinNotice() == 1 ? true : false;
	bUpdatedPluginNotice = SETTINGS::getInstance().getUpdatePluginNotice() == 1 ? true : false;
	bUpdatedBetaNotice = SETTINGS::getInstance().getUpdateOnBeta() == 1 ? true : false;
	
	// Store Version Info 
	curDashVersion.nFileType = FILETYPE_DASH;
	curDashVersion.nMajorVersion = VersionManager::getInstance().getFSDDashVersion().Version.dwMajor;
	curDashVersion.nMinorVersion = VersionManager::getInstance().getFSDDashVersion().Version.dwMinor;
	curDashVersion.nRevisionNumber = VersionManager::getInstance().getFSDDashVersion().Version.dwRevision;
	curDashVersion.nVersionType = VersionManager::getInstance().getFSDDashVersion().Version.dwVersionType;

	// Extract Skin Version Info
	curSkinVersion.nFileType = FILETYPE_SKIN;
	curSkinVersion.nMajorVersion = VersionManager::getInstance().getFSDSkinVersion().Version.dwMajor;
	curSkinVersion.nMinorVersion = VersionManager::getInstance().getFSDSkinVersion().Version.dwMinor;
	curSkinVersion.nRevisionNumber = VersionManager::getInstance().getFSDSkinVersion().Version.dwRevision;
	curSkinVersion.nVersionType = VersionManager::getInstance().getFSDSkinVersion().Version.dwVersionType;

	// Extract Plugin Version Info
	curPluginVersion.nFileType = FILETYPE_PLUGIN;
	curPluginVersion.nMajorVersion = VersionManager::getInstance().getFSDPluginVersion().Version.dwMajor;
	curPluginVersion.nMinorVersion = VersionManager::getInstance().getFSDPluginVersion().Version.dwMinor;
	curPluginVersion.nRevisionNumber = VersionManager::getInstance().getFSDPluginVersion().Version.dwRevision;
	curPluginVersion.nVersionType = VersionManager::getInstance().getFSDPluginVersion().Version.dwVersionType;

	// All settings are initialized

	return S_OK;
}

HRESULT AutoUpdater::ClearTempFiles( void )
{
	string szTempXex = getAutoupdaterTempXexPath(false);
	string szTempSkin = getAutoupdaterTempSkinPath(false);
	string szTempPlugin = getAutoupdaterTempPluginPath(false);

	DebugMsg("AutoUpdater", "Clearing Temporary Files");

	if(FileExistsA(szTempXex))
		_unlink(szTempXex.c_str());

	if(FileExistsA(szTempSkin))
		_unlink(szTempSkin.c_str());

	if(FileExistsA(szTempPlugin))
		_unlink(szTempPlugin.c_str());

	return S_OK;	
}

string AutoUpdater::ConvertTypeToString( int nType )
{
	switch (nType)
	{
	case VERSION_ALPHA:
		return "- Alpha";
	case VERSION_BETA:
		return "- Beta";
	case VERSION_RELEASE:
	case VERSION_UNDEFINED:
		return "";
	};

	return "";
}


HRESULT AutoUpdater::ClearXMLDocumentData( void )
{
	// Zero out Header Data
	xmlDoc.xmlHeader.szFilePath = "";

	xmlDoc.xmlHeader.docHeader.defName = "";
	xmlDoc.xmlHeader.docHeader.updDate = "";
	
	xmlDoc.xmlHeader.updHeader.dashTotal = 0;
	xmlDoc.xmlHeader.updHeader.skinTotal = 0;
	xmlDoc.xmlHeader.updHeader.pluginTotal = 0;
	
	// Clear all Descriptors
	xmlDoc.xmlDescriptors.clear();
	xmlDoc.xmlChangeLog.szDate = "";
	xmlDoc.xmlChangeLog.szName = "";
	xmlDoc.xmlChangeLog.szLogData = "";

	return S_OK;
}

bool AutoUpdater::VersionCompare(VersionDescriptor newVersion, VersionDescriptor curVersion, bool bIncludeRevs, bool bReleaseBuildOnly)
{
	if(bReleaseBuildOnly && newVersion.nVersionType != VERSION_RELEASE)
		return false;

	// Check to see if the received major version is larger than current
	if( newVersion.nMajorVersion > curVersion.nMajorVersion ){
		return true;
	}

	if( newVersion.nMajorVersion == curVersion.nMajorVersion )
	{
		// Check to see if the received minor version is larger than current
		if( newVersion.nMinorVersion > curVersion.nMinorVersion ){
			return true;
		}
	
		if( newVersion.nMinorVersion == curVersion.nMinorVersion )
		{
			// Check to see if user wants updated beta notifications
			if ( bIncludeRevs )
			{
				// Check to see if the received revision number is larger than current
				if (newVersion.nRevisionNumber > curVersion.nRevisionNumber ){
					return true;
				}
			}
		}
	}

	return false;
}


HRESULT AutoUpdater::ParseCurrentVersionXML(HttpItem *itm)
{
	// Add File Path to internal structure
	xmlDoc.xmlHeader.szFilePath = itm->getSaveAsPath();
	
	// Parse the file if it exists
	XMLReader * xmlFile = LoadConfigFile(xmlDoc.xmlHeader.szFilePath);

	// Read XML File and Fill in VersionDescriptors
	while(!EndAttribute(xmlFile, "Header"))
	{
		if(StartAttribute(xmlFile, "Document"))
		{	
			xmlDoc.xmlHeader.docHeader.defName = xmlFile->getAttributeValue("name");
			xmlDoc.xmlHeader.docHeader.updDate = xmlFile->getAttributeValue("updated");
		}

		if(StartAttribute(xmlFile, "UpdateList"))
		{
			xmlDoc.xmlHeader.updHeader.dashTotal = xmlFile->getAttributeValueAsInt("dash");
			xmlDoc.xmlHeader.updHeader.skinTotal = xmlFile->getAttributeValueAsInt("skins");
			xmlDoc.xmlHeader.updHeader.pluginTotal = xmlFile->getAttributeValueAsInt("plugins");
		}
	}

	DebugMsg("AutoUpdater", "Version Definition File:  Header Data Loaded");

	// Parse the dash update data and place into version descriptors
	xmlFile->restart();
	while(!EndAttribute(xmlFile, "DashUpdate"))
	{
		if(StartAttribute(xmlFile, "Dash"))
		{
			tempVersion = new VersionDescriptor;

			tempVersion->szName = xmlFile->getAttributeValue("name");
			tempVersion->nMajorVersion = xmlFile->getAttributeValueAsInt("major");
			tempVersion->nMinorVersion = xmlFile->getAttributeValueAsInt("minor");
			tempVersion->nRevisionNumber = xmlFile->getAttributeValueAsInt("rev");
			tempVersion->nVersionType = xmlFile->getAttributeValueAsInt("type");
			tempVersion->szFileName = xmlFile->getAttributeValue("file");
			tempVersion->nUpdateID = xmlFile->getAttributeValueAsInt("uid");
			tempVersion->nFileType = FILETYPE_DASH;

			// Insert data into xmlDoc version descriptor map
			xmlDoc.xmlDescriptors.insert(map<int, VersionDescriptor>::value_type(tempVersion->nUpdateID, *tempVersion));
			delete tempVersion;
		}
	}
	DebugMsg("AutoUpdater", "Version Definition File:  Dash Data Loaded: %d", xmlDoc.xmlHeader.updHeader.dashTotal);
	// Parse the Skin update data and place into version descriptors
	xmlFile->restart();
	while(!EndAttribute(xmlFile, "SkinUpdate"))
	{
		if(StartAttribute(xmlFile, "Skin"))
		{
			tempVersion = new VersionDescriptor;

			tempVersion->szName = xmlFile->getAttributeValue("name");
			tempVersion->nMajorVersion = xmlFile->getAttributeValueAsInt("major");
			tempVersion->nMinorVersion = xmlFile->getAttributeValueAsInt("minor");
			tempVersion->nRevisionNumber = xmlFile->getAttributeValueAsInt("rev");
			tempVersion->nVersionType = xmlFile->getAttributeValueAsInt("type");
			tempVersion->szFileName = xmlFile->getAttributeValue("file");
			tempVersion->nUpdateID = xmlFile->getAttributeValueAsInt("uid");
			tempVersion->nFileType = FILETYPE_SKIN;

			// Insert data into xmlDoc version descriptor map
			xmlDoc.xmlDescriptors.insert(map<int, VersionDescriptor>::value_type(tempVersion->nUpdateID, *tempVersion));
			delete tempVersion;
		}
	}
	
	DebugMsg("AutoUpdater", "Version Definition File:  Skin Data Loaded: %d", xmlDoc.xmlHeader.updHeader.skinTotal);
	// Parse the Skin update data and place into version descriptors
	xmlFile->restart();
	while(!EndAttribute(xmlFile, "PluginUpdate"))
	{
		if(StartAttribute(xmlFile, "Plugin"))
		{
			tempVersion = new VersionDescriptor;

			tempVersion->szName = xmlFile->getAttributeValue("name");
			tempVersion->nMajorVersion = xmlFile->getAttributeValueAsInt("major");
			tempVersion->nMinorVersion = xmlFile->getAttributeValueAsInt("minor");
			tempVersion->nRevisionNumber = xmlFile->getAttributeValueAsInt("rev");
			tempVersion->nVersionType = xmlFile->getAttributeValueAsInt("type");
			tempVersion->szFileName = xmlFile->getAttributeValue("file");
			tempVersion->nUpdateID = xmlFile->getAttributeValueAsInt("uid");
			tempVersion->nFileType = FILETYPE_PLUGIN;

			// Insert data into xmlDoc version descriptor map
			xmlDoc.xmlDescriptors.insert(map<int, VersionDescriptor>::value_type(tempVersion->nUpdateID, *tempVersion));
			delete tempVersion;
		}
	}

	DebugMsg("AutoUpdater", "Version Definition File:  Plugin Data Loaded: %d", xmlDoc.xmlHeader.updHeader.pluginTotal);
	// Parse the ChangeLog update data and place into version descriptors
	
	xmlFile->restart();
	tempChangelog = new ChangelogDescriptor;
	while(!EndAttribute(xmlFile, "Update"))
	{
		
		if(StartAttribute(xmlFile, "Changelog"))
		{
			tempChangelog->szName = xmlFile->getAttributeValue("name");
			tempChangelog->szDate = xmlFile->getAttributeValue("date");

			string key, val;

			while (!EndAttribute(xmlFile, "Changelog"))
			{
				key = xmlFile->getNodeName();
				key = xmlFile->getNodeData();

				if (StartAttribute(xmlFile, key.c_str()))
				{
					val = xmlFile->getSubNodeData();
					if(strcmp(make_lowercaseA(key).c_str(), "data")==0) {
						tempChangelog->szLogData = val;
					}
				}
			}
		}
	}

	// Insert data into xmlDoc version descriptor map
	xmlDoc.xmlChangeLog = *tempChangelog;
	delete tempChangelog;

	DebugMsg("AutoUpdater", "Version Definition File:  Plugin Data Loaded: %d", xmlDoc.xmlHeader.updHeader.pluginTotal);
	return S_OK;
}

void AutoUpdater::HandleUserNotification( void )
{
	vector<VersionDescriptor> dashVersions;
	vector<VersionDescriptor> skinVersions;
	vector<VersionDescriptor> pluginVersions;

	// Cycle through colloection of downloaded updates and split into vectors
	int nTotalCount = xmlDoc.xmlDescriptors.size();
	DebugMsg("AutoUpdater", "TotalCount:  %d", nTotalCount);
	for(int i = 0; i < nTotalCount; i++)
	{
		if(xmlDoc.xmlDescriptors[i].nFileType == FILETYPE_DASH)
			dashVersions.push_back(xmlDoc.xmlDescriptors[i]);

		if(xmlDoc.xmlDescriptors[i].nFileType == FILETYPE_SKIN)
			skinVersions.push_back(xmlDoc.xmlDescriptors[i]);

		if(xmlDoc.xmlDescriptors[i].nFileType == FILETYPE_PLUGIN)
			pluginVersions.push_back(xmlDoc.xmlDescriptors[i]);
	}

	bool bResult = false;
	// Cycle through each set to see if a new update is available
	int nPluginCount = pluginVersions.size();
	DebugMsg("AutoUpdater", "Plugin Count:  %d", nPluginCount);
	for(int i = 0; i < nPluginCount; i++)
	{
		bResult = VersionCompare(pluginVersions.at(i), curPluginVersion, true, !bUpdatedBetaNotice);
		if(bResult == true) // if we found a new version, no need to search further
			break;
	}

	// If the user wants dash updates, display notification
	if(bUpdatedPluginNotice == true && bResult == true)
		XNotifyQueueUICustom(&updStringC[0]);

	bResult = false;
	// Cycle through each set to see if a new update is available
	int nSkinCount = skinVersions.size();
	DebugMsg("AutoUpdater", "Skin Count:  %d", nSkinCount);
	for(int i = 0; i < nSkinCount; i++)
	{
		bResult = VersionCompare(skinVersions.at(i), curSkinVersion, true, !bUpdatedBetaNotice);
		if(bResult == true) // if we found a new version, no need to search further
			break;
	}

	// If the user wants dash updates, display notification
	if(bUpdatedSkinNotice == true && bResult == true)
		XNotifyQueueUICustom(&updStringB[0]);

	bResult = false;
	// Cycle through each set to see if a new update is available
	int nDashCount = dashVersions.size();
	DebugMsg("AutoUpdater", "Dash Count:  %d", nDashCount);
	for(int i = 0; i < nDashCount; i++)
	{
		bResult = VersionCompare(dashVersions.at(i), curDashVersion, true, !bUpdatedBetaNotice);
		if(bResult == true) // if we found a new version, no need to search further
			break;
	}

	// If the user wants dash updates, display notification
	if(bUpdatedXexNotice == true && bResult == true)
		XNotifyQueueUICustom(&updStringA[0]);

}

ListGroup AutoUpdater::CreateDashListMap( bool bIncludeBetaBuilds )
{
	ListGroup retList;
	ListItem listItem;
	int nListCounter = 0;

	// Cycle through colloection of downloaded updates and split into vectors
	int nTotalCount = xmlDoc.xmlDescriptors.size();
	DebugMsg("AutoUpdater", "TotalCount:  %d", nTotalCount);
	for(int i = 0; i < nTotalCount; i++)
	{
		if(xmlDoc.xmlDescriptors[i].nFileType == FILETYPE_DASH)
		{
			if(VersionCompare(xmlDoc.xmlDescriptors[i], curDashVersion, true, !bIncludeBetaBuilds))
			{
				string szDisplayString = sprintfaA("%s Update. Version: %d.%d r%d %s", 
					xmlDoc.xmlDescriptors[i].szName.c_str(),
					xmlDoc.xmlDescriptors[i].nMajorVersion,
					xmlDoc.xmlDescriptors[i].nMinorVersion,
					xmlDoc.xmlDescriptors[i].nRevisionNumber,
					ConvertTypeToString(xmlDoc.xmlDescriptors[i].nVersionType).c_str()
				);

				listItem.nFileType = FILETYPE_DASH;
				listItem.nListIndex = nListCounter;
				listItem.nMapIndex = i;
				listItem.szDisplayTitles = szDisplayString;

				retList.listItems.push_back(listItem);

				nListCounter++;
			}
			// Item in List was not a newer version
		}
		// Item in list was not of FILETYPE_DASH
	}

	// Create an quick access variable for quantity
	retList.nListItemCount = retList.listItems.size();

	return retList;
}

ListGroup AutoUpdater::CreateSkinListMap( bool bIncludeBetaBuilds )
{
	ListGroup retList;
	ListItem listItem;
	int nListCounter = 0;

	// Cycle through colloection of downloaded updates and split into vectors
	int nTotalCount = xmlDoc.xmlDescriptors.size();
	DebugMsg("AutoUpdater", "Item:  %d", xmlDoc.xmlDescriptors[0].nRevisionNumber);
	DebugMsg("AutoUpdater", "TotalCount:  %d", nTotalCount);
	for(int i = 0; i < nTotalCount; i++)
	{
		if(xmlDoc.xmlDescriptors[i].nFileType == FILETYPE_SKIN)
		{
			if(VersionCompare(xmlDoc.xmlDescriptors[i], curSkinVersion, true, !bIncludeBetaBuilds))
			{
				string szDisplayString = sprintfaA("%s Update. Version: %d.%d r%d %s", 
					xmlDoc.xmlDescriptors[i].szName.c_str(),
					xmlDoc.xmlDescriptors[i].nMajorVersion,
					xmlDoc.xmlDescriptors[i].nMinorVersion,
					xmlDoc.xmlDescriptors[i].nRevisionNumber,
					ConvertTypeToString(xmlDoc.xmlDescriptors[i].nVersionType).c_str()
				);

				listItem.nFileType = FILETYPE_SKIN;
				listItem.nListIndex = nListCounter;
				listItem.nMapIndex = i;
				listItem.szDisplayTitles = szDisplayString;

				retList.listItems.push_back(listItem);
				
				nListCounter++;
			}
			// Item in List was not a newer version
		}
		// Item in list was not of FILETYPE_SKIN
	}

	// Create an quick access variable for quantity
	retList.nListItemCount = retList.listItems.size();

	return retList;
}

ListGroup AutoUpdater::CreatePluginListMap( bool bIncludeBetaBuilds )
{
	ListGroup retList;
	ListItem listItem;
	int nListCounter = 0;

	// Cycle through colloection of downloaded updates and split into vectors
	int nTotalCount = xmlDoc.xmlDescriptors.size();
	DebugMsg("AutoUpdater", "TotalCount:  %d", nTotalCount);
	for(int i = 0; i < nTotalCount; i++)
	{
		if(xmlDoc.xmlDescriptors[i].nFileType == FILETYPE_PLUGIN)
		{
			if(VersionCompare(xmlDoc.xmlDescriptors[i], curPluginVersion, true, !bIncludeBetaBuilds))
			{
				string szDisplayString = sprintfaA("%s Update. Version: %d.%d r%d %s", 
					xmlDoc.xmlDescriptors[i].szName.c_str(),
					xmlDoc.xmlDescriptors[i].nMajorVersion,
					xmlDoc.xmlDescriptors[i].nMinorVersion,
					xmlDoc.xmlDescriptors[i].nRevisionNumber,
					ConvertTypeToString(xmlDoc.xmlDescriptors[i].nVersionType).c_str()
				);

				listItem.nFileType = FILETYPE_PLUGIN;
				listItem.nListIndex = nListCounter;
				listItem.nMapIndex = i;
				listItem.szDisplayTitles = szDisplayString;

				retList.listItems.push_back(listItem);

				nListCounter++;
			}
			// Item in List was not a newer version
		}
		// Item in list was not of FILETYPE_PLUGIN
	}

	// Create an quick access variable for quantity
	retList.nListItemCount = retList.listItems.size();

	return retList;

}

ListGroup AutoUpdater::getListMap(int FileType)
{
	switch (FileType)
	{
	case FILETYPE_DASH:
		return dashList;
	case FILETYPE_SKIN:
		return skinList;
	case FILETYPE_PLUGIN:
		return pluginList;
	};

	return pluginList;
}

void AutoUpdater::DownloadCompleted(HttpItem* itm)
{
	DebugMsg("AutoUpdater", "Download Completed");
	if(itm->getResponseCode() == 200)
	{
		if(strcmp(itm->getTag().c_str(),"UpdatedXEX") == 0)
		{
			_notifyStatusUpdate("Dash Update Successfully downloaded");
			// Create .ver file with download
			int nCurrentMapIdx = m_dlRequest[itm].nMapIndex;
			VERSION_INFO verInfo;

			VersionManager::getInstance().CreateNullVersionInfo(&verInfo);

			verInfo.Version.dwMajor = xmlDoc.xmlDescriptors[nCurrentMapIdx].nMajorVersion;
			verInfo.Version.dwMinor = xmlDoc.xmlDescriptors[nCurrentMapIdx].nMinorVersion;
			verInfo.Version.dwRevision = xmlDoc.xmlDescriptors[nCurrentMapIdx].nRevisionNumber;
			verInfo.Version.dwVersionType = xmlDoc.xmlDescriptors[nCurrentMapIdx].nVersionType;

			string szVersionFile = getAutoupdaterTempXexVerPath(true);
			
			VersionManager::getInstance().WriteVersionToFile(szVersionFile, verInfo);
			m_dlRequest.erase(itm);
		}
		if(strcmp(itm->getTag().c_str(), "UpdatedSKIN") == 0)
		{
			_notifyStatusUpdate("Skin Update Successfully downloaded");
			int nCurrentMapIdx = m_dlRequest[itm].nMapIndex;
			VERSION_INFO verInfo;

			VersionManager::getInstance().CreateNullVersionInfo(&verInfo);

			verInfo.Version.dwMajor = xmlDoc.xmlDescriptors[nCurrentMapIdx].nMajorVersion;
			verInfo.Version.dwMinor = xmlDoc.xmlDescriptors[nCurrentMapIdx].nMinorVersion;
			verInfo.Version.dwRevision = xmlDoc.xmlDescriptors[nCurrentMapIdx].nRevisionNumber;
			verInfo.Version.dwVersionType = xmlDoc.xmlDescriptors[nCurrentMapIdx].nVersionType;

			string szVersionFile = getAutoupdaterTempSkinVerPath(true);
			
			VersionManager::getInstance().WriteVersionToFile(szVersionFile, verInfo);
			m_dlRequest.erase(itm);
		}
		if(strcmp(itm->getTag().c_str(), "UpdatedPLUGIN") == 0)
		{
			_notifyStatusUpdate("Plug-In Update Successfully downloaded");
			int nCurrentMapIdx = m_dlRequest[itm].nMapIndex;
			VERSION_INFO verInfo;

			VersionManager::getInstance().CreateNullVersionInfo(&verInfo);

			verInfo.Version.dwMajor = xmlDoc.xmlDescriptors[nCurrentMapIdx].nMajorVersion;
			verInfo.Version.dwMinor = xmlDoc.xmlDescriptors[nCurrentMapIdx].nMinorVersion;
			verInfo.Version.dwRevision = xmlDoc.xmlDescriptors[nCurrentMapIdx].nRevisionNumber;
			verInfo.Version.dwVersionType = xmlDoc.xmlDescriptors[nCurrentMapIdx].nVersionType;

			string szVersionFile = getAutoupdaterTempPluginVerPath(true);
			
			VersionManager::getInstance().WriteVersionToFile(szVersionFile, verInfo);
			m_dlRequest.erase(itm);
		}
		if(strcmp(itm->getTag().c_str(),"AutoUpdateXML") == 0)
		{
			if(m_StateNotifyUser == true)
			{
				// File download requested for notification only
				ReadSettings();
				ClearXMLDocumentData();
				ParseCurrentVersionXML(itm);
				HandleUserNotification();
				m_StateNotifyUser = false;
			}
			else
			{
				_notifyStatusUpdate("Version Definition File Download Completed");
				m_StateListsLoaded = false;
				ReadSettings();
				ClearXMLDocumentData();
				ParseCurrentVersionXML(itm);

				// Create precalculated lists for updater scene
				dashList = CreateDashListMap(bUpdatedBetaNotice);
				skinList = CreateSkinListMap(bUpdatedBetaNotice);
				pluginList = CreatePluginListMap(bUpdatedBetaNotice);
				m_StateListsLoaded = true;

				_notifyStatusUpdate("Enumerating Lists");
				_notifyLoadingComplete();
				
				int nUpdateCount = dashList.nListItemCount + skinList.nListItemCount + pluginList.nListItemCount;
				string szCaption = sprintfaA("Found %d New Updates", nUpdateCount);
				_notifyStatusUpdate(szCaption);

			}

		}
	}
	else
	{
		if(strcmp(itm->getTag().c_str(),"UpdatedXEX") == 0)
		{
			_notifyStatusUpdate("There was a problem downloading dash update.");
			bErrorOccurred = true;
			m_dlRequest.erase(itm);
		}
		if(strcmp(itm->getTag().c_str(), "UpdatedSKIN") == 0)
		{
			_notifyStatusUpdate("There was a problem downloading skin update.");
			bErrorOccurred = true;
			m_dlRequest.erase(itm);
		}
		if(strcmp(itm->getTag().c_str(), "UpdatedPLUGIN") == 0)
		{
			_notifyStatusUpdate("There was a problem downloading plug-in update.");
			bErrorOccurred = true;
			m_dlRequest.erase(itm);
		}
		if(strcmp(itm->getTag().c_str(),"AutoUpdateXML") == 0)
		{
			_notifyStatusUpdate("Could not download version description file.");
			return;
		}
	}

	int nSize = m_dlRequest.size();
	if(nSize == 0 && m_StateDownloading)
	{
		m_StateDownloading = false;
		bErrorOccurred = false;
		InitializeInstaller();

		if(bErrorOccurred){
			_notifyStatusUpdate("Error- not all downloads completed.  Restart to install.");
			XNotifyQueueUICustom(L"Downloading Complete.  Some updates did not download properly.");

		}
		else
		{
			_notifyStatusUpdate("All downloads operations complete.  Restart to Install.");
//			XNotifyQueueUICustom(L"Updates Downloaded- Restart Dash to Install.");
			PromptToRestart();
		}		
		
	}
}

void AutoUpdater::InitializeInstaller( void )
{
	DebugMsg("AutoUpdater", "Initializing Installer");
	SETTINGS::getInstance().setInstallerState(1);
}

void AutoUpdater::RunInstaller( void )
{
	// Create predefined copy paths to easily change later
	string szDashPath = "game:\\";
	string szSkinPath = "game:\\skins\\";
	string szPluginPath = "game:\\plugins\\";
	
	// Create directories in case they don't exist
	CreateDirectory(szDashPath.c_str(), NULL);
	CreateDirectory(szSkinPath.c_str(), NULL);
	CreateDirectory(szPluginPath.c_str(), NULL);

	// Release current Direct 3D Device and initialize console view
	CFreestyleApp::getInstance().m_pd3dDevice->Release();
	ATG::Console console;

	DebugMsg("AutoUpdater", "Installer Running");
	
	// Locate the embedded font file in memory
	DWORD dwStoredDigest;
	VOID * pSectionData;

	
	BOOL retVal = XGetModuleSection( GetModuleHandle(NULL), "FSDFont", &pSectionData, &dwStoredDigest);
	if(retVal == FALSE)
	{
		// Font Section could not be found within Xex file- compiled wrong?
		DebugMsg("AutoUpdater", "Font File Could Not Be Extracted.");
		XNotifyQueueUICustom(L"Font Not Found - Update Manually");
		RestartDashboard(&console, true);
		return;
	}


	// Dump the file to a temporary file for use in this updater
	string szFontPath = "game:\\ConsoleFont.xpr";
	FILE * fHandle;
	fopen_s(&fHandle, szFontPath.c_str(), "wb");
	fwrite(pSectionData, dwStoredDigest, 1, fHandle);
	fclose(fHandle);
	
	// Verify that the font file was dumped and the file exists
	if(!FileExistsA(szFontPath))
	{
			XNotifyQueueUICustom(L"Font Not Found - Update Manually");
			RestartDashboard(&console, true);
			return;
	}

	// Font Extracted Fine- continue with updater.
	console.Create(szFontPath.c_str(), 0xFF000000, 0xFFFFFFFF );
	console.Clear();

	// Create and Display Intro along with all the version information...
	console.Format("Welcome to the Freestyle Dash Update Installer. (v.01a)\n\n");

	// Scan tempfolder for updates
	console.Format("** Checking system for downloaded updates....");
	bool hasDashFile = false, hasSkinFile = false, hasPluginFile = false;
	if(FileExistsA(getAutoupdaterTempXexPath(false))) hasDashFile = true;
	if(FileExistsA(getAutoupdaterTempSkinPath(false))) hasSkinFile = true;
	if(FileExistsA(getAutoupdaterTempPluginPath(false))) hasPluginFile = true;
	
	// Scan tempfolder for version files for updates that were found
	if(hasDashFile)
	{
		// Check Version File for display information
		string szVerFile = getAutoupdaterTempXexVerPath(false);
		string szUpdateDisplay = sprintfaA("\n\tFreestyle Dash Update Found ( Version: Unknown )");
		if(FileExistsA(szVerFile))
		{
			// File exists, so extract version information
			VERSION_DATA verInfo;
			FILE * fHandle;
			fopen_s(&fHandle, szVerFile.c_str(), "rb");
			fread(&verInfo, sizeof(VERSION_DATA), 1, fHandle);
			fclose(fHandle);
			szUpdateDisplay = sprintfaA("\n\tFreestyle Dash Update Found ( Version: %d.%d r%d %s )", verInfo.dwMajor, verInfo.dwMinor, verInfo.dwRevision, ConvertTypeToString(verInfo.dwVersionType).c_str());
		}
		// Output results to screen
		console.Format(szUpdateDisplay.c_str());
	}

	if(hasSkinFile)
	{
		// Check Version File for display information
		string szVerFile = getAutoupdaterTempSkinVerPath(false);
		string szUpdateDisplay = sprintfaA("\n\tFreestyle Skin Update Found ( Version: Unknown )");
		if(FileExistsA(szVerFile))
		{
			// File exists, so extract version information
			VERSION_DATA verInfo;
			FILE * fHandle;
			fopen_s(&fHandle, szVerFile.c_str(), "rb");
			fread(&verInfo, sizeof(VERSION_DATA), 1, fHandle);
			fclose(fHandle);
			szUpdateDisplay = sprintfaA("\n\tFreestyle Skin Update Found ( Version: %d.%d r%d %s )", verInfo.dwMajor, verInfo.dwMinor, verInfo.dwRevision, ConvertTypeToString(verInfo.dwVersionType).c_str());
		}
		// Output results to screen
		console.Format(szUpdateDisplay.c_str());
	}

	if(hasPluginFile)
	{
		// Check Version File for display information
		string szVerFile = getAutoupdaterTempPluginVerPath(false);
		string szUpdateDisplay = sprintfaA("\n\tFreestyle Plugin Update Found ( Version: Unknown )");
		if(FileExistsA(szVerFile))
		{
			// File exists, so extract version information
			VERSION_DATA verInfo;
			FILE * fHandle;
			fopen_s(&fHandle, szVerFile.c_str(), "rb");
			fread(&verInfo, sizeof(VERSION_DATA), 1, fHandle);
			fclose(fHandle);
			szUpdateDisplay = sprintfaA("\n\tFreestyle Plugin Update Found ( Version: %d.%d r%d %s )", verInfo.dwMajor, verInfo.dwMinor, verInfo.dwRevision, ConvertTypeToString(verInfo.dwVersionType).c_str());
		}
		// Output results to screen
		console.Format(szUpdateDisplay.c_str());
	}

	console.Format("\n\nTo Begin Installation - Press the 'Y' Button.");
	console.Format("\nTo Exit Installation - Press the 'Back' Button.");

	// Wait for Button Input to continue
	for( ; ; ) {
		ATG::Input::GetMergedInput();
		if(ATG::Input::m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_Y )
			break;
		if(ATG::Input::m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_BACK )
			RestartDashboard(&console, true);
	}

	// ExtractZip Files To Correct Location
	bool dashResult = false, skinResult = false, pluginResult = false;
	bool bError = false;
	if(hasDashFile)
	{
		dashResult = InstallDashUpdate(&console, szDashPath);
		if(!dashResult){
			bError = true;
			console.Format("\n An Error Occurred installing Dash Updates");
		}
	}
	if(hasSkinFile)
	{
		skinResult = InstallSkinUpdate(&console, szSkinPath);
		if(!skinResult){
			bError = true;
			console.Format("\n An Error Occurred installing Skin Updates");
		}
	}
	if(hasPluginFile)
	{
		pluginResult = InstallPluginUpdate(&console, szPluginPath);
		if(!pluginResult){
			bError = true;
			console.Format("\n An Error Occurred installing Plugin Updates");
		}
	}

	// Show Completion Message
	if(!bError)
	{
		console.Format("\n\nUpdate has completed successfully.");
	}
	else
	{
		console.Format("\n\nUpdate has not completed successfully- please update manually");
	}
	console.Format("\nPress Any Key to Reboot Dashboard.");

	// Reboot with changed files
	for( ; ; ) {
		ATG::Input::GetMergedInput();
		if(ATG::Input::m_DefaultGamepad.wPressedButtons )
			RestartDashboard(&console, true);
	}
	
	// Serious Error- should never get to this code - if we do, just restart.
}

void AutoUpdater::RestartDashboard(ATG::Console * console, bool resetFlag)
{
	if(resetFlag)
		SETTINGS::getInstance().setInstallerState(0);

	console->Format("\n\nRestarting...");
	Sleep(500);
	console->Clear();
	if(FileExistsA("game:\\ConsoleFont.xpr"))
		_unlink("game:\\ConsoleFont.xpr");

	Restart();
}

void AutoUpdater::PromptToRestart()
{

	XOVERLAPPED overlapped;
	MESSAGEBOX_RESULT mbresult;
	HANDLE hEvent = WSACreateEvent();
	memset(&overlapped,0,sizeof(overlapped));
	overlapped.hEvent = hEvent;
	memset(&mbresult,0,sizeof(mbresult));
	LPCWSTR btnOptions[3] = {L"Restart FSD", L"Return To NXE",L"Update Later"};
	XShowMessageBoxUI(XUSER_INDEX_ANY,L"Updates Completed",L"The updates have completed downloading.  Please restart Dash to apply.",3,btnOptions,0,XMB_ALERTICON,&mbresult,&overlapped);
	WaitForSingleObject( hEvent, INFINITE );
	
	
	WSAResetEvent( hEvent);
	
	if(mbresult.dwButtonPressed == 0)
	{
		DebugMsg("Debug","Restart pressed");
		Restart();
	}
	else
	{
		if(mbresult.dwButtonPressed == 1)
		{
			BackToNXE();
		}
		else
		{
			if(mbresult.dwButtonPressed == 2)
			{

			}
			else
			{
				DebugMsg("Debug","Nothing choosed, restart");
				BackToNXE();
			}
		}
	}

}

bool AutoUpdater::InstallDashUpdate(ATG::Console * console, string szDestination)
{
	HRESULT hr = NULL;
	PkZip zipFile;

	console->Format("\n\n Extracting and Installing Dashboard Updates...");
	string szExistingPath = getAutoupdaterTempXexPath(false);

	// Rename the original as a .old file
	string szExecutablePath = DrivesManager::getInstance().getExecutablePath();
	string szExecutableDirectory = szExecutablePath.substr(0, szExecutablePath.find_last_of("\\") + 1);
	string szFileNameWithExt = szExecutablePath.substr(szExecutablePath.find_last_of("\\") + 1);
	string szFileName = szFileNameWithExt.substr(0, szFileNameWithExt.find_first_of("."));

	string szOldExecutable = szExecutableDirectory + szFileName + ".old";
	string szDeleteMe = szExecutableDirectory + szFileName + ".delete_me";
	CopyFile(szExecutablePath.c_str(), szOldExecutable.c_str(), FALSE);
	rename(szExecutablePath.c_str(), szDeleteMe.c_str());

	if((hr = zipFile.Open(szExistingPath)) != S_OK)
		return false;

	if((hr = zipFile.ExtractAllEntries(szDestination)) != S_OK){
		zipFile.Close();
		return false;
	}

	zipFile.Close();

	// Done with the temp file- so erase from temp folder
	if(FileExistsA(szExistingPath))
		_unlink(szExistingPath.c_str());

	// Rename new file 
	string szInstalledXex = szExecutableDirectory + "default.xex";
	string szInstalledXexNew = szExecutableDirectory + szFileNameWithExt;
	_unlink(szDeleteMe.c_str());
	rename(szInstalledXex.c_str(), szInstalledXexNew.c_str());

	//DebugMsg("Test", "Test:  %s,  %s", szExecutablePath.c_str(), szFileName.c_str());

	console->Format("\n**** Extraction Complete");

	return true;
}

bool AutoUpdater::InstallSkinUpdate(ATG::Console * console, string szDestination)
{
	HRESULT hr = NULL;
	PkZip zipFile;

	console->Format("\n\n Extracting and Installing Skin Updates...");
	string szExistingPath = getAutoupdaterTempSkinPath(false);

	if((hr = zipFile.Open(szExistingPath)) != S_OK)
		return false;

	if((hr = zipFile.ExtractAllEntries(szDestination)) != S_OK){
		zipFile.Close();
		return false;
	}

	zipFile.Close();

	// Done with the file- so erase from temp folder
	if(FileExistsA(szExistingPath))
		_unlink(szExistingPath.c_str());

	console->Format("\n**** Extraction Complete");

	return true;
}

bool AutoUpdater::InstallPluginUpdate(ATG::Console * console, string szDestination)
{
	HRESULT hr = NULL;
	PkZip zipFile;

	console->Format("\n\n Extracting and Installing Plugin Updates...");
	string szExistingPath = getAutoupdaterTempPluginPath(false);

	if((hr = zipFile.Open(szExistingPath)) != S_OK)
		return false;

	if((hr = zipFile.ExtractAllEntries(szDestination)) != S_OK){
		zipFile.Close();
		return false;
	}

	zipFile.Close();

	// Done with the file- so erase from temp folder
	if(FileExistsA(szExistingPath))
		_unlink(szExistingPath.c_str());

	console->Format("\n**** Extraction Complete");

	return true;
}