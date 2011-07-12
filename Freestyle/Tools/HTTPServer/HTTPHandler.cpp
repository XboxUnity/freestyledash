#pragma once
#include "stdafx.h"
#include "HTTPServer.h"
#include "../Debug/Debug.h"
#include "embedhttp.h"
#include "connection.h"
#include "../Managers/Version/VersionManager.h"
#include "../Managers/Drives/DrivesManager.h"
#include "../Settings/Settings.h"
#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Generic/PNG/lodepng.h"
#include "../Managers/Skin/SkinManager.h"
#include "../Managers/Skin/SkinXmlReader/SkinXmlReader.h"
#include "../Texture/TextureCache.h"
#include "../Monitors/Status/StatusMonitor.h"
#include "../ContentList/ContentItemNew.h"
#include "../ContentList/ContentManager.h"
#include "../Encryption/MD5/md5.h"
#include "../Xex/Xbox360Container.h"

#include "HTTPHandler.h"
#include "../Managers/XZP/XZPManager.h"

#include "../../Application/FreestyleApp.h"

bool sortScreenshotList(string& left, string& right)
{
	int nLeftIndex, nRightIndex;
	string szFileName, szFileIndex;

	szFileName = left.substr(left.find_last_of("\\") + 1);
	szFileName = szFileName.substr(0, szFileName.find_first_of("."));
	szFileIndex = szFileName.substr(strlen("screenshot"));
	nLeftIndex = atoi(szFileIndex.c_str());

	szFileName = right.substr(right.find_last_of("\\") + 1);
	szFileName = szFileName.substr(0, szFileName.find_first_of("."));
	szFileIndex = szFileName.substr(strlen("screenshot"));
	nRightIndex = atoi(szFileIndex.c_str());
	
	if(nLeftIndex < nRightIndex) return true;
	return false;
}

int handleFiles( ehttp &obj, void * cookie )
{
	string requestedFile = obj.getFilename();
	string requestedPath = requestedFile.substr(7);
	requestedPath = str_replaceallA(requestedPath, "/", "\\");
	string szRelativePath = "wwwroot\\" + requestedPath;

	if(HTTPServer::getInstance().bXZPMode == true) {
		BYTE * fileData = NULL; DWORD fileSize = 0;
		XZPManager::getInstance().XZPOpenMemory(SkinManager::getInstance().getCurrentFullSkinPath(), szRelativePath, &fileData, (UINT*)&fileSize);

		obj.out_set_file((void*)fileData, fileSize, EHTTP_MEMORY_FILE);
	} else {
		string filePath = SkinManager::getInstance().getCurrentFullSkinPath() + szRelativePath;
		obj.out_set_file(filePath, EHTTP_BINARY_FILE);
	}

	return EHTTP_ERR_OK;
}

int handleIndex( ehttp &obj, void * cookie )
{
	string szRelativePath = "wwwroot\\index.html";

	if(HTTPServer::getInstance().bXZPMode == true) {
		BYTE * fileData = NULL; DWORD fileSize = 0;
		XZPManager::getInstance().XZPOpenMemory(SkinManager::getInstance().getCurrentFullSkinPath(), szRelativePath, &fileData, (UINT*)&fileSize);

		obj.out_set_file((void*)fileData, fileSize, EHTTP_TEXT_FILE);
		obj.out_replace_token("SESSIONID", ((HTTP_SESSION_DATA*)cookie)->SessionId);
		ApplyGenericTokens( obj );
		obj.out_replace_mem();
		free(fileData);
	} else {
		string indexPath = SkinManager::getInstance().getCurrentFullSkinPath() + szRelativePath;
		obj.out_set_file(indexPath);
		obj.out_replace_token("SESSIONID", ((HTTP_SESSION_DATA*)cookie)->SessionId);
		ApplyGenericTokens( obj );
		obj.out_replace();
	}

	return 0;
}

int handleLogin( ehttp &obj, void * cookie )
{
	string szRelativePath = "wwwroot\\login.html";

	if(HTTPServer::getInstance().bXZPMode == true) {
		BYTE * fileData = NULL; DWORD fileSize = 0;
		XZPManager::getInstance().XZPOpenMemory(SkinManager::getInstance().getCurrentFullSkinPath(), szRelativePath, &fileData, (UINT*)&fileSize);
		obj.out_set_file((void*)fileData, fileSize, EHTTP_TEXT_FILE);
		obj.out_replace_token("TARGETURL", obj.getURL());
		//obj.out_replace_token("TARGETURL", obj.getFilename());
		obj.out_replace_mem();
		free(fileData);
	} else {
		string indexPath = SkinManager::getInstance().getCurrentFullSkinPath() + szRelativePath;
		obj.out_set_file(indexPath);
		obj.out_replace_token("TARGETURL", obj.getURL());
		//obj.out_replace_token("TARGETURL", obj.getFilename());
		obj.out_replace();
	}
	return 0;
}

int handlePaths( ehttp &obj, void * cookie )
{
	string szRelativePath = "wwwroot\\paths.html";

	if(HTTPServer::getInstance().bXZPMode == true) {
		BYTE * fileData = NULL; DWORD fileSize = 0;
		XZPManager::getInstance().XZPOpenMemory(SkinManager::getInstance().getCurrentFullSkinPath(), szRelativePath, &fileData, (UINT*)&fileSize);

		obj.out_set_file((void*)fileData, fileSize, EHTTP_TEXT_FILE);
		obj.out_replace_token("SESSIONID", ((HTTP_SESSION_DATA*)cookie)->SessionId);
		ApplyGenericTokens(obj);
		ApplyPathEntriesToken(obj);
		obj.out_replace_mem();
		free(fileData);
	} else {
		string indexPath = SkinManager::getInstance().getCurrentFullSkinPath() + szRelativePath;
		obj.out_set_file(indexPath);
		obj.out_replace_token("SESSIONID", ((HTTP_SESSION_DATA*)cookie)->SessionId);
		ApplyGenericTokens(obj);
		ApplyPathEntriesToken(obj);
		obj.out_replace();
	}

	return 0;
}

int handleGameScreenshots( ehttp &obj, void * cookie )
{
	map<string,string>& mParams = obj.getUrlParams();
	DWORD dwContentId = strtoul(mParams["contentid"].c_str(), NULL, 16);
	DWORD dwTitleId = strtoul(mParams["titleid"].c_str(), NULL, 16);
	int nContentType = atoi(mParams["contenttype"].c_str());
	
	// Set up all of our tokens
	ApplyGenericTokens(obj);
	ApplyGameScreenshotTokens(obj, dwContentId, (DWORD)nContentType);

	ContentItemNew * pContentItem = ContentManager::getInstance().GetContentByContentId(dwContentId);
	if(pContentItem == NULL)
		return 0;

	// Set Relative Paths
	string szScreenshotRelativePath = "wwwroot\\screenshots.html";
	string szEntriesRelativePath = "wwwroot\\screenshotentry.inc";
	string szLoadingEntriesRelativePath = "wwwroot\\screenshotloadingentry.inc";
	
	// Load the list entry template into our entrytemplate string
	string szEntryTemplate = "";
	string szLoadingEntryTemplate = "";

	if( HTTPServer::getInstance().bXZPMode == true ) {
		// Entry Template
		BYTE * entryTemplateData = NULL; DWORD entryTemplateSize = 0;
		XZPManager::getInstance().XZPOpenMemory(SkinManager::getInstance().getCurrentFullSkinPath(), szEntriesRelativePath, &entryTemplateData, (UINT*)&entryTemplateSize);

		// Copy the data from the file buffer to our Entry Template string
		char * szTempStringA = (char*)malloc(entryTemplateSize + 1);
		memset( szTempStringA, 0, entryTemplateSize + 1);
		memcpy( szTempStringA, (char*)entryTemplateData, entryTemplateSize);
		szEntryTemplate = szTempStringA;
		free(szTempStringA);
		free(entryTemplateData);

		// Loading Entry Template
		BYTE * loadingEntryTemplateData = NULL; DWORD loadingEntryTemplateSize = 0;
		XZPManager::getInstance().XZPOpenMemory(SkinManager::getInstance().getCurrentFullSkinPath(), szLoadingEntriesRelativePath, &loadingEntryTemplateData, (UINT*)&loadingEntryTemplateSize);

		// Copy the data from the file buffer to our Entry Template string
		char * szTempStringB = (char*)malloc(loadingEntryTemplateSize + 1);
		memset( szTempStringB, 0, loadingEntryTemplateSize + 1);
		memcpy( szTempStringB, (char*)loadingEntryTemplateData, loadingEntryTemplateSize);
		szLoadingEntryTemplate = szTempStringB;
		free(szTempStringB);
		free(loadingEntryTemplateData);

	} else {
		// Non XZP is easier to load
		szEntryTemplate = ReadFileToString(SkinManager::getInstance().getCurrentFullSkinPath() + szEntriesRelativePath);
		szLoadingEntryTemplate = ReadFileToString(SkinManager::getInstance().getCurrentFullSkinPath() + szLoadingEntriesRelativePath);
	}

	// Create entries token to be used in token_replace
	string szEntries = "", szLoadingEntries = "";
	
	int nSSCount = pContentItem->getScreenshotCount();
	if( nSSCount > SETTINGS::getInstance().getMaxScreenshots() ) nSSCount = SETTINGS::getInstance().getMaxScreenshots();

	// Copy all of our screenshot paths to a local vector
	vector<string> m_ScreenshotPaths;
	for( int nCount = 0; nCount < nSSCount; nCount++)
		m_ScreenshotPaths.push_back(pContentItem->getScreenshotPath(nCount));
	
	// Sort the vector with our custom routine to push the paths in the correct order
	sort(m_ScreenshotPaths.begin(), m_ScreenshotPaths.end(), sortScreenshotList);

	for( unsigned int nCount = 0; nCount < m_ScreenshotPaths.size(); nCount++)
	{
		string id = sprintfaA("SS%d", nCount+1);
		string id2 = sprintfaA("SSImg%d", nCount+1);
		string szDataPath = SETTINGS::getInstance().getDataPath();
		string screenshotPath = m_ScreenshotPaths.at(nCount);

		string szFileName = screenshotPath.substr(screenshotPath.find_last_of("\\") + 1);
		szFileName = szFileName.substr(0, szFileName.find_first_of("."));
		string ssid = szFileName.substr(strlen("screenshot"));
		

		screenshotPath = str_replaceallA(screenshotPath,szDataPath,"");
		screenshotPath = str_replaceallA(screenshotPath,"\\","/");
		screenshotPath = str_replaceallA(screenshotPath,".dds",".dds");

		string szCurrentEntry = szEntryTemplate;
		szCurrentEntry = str_replaceallA(szCurrentEntry, "##SCREENSHOT##", "fsdata/" + screenshotPath);
		szCurrentEntry = str_replaceallA(szCurrentEntry, "##SSID##", ssid);
		szEntries = szEntries + szCurrentEntry;

		// Enumerating and Replacing Loading Entry Template Items
		string szCurrentLoadingEntry = szLoadingEntryTemplate;
		szCurrentLoadingEntry = str_replaceallA(szCurrentLoadingEntry, "##SCREENSHOT##", "fsdata/" + screenshotPath);
		szCurrentLoadingEntry = str_replaceallA(szCurrentLoadingEntry, "##SSID##", ssid);
		szCurrentLoadingEntry = str_replaceallA(szCurrentLoadingEntry, "##ID##", id);
		szCurrentLoadingEntry = str_replaceallA(szCurrentLoadingEntry, "##ID2##", id2);

		szLoadingEntries = szLoadingEntries + szCurrentLoadingEntry;
	}

	// Set up all of our tokens
	obj.out_replace_token("SCREENSHOTLIST", szEntries);
	obj.out_replace_token("SCREENSHOTLOADING", szLoadingEntries);

	// Load the gamelist html page, and output to browser
	if(HTTPServer::getInstance().bXZPMode == true) {
		BYTE * fileData = NULL; DWORD fileSize = 0;
		XZPManager::getInstance().XZPOpenMemory(SkinManager::getInstance().getCurrentFullSkinPath(), szScreenshotRelativePath, &fileData, (UINT*)&fileSize);

		obj.out_set_file((void*)fileData, fileSize, EHTTP_TEXT_FILE);
		obj.out_replace_token("SESSIONID", ((HTTP_SESSION_DATA*)cookie)->SessionId);
		ApplyGenericTokens(obj);
		obj.out_replace_mem();
		free(fileData);
	} else {
		string indexPath = SkinManager::getInstance().getCurrentFullSkinPath() + szScreenshotRelativePath;
		obj.out_replace_token("SESSIONID", ((HTTP_SESSION_DATA*)cookie)->SessionId);
		obj.out_set_file(indexPath);
		obj.out_replace();
	}
	return 0;
}

int handleGames( ehttp &obj, void * cookie )
{
	string requestedFile = obj.getFilename();
	// First determine what the content type is from the url path
	string szContentType = requestedFile.substr(1);
	CONTENT_ITEM_TAB contentTab = CONTENT_UNKNOWN;
	
	// Convert requested content type to CONTENT_ITEM_TAB
	if( szContentType.compare("360") == 0 )				contentTab = CONTENT_360;
	else if( szContentType.compare("XBLA") == 0 )		contentTab = CONTENT_XBLA;
	else if( szContentType.compare("Homebrew") == 0 )	contentTab = CONTENT_HOMEBREW;
	else if( szContentType.compare("Emulators") == 0 )	contentTab = CONTENT_EMULATOR;
	else if( szContentType.compare("Xbox1") == 0 )		contentTab = CONTENT_XBOX1;

	// Set Relative Paths
	string szGamelistRelativePath = "wwwroot\\gamelist.html";
	string szEntriesRelativePath = "wwwroot\\gamelistentry.inc";
	string szLoadingEntriesRelativePath = "wwwroot\\gamelistloadingentry.inc";
	
	// Load the list entry template into our entrytemplate string
	string szEntryTemplate = "";
	string szLoadingEntryTemplate = "";

	if( HTTPServer::getInstance().bXZPMode == true ) {
		// Entry Template
		BYTE * entryTemplateData = NULL; DWORD entryTemplateSize = 0;
		XZPManager::getInstance().XZPOpenMemory(SkinManager::getInstance().getCurrentFullSkinPath(), szEntriesRelativePath, &entryTemplateData, (UINT*)&entryTemplateSize);

		// Copy the data from the file buffer to our Entry Template string
		char * szTempStringA = (char*)malloc(entryTemplateSize + 1);
		memset( szTempStringA, 0, entryTemplateSize + 1);
		memcpy( szTempStringA, (char*)entryTemplateData, entryTemplateSize);
		szEntryTemplate = szTempStringA;
		free(szTempStringA);
		free(entryTemplateData);

		// Loading Entry Template
		BYTE * loadingEntryTemplateData = NULL; DWORD loadingEntryTemplateSize = 0;
		XZPManager::getInstance().XZPOpenMemory(SkinManager::getInstance().getCurrentFullSkinPath(), szLoadingEntriesRelativePath, &loadingEntryTemplateData, (UINT*)&loadingEntryTemplateSize);

		// Copy the data from the file buffer to our Entry Template string
		char * szTempStringB = (char*)malloc(loadingEntryTemplateSize + 1);
		memset( szTempStringB, 0, loadingEntryTemplateSize + 1);
		memcpy( szTempStringB, (char*)loadingEntryTemplateData, loadingEntryTemplateSize);
		szLoadingEntryTemplate = szTempStringB;
		free(szTempStringB);
		free(loadingEntryTemplateData);

	} else {
		// Non XZP is easier to load
		szEntryTemplate = ReadFileToString(SkinManager::getInstance().getCurrentFullSkinPath() + szEntriesRelativePath);
		szLoadingEntryTemplate = ReadFileToString(SkinManager::getInstance().getCurrentFullSkinPath() + szLoadingEntriesRelativePath);
	}

	// Create entries token to be used in token_replace
	string szEntries = "", szLoadingEntries = "";
	int nCounter = 0;
	vector<ContentItemNew*>::iterator iter;
	ContentItemVector vContentItems = ContentManager::getInstance().GetContentByItemTab(contentTab);
	
	for( iter = vContentItems.begin(); iter != vContentItems.end(); iter++ ) {
		
		ContentItemNew * pContentItem = (*iter);
		BYTE * iconData = NULL; DWORD iconSize = 0;
		iconSize = pContentItem->getIconData(&iconData);
		string szIconPath = sprintfaA("/memory/gameicon.png?titleid=%X&address=%X&size=%X", pContentItem->getTitleId(), iconData, iconSize );
		string szContentId = sprintfaA("%X", pContentItem->GetItemId());
		string szGameId = sprintfaA("game%d", nCounter);

		// Enumerating and Replacing Entry Template Items
		string szCurrentEntry = szEntryTemplate;
		szCurrentEntry = str_replaceallA(szCurrentEntry, "##GAMECONTENTTYPE##", sprintfaA("%d", (int)contentTab));
		szCurrentEntry = str_replaceallA(szCurrentEntry, "##GAMEICON##", szIconPath);
		string szTitleName = wstrtostr(pContentItem->getTitle());
		if(szTitleName == "") szTitleName = "Game Title Not Found";
		szCurrentEntry = str_replaceallA(szCurrentEntry, "##GAMENAME##", szTitleName);
		szCurrentEntry = str_replaceallA(szCurrentEntry, "##GAMETITLEID##", wstrtostr(pContentItem->getId()));
		szCurrentEntry = str_replaceallA(szCurrentEntry, "##GAMEMEDIAID##", wstrtostr(pContentItem->getMId()));
		szCurrentEntry = str_replaceallA(szCurrentEntry, "##GAMECONTENTID##", szContentId);
		szCurrentEntry = str_replaceallA(szCurrentEntry, "##GAMEPATH##", pContentItem->getRoot() + pContentItem->getPath());
		szCurrentEntry = str_replaceallA(szCurrentEntry, "##SESSIONID##", ((HTTP_SESSION_DATA*)cookie)->SessionId);

		szEntries = szEntries + szCurrentEntry;

		// Enumerating and Replacing Loading Entry Template Items
		string szCurrentLoadingEntry = szLoadingEntryTemplate;
		szCurrentLoadingEntry = str_replaceallA(szCurrentLoadingEntry, "##GAMEICON##", szIconPath);
		szCurrentLoadingEntry = str_replaceallA(szCurrentLoadingEntry, "##GAMECONTENTID##", szContentId);
		szCurrentLoadingEntry = str_replaceallA(szCurrentLoadingEntry, "##GAMEID##", szGameId);

		szLoadingEntries = szLoadingEntries + szCurrentLoadingEntry;
		nCounter++;

	}

	// Set up all of our tokens
	ApplyGenericTokens(obj);
	obj.out_replace_token("GAMELISTENTRIES", szEntries);
	obj.out_replace_token("GAMELISTLOADING", szLoadingEntries);

	SkinXMLReader xmlReader;
	xmlReader.LoadSettings("ScnGameView", "Captions");

	switch (contentTab) {
		case CONTENT_360:
			obj.out_replace_token("GAMELISTTITLE", xmlReader.GetSetting("XBOX360", "Xbox 360 Games"));
			break;
		case CONTENT_XBLA:
			obj.out_replace_token("GAMELISTTITLE", xmlReader.GetSetting("XBLA", "Xbox Arcade Games"));
			break;
		case CONTENT_HOMEBREW:
			obj.out_replace_token("GAMELISTTITLE", xmlReader.GetSetting("HOMEBREW", "Homebrew"));
			break;
		case CONTENT_EMULATOR:
			obj.out_replace_token("GAMELISTTITLE", xmlReader.GetSetting("EMULATORS", "Emulators"));
			break;
		case CONTENT_XBOX1:
			obj.out_replace_token("GAMELISTTITLE", xmlReader.GetSetting("XBOXCLASSIC", "Xbox Classic Games"));
			break;
	};

	// Load the gamelist html page, and output to browser
	if(HTTPServer::getInstance().bXZPMode == true) {
		BYTE * fileData = NULL; DWORD fileSize = 0;
		XZPManager::getInstance().XZPOpenMemory(SkinManager::getInstance().getCurrentFullSkinPath(), szGamelistRelativePath, &fileData, (UINT*)&fileSize);

		obj.out_set_file((void*)fileData, fileSize, EHTTP_TEXT_FILE);
		obj.out_replace_token("SESSIONID", ((HTTP_SESSION_DATA*)cookie)->SessionId);
		ApplyGenericTokens(obj);
		obj.out_replace_mem();
		free(fileData);
	} else {
		string indexPath = SkinManager::getInstance().getCurrentFullSkinPath() + szGamelistRelativePath;
		obj.out_set_file(indexPath);
		obj.out_replace_token("SESSIONID", ((HTTP_SESSION_DATA*)cookie)->SessionId);
		ApplyGenericTokens(obj);
		obj.out_replace();
	}
	return 0;
}

int handleGameDetails( ehttp &obj, void * cookie )
{
	map<string,string>& mParams = obj.getUrlParams();
	DWORD dwContentId = strtoul(mParams["contentid"].c_str(), NULL, 16);
	DWORD dwTitleId = strtoul(mParams["titleid"].c_str(), NULL, 16);
	int nContentType = atoi(mParams["contenttype"].c_str());

	obj.out_replace_token("SESSIONID", ((HTTP_SESSION_DATA*)cookie)->SessionId);

	string szRelativePath = "wwwroot\\gamedetails.html";

	// Set up all of our tokens
	ApplyGenericTokens(obj);
	ApplyGameDetailTokens(obj, dwContentId, (DWORD)nContentType);

	// Commit modified HTML to the browser
	if(HTTPServer::getInstance().bXZPMode == true) {
		BYTE * fileData = NULL; DWORD fileSize = 0;
		XZPManager::getInstance().XZPOpenMemory(SkinManager::getInstance().getCurrentFullSkinPath(), szRelativePath, &fileData, (UINT*)&fileSize);

		obj.out_set_file((void*)fileData, fileSize, EHTTP_TEXT_FILE);
		obj.out_replace_mem();
		free(fileData);
	} else {
		string indexPath = SkinManager::getInstance().getCurrentFullSkinPath() + szRelativePath;
		obj.out_set_file(indexPath);
		obj.out_replace();
	}

	return 0;
}

int handleGameAssets( ehttp &obj, void * cookie )
{
	string requestedFile = obj.getFilename();
	string requestedPath = requestedFile.substr(8);
	requestedPath = str_replaceallA(requestedPath,"/","\\");
	string filePath = SETTINGS::getInstance().getDataPath() + requestedPath;

	if(FileExists(filePath) && FileExt(filePath) == "dds")
	{
		
		BYTE * bytes;
		long len;
		if( filePath.find("bg.dds") != string::npos || filePath.find("box.dds") != string::npos ||
			filePath.find("screenshot") != string::npos)
			ConvertImageFileToBMPBuffer(filePath, &bytes, &len);
		else
			ConvertImageFromFileToPNGBuffer(filePath, &bytes, &len);

		DebugMsg("HTTPServer","Sending %s",filePath.c_str());
		obj.out_set_file(bytes, len, EHTTP_MEMORY_FILE);
	}

	return EHTTP_ERR_OK;
}

int handleMemoryImage( ehttp &obj, void * cookie )
{

	string requestedFile = obj.getFilename();
	string requestedPath = requestedFile.substr(8);
	requestedPath = str_replaceallA(requestedPath,"/","\\");
	if(requestedPath == "gameicon.png")
	{
		// URL Format:  /memory/gameicon.png?titleid=01234567&address=01234567&size=01234567
		// titleid - DWORD to hold the requested gameicon TitleID
		// address - DWORD to hold the address in xbox360 memory where the requested icon lives
		// size - DWORD containing the size of the file in memory

		// Parse URL Parameters
		map<string,string>& mParams = obj.getUrlParams();
		DWORD * dwAddress = (DWORD*)strtoul(mParams["address"].c_str(), NULL, 16);
		DWORD dwSize = strtoul(mParams["size"].c_str(), NULL, 16);
		DWORD dwTitleID = strtoul(mParams["titleid"].c_str(), NULL, 16);

		// Create our PNG Buffer with the known data
		BYTE * pngData;	DWORD pngLen;
		ConvertImageInMemoryToPNGBuffer((BYTE*)dwAddress, dwSize, &pngData, (long*)&pngLen);
		
		// Set the Memory Buffer and Commit to Cleint
		obj.out_set_file(pngData, pngLen, EHTTP_MEMORY_FILE);
	}
	else if(requestedPath == "gamerpic.png")
	{
		// URL Format:  /memory/gamerpic.png?playerid=01234567
		// playerid - DWORD to hold the requested player's id

		// Parse URL Parameters
		map<string,string>& mParams = obj.getUrlParams();
		DWORD dwPlayerId = strtoul(mParams["playerid"].c_str(), NULL, 16);

		// Create our PNG Buffer with the known data
		BYTE * pngData; DWORD pngLen;
		HRESULT retVal = ConvertGamerPicToPNGBuffer(dwPlayerId, &pngData, (long*)&pngLen);
		
		// Commit data only if the result was S_OK
		if(retVal == S_OK) {
			// Set the Memory Buffer and Commit to Cleint
			obj.out_set_file(pngData, pngLen, EHTTP_MEMORY_FILE);
		}
	}
	return EHTTP_ERR_OK;
}

int handleScreenshotUpload( ehttp &obj, void * cookie )
{
	// Variables for Hidden Fields - required to find proper contentitem
	DWORD dwContentId, dwTitleId, dwContentType, dwScreenshotCount;
	DWORD dwScreenshotId;
	string szSessionId;

	DWORD dwActionType; // 0 = unknown, 1 = replace, 2 = add

	// Pointers to hold image information as data is uploaded
	HTTP_ENTRY_DATA assetScreenshot;
	bool hasScreenshot =  false, hasSessionId = false;

	// Loop through the upload and find the data chunk containing our content id
	DWORD chunkCount = obj.post_chunks.size();
	for(DWORD nChunks = 0; nChunks < chunkCount; nChunks++) {
		DWORD entryCount = obj.post_chunks.at(nChunks).Entries.size();
		for( DWORD nEntries = 0; nEntries < entryCount; nEntries++) {
			// loop through every entry and find the ones we want
			if( obj.post_chunks.at(nChunks).Entries.at(nEntries).EntryType == HTTP_ENTRYTYPE_HEADER) {
				if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters.find("name") != obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters.end()) {
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "sessionid") {
					
						// Grab the Content ID
						szSessionId = getEntryContent(obj, nChunks);
						hasSessionId = true;
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "ssAction") {
						
						// Grab the Action Type
						string szActionType = getEntryContent(obj, nChunks);
						dwActionType = strtoul(szActionType.c_str(), NULL, 16);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "contentid") {
					
						// Grab the Content ID
						string szContentId = getEntryContent(obj, nChunks);
						dwContentId = strtoul(szContentId.c_str(), NULL, 16);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "titleid") {
			
						// Grab the Title ID
						string szTitleId = getEntryContent(obj, nChunks);
						dwTitleId = strtoul(szTitleId.c_str(), NULL, 16);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "contenttype") {
						// Grab the ContentType
						string szContentType = getEntryContent(obj, nChunks);
						dwContentType = strtoul(szContentType.c_str(), NULL, 16);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "sscount") {
						// Grab the Screenshot Count
						string szScreenshotCount = getEntryContent(obj, nChunks);
						dwScreenshotCount = strtoul(szScreenshotCount.c_str(), NULL, 16);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "SSID") {
						// Grab the Current Screenshot Id
						string szScreenshotId= getEntryContent(obj, nChunks);
						dwScreenshotId = atoi(szScreenshotId.c_str());
						if(dwScreenshotId < 0) dwScreenshotId = 0;
						if(dwScreenshotId > (DWORD)SETTINGS::getInstance().getMaxScreenshots()) dwScreenshotId = SETTINGS::getInstance().getMaxScreenshots();
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "GameScreenshot") {
						// check to make sure that a file was uploaded
						if(strcmp(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["filename"].c_str(), "") != 0) {
							
							// Extract image data
							assetScreenshot = obj.post_chunks.at(nChunks).FileData;
							hasScreenshot = true;
						}
					}
				}
			}
		}
	}

	// Verify that the session Id matches before applying changes.
	if(strcmp(szSessionId.c_str(), HTTPServer::getInstance().g_mGlobalSessionId.c_str()) != 0)
		return 0;

	// We have officially received all of our posted data- now let's update our database and send out our messages to the ContentObservers
	ContentItemNew * pContentItem = NULL;
	pContentItem = ContentManager::getInstance().GetContentByContentId(dwContentId);

	if(pContentItem != NULL)
	{
		// Handle uploaded content to the database at this point
		CONTENT_SCREENSHOT_INFO ssInfo;	
		switch (dwActionType)
		{
		case 0:  // Replace Action
				
			ssInfo.nScreenshotIndex = dwScreenshotId;
			ssInfo.nScreenshotFlag = CONTENT_SCREENSHOT_FLAG_REPLACE;
			pContentItem->AddAsset(ITEM_UPDATE_TYPE_SCREENSHOT, (VOID*) assetScreenshot.Data, assetScreenshot.Length, (VOID*)&ssInfo);
			break;
		case 1:  // Add Action
			
			if(dwScreenshotCount == SETTINGS::getInstance().getMaxScreenshots()) { // Replace last screenshot if count is max
				ssInfo.nScreenshotFlag = CONTENT_SCREENSHOT_FLAG_REPLACE;
				ssInfo.nScreenshotIndex = SETTINGS::getInstance().getMaxScreenshots() - 1;
			} else {										   // Add additional screenshots
				ssInfo.nScreenshotFlag = CONTENT_SCREENSHOT_FLAG_ADD; 
				ssInfo.nScreenshotIndex = dwScreenshotCount;
			}

			pContentItem->AddScreenshot();
			pContentItem->AddAsset(ITEM_UPDATE_TYPE_SCREENSHOT, (VOID*) assetScreenshot.Data, assetScreenshot.Length, (VOID*)&ssInfo);
			break;
		default:
			break;
		};

	}

	// Standard link was clicked- let's submit our session data and load the next page
	HTTP_SESSION_DATA * pData = new HTTP_SESSION_DATA;

	pData->Username = "";
	pData->Password = "";
	pData->SessionId = szSessionId;

	int ret = handleGetRequest( obj, (void*)pData );

	return ret;
}

int handleAssetUpload( ehttp &obj, void * cookie ) 
{	
	// Variables for Hidden Fields - required to find proper contentitem
	DWORD dwContentId, dwTitleId, dwContentType;
	string szSessionId;

	// Pointers to hold image information as data is uploaded
	HTTP_ENTRY_DATA assetIcon, assetBanner, assetBoxart, assetBackground;
	bool hasIconData = false, hasBannerData = false, hasBoxartData = false, hasBackgroundData = false;

	string szGameTitle, szGameDescription, szGameGenre, szGameDeveloper, szGameExecutable;

	// Loop through the upload and find the data chunk containing our content id
	DWORD chunkCount = obj.post_chunks.size();
	for(DWORD nChunks = 0; nChunks < chunkCount; nChunks++) {
		DWORD entryCount = obj.post_chunks.at(nChunks).Entries.size();
		for( DWORD nEntries = 0; nEntries < entryCount; nEntries++) {
			// loop through every entry and find the ones we want
			if( obj.post_chunks.at(nChunks).Entries.at(nEntries).EntryType == HTTP_ENTRYTYPE_HEADER) {
				if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters.find("name") != obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters.end()) {
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "sessionid") {
					
						// Grab the Content ID
						szSessionId = getEntryContent(obj, nChunks);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "contentid") {
					
						// Grab the Content ID
						string szContentId = getEntryContent(obj, nChunks);
						dwContentId = strtoul(szContentId.c_str(), NULL, 16);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "titleid") {
			
						// Grab the Title ID
						string szTitleId = getEntryContent(obj, nChunks);
						dwTitleId = strtoul(szTitleId.c_str(), NULL, 16);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "contenttype") {
						// Grab the ContentType
						string szContentType = getEntryContent(obj, nChunks);
						dwContentType = strtoul(szContentType.c_str(), NULL, 16);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "GameTitle") {
						
						// Grab the Title
						szGameTitle = getEntryContent(obj, nChunks);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "GameDescription") {
						
						// Grab the Description
						szGameDescription = getEntryContent(obj, nChunks);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "GameGenre") {
						
						// Grab the Genre
						szGameGenre = getEntryContent(obj, nChunks);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "GameDeveloper") {
						
						// Grab the Developer 
						szGameDeveloper = getEntryContent(obj, nChunks);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "GameExecutable") {
						
						// Grab the Executable
						szGameExecutable = getEntryContent(obj, nChunks);
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "GameIcon") {

						if(strcmp(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["filename"].c_str(), "") != 0) {
							
							// Extract image data
							assetIcon = obj.post_chunks.at(nChunks).FileData;
							hasIconData = true;
						}
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "GameBanner") {
						// check to make sure that a file was uploaded
						if(strcmp(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["filename"].c_str(), "") != 0) {
							
							// Extract image data
							assetBanner = obj.post_chunks.at(nChunks).FileData;
							hasBannerData = true;
						}
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "GameBoxart") {
						// check to make sure that a file was uploaded
						if(strcmp(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["filename"].c_str(), "") != 0) {
							
							// Extract image data
							assetBoxart = obj.post_chunks.at(nChunks).FileData;
							hasBoxartData = true;
						}
					}
					if(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["name"] == "GameBackground") {
						// check to make sure that a file was uploaded
						if(strcmp(obj.post_chunks.at(nChunks).Entries.at(nEntries).Parameters["filename"].c_str(), "") != 0) {
							
							// Extract image data
							assetBackground = obj.post_chunks.at(nChunks).FileData;
							hasBackgroundData = true;
						}
					}
				}
			}
		}
	}

	// Verify that the session Id matches before applying changes.
	if(strcmp(szSessionId.c_str(), HTTPServer::getInstance().g_mGlobalSessionId.c_str()) != 0)
		return 0;

	// We have officially received all of our posted data- now let's update our database and send out our messages to the ContentObservers
	ContentItemNew * pContentItem = NULL;
	pContentItem = ContentManager::getInstance().GetContentByContentId(dwContentId);

	if(pContentItem != NULL) 
	{
		if(pContentItem->getTitleId() != dwTitleId) 
			DebugMsg("HTTPServer", "WARNING:  Extracted ContentItem TitleID does not match uploaded content");

		// First we'll update our our text items
		pContentItem->setDisplayInfo(szGameTitle, szGameDescription, szGameGenre, szGameDeveloper);

		// Now we can start on the images
		if(hasIconData == true) {
			DebugMsg("HTTPServer", "GameIcon Path Found, Updating Content.");
			pContentItem->AddAsset(ITEM_UPDATE_TYPE_THUMBNAIL, assetIcon.Data, assetIcon.Length);
		}

		if(hasBannerData == true) {
			DebugMsg("HTTPServer", "GameBanner Path Found, Updating Content.");
			pContentItem->AddAsset(ITEM_UPDATE_TYPE_BANNER, assetBanner.Data, assetBanner.Length);
		}

		if(hasBoxartData == true) {
			DebugMsg("HTTPServer", "GameBoxart Path Found, Updating Content.");
			pContentItem->AddAsset(ITEM_UPDATE_TYPE_BOXART, assetBoxart.Data, assetBoxart.Length);
		}

		if(hasBackgroundData == true) {
			DebugMsg("HTTPServer", "GameBackground Path Found, Updating Content.");
			pContentItem->AddAsset(ITEM_UPDATE_TYPE_BACKGROUND, assetBackground.Data, assetBackground.Length);
		}
	}

	// Standard link was clicked- let's submit our session data and load the next page
	HTTP_SESSION_DATA * pData = new HTTP_SESSION_DATA;

	pData->Username = "";
	pData->Password = "";
	pData->SessionId = szSessionId;

	int ret = handleGetRequest( obj, (void*)pData );

	return ret;
}

string getEntryContent(ehttp &obj, int nChunk)
{
	string strContent = "";
	
	BYTE * dataAddress = obj.post_chunks.at(nChunk).FileData.Data;
	DWORD dataLength = obj.post_chunks.at(nChunk).FileData.Length;
	char * szData = (char *)malloc(dataLength + 1);
	memset(szData, 0, dataLength + 1);
	memcpy(szData, dataAddress, dataLength);

	strContent = szData;
	free(szData);

	return strContent;
}