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
#include "../Texture/TextureCache.h"
#include "../Monitors/Status/StatusMonitor.h"
#include "../ContentList/ContentItemNew.h"
#include "../ContentList/ContentManager.h"
#include "../Encryption/MD5/md5.h"
#include "../Xex/Xbox360Container.h"
#include "../ContentList/MarketplaceXML/MarketPlaceASX.h"
#include "../ContentList/ContentMerger.h"
#include "../Managers/Skin/SkinXmlReader/SkinXmlReader.h"

#include "HTTPHandler.h"
#include "../Managers/XZP/XZPManager.h"

#include "../../Application/FreestyleApp.h"

#define HTTPReceiveBufferSize 1024*128
#define HTTPMaxGamerTagLength 100

int handleDefault( ehttp &obj, void *cookie );

HTTPServer::HTTPServer()
{
	BuildExtensionList();
	m_szHttpUsername = "fsdhttp";
	m_szHttpPassword = "fsdhttp";
	bXZPMode = SkinManager::getInstance().isCurrentCompressed();
	g_mGlobalSessionId = "$(&@%@&$%(@&$9792487529846287562452487648756345";
	Start();
}
HTTPServer::~HTTPServer()
{
	Stop();
}

void HTTPServer::BuildExtensionList()
{
	BlockedExtensions.push_back("png");
	BlockedExtensions.push_back("jpg");
	BlockedExtensions.push_back("bmp");
	BlockedExtensions.push_back("gif");
	BlockedExtensions.push_back("css");
	BlockedExtensions.push_back("js");
	BlockedExtensions.push_back("inc");
	BlockedExtensions.push_back("txt");
}

bool HTTPServer::IsExtensionBlocked(string szExtension)
{
	vector<string>::iterator iter;
	szExtension = make_lowercaseA(szExtension);
	for(iter = BlockedExtensions.begin(); iter != BlockedExtensions.end(); ++iter)
	{
		string szBlockedExtension = (*iter);
		if(strcmp(szBlockedExtension.c_str(), szExtension.c_str()) == 0)
			return true;
	}

	return false;
}

unsigned long HTTPServer::Process (void* parameter)
{
	SetThreadName("HTTPServer");
	SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
	
	if( http.init() != EHTTP_ERR_OK )
	{
		DebugMsg("HTTPServer","Can't initalize ehttpd");
		return -1;
	}
	http.add_handler(NULL,handleDefault);
	Connection c(1);
	if( c.init(80) )
	{
		DebugMsg("HTTPServer","Can't open port 80");
		return -1;
	}
// Process incomming requests
	int s=0;
	DebugMsg("HTTPServer","Wait for request");
	while(s>-1)
	{
		if(SETTINGS::getInstance().getHttpServerOn() == false) {
			Sleep(500);
			continue;
		}
		DebugMsg("HTTPServer","accept...");
		int s=c.accept();
		if( s >-1 )
		{
			DebugMsg("HTTPServer","parse...");

			//---------------------------------------------------------------
			//	#3a
			//	For each socket accepted, call the objects parse_request() 
			//	method.  The second param	is a void* which is passed to the 
			//	request handler, and provides a link between the  request 
			//	handler and  your application.
			//---------------------------------------------------------------
				
			http.parse_request(s, NULL);
			DebugMsg("HTTPServer","close...");
			c.close(s);
		}
	}

	// Close down our connection class
	c.terminate();
	return 0;
}
void HTTPServer::Start()
{
	CreateThread(CPU2_THREAD_2);
	
}
void HTTPServer::Stop()
{
}

string GenerateSessionId()
{
	SYSTEMTIME LocalSysTime;
	GetLocalTime( &LocalSysTime );
	string szFileName = sprintfaA("Timestamp[%04d%02d%02d%02d%02d%02d%02d]", 
		LocalSysTime.wYear, 
		LocalSysTime.wMonth, 
		LocalSysTime.wDay, 
		LocalSysTime.wHour, 
		LocalSysTime.wMinute, 
		LocalSysTime.wSecond, 
		LocalSysTime.wMilliseconds
	);				

	// Let's return our completed session id
	return make_uppercaseA(md5(szFileName));
}

int modifyContentHandler( ehttp &obj );

void ApplyGameScreenshotTokens(ehttp &obj, DWORD dwContentId, DWORD dwContentType)
{
	obj.out_replace_token("GAMECONTENTTYPE", sprintfaA("%d", dwContentType));

	ContentItemNew * pContentItem = ContentManager::getInstance().GetContentByContentId(dwContentId);
	if(pContentItem != NULL)
	{
		obj.out_replace_token("GAMETITLEID", sprintfaA("%X", pContentItem->getTitleId()));
		obj.out_replace_token("GAMETITLE",wstrtostr(pContentItem->getTitle()));
		obj.out_replace_token("GAMECONTENTID", sprintfaA("%X", dwContentId)); 
		DWORD dwScreenshotCount = pContentItem->getScreenshotCount();
		if(dwScreenshotCount > (DWORD)SETTINGS::getInstance().getMaxScreenshots()) dwScreenshotCount = SETTINGS::getInstance().getMaxScreenshots();
		obj.out_replace_token("SCREENSHOTCOUNT", sprintfaA("%d", dwScreenshotCount));
		obj.out_replace_token("MAXSCREENSHOTS", sprintfaA("%d", SETTINGS::getInstance().getMaxScreenshots()));

	}
}

void ApplyGameDetailTokens(ehttp &obj, DWORD dwContentId, DWORD dwContentType)
{
	obj.out_replace_token("GAMECONTENTTYPE", sprintfaA("%d", dwContentType));

	vector <ContentItemNew*>::iterator iter;
	string szDataPath = SETTINGS::getInstance().getDataPath();
	ContentItemVector vContentItems = ContentManager::getInstance().GetContentByItemTab((CONTENT_ITEM_TAB)dwContentType);
	for (iter = vContentItems.begin(); iter != vContentItems.end(); iter++)
	{
		ContentItemNew * pContentItem = (*iter);
		// Found the content item we are looking for, lets finish setting up our tokens
		if(pContentItem != NULL && pContentItem->GetItemId() == dwContentId) 
		{
			obj.out_replace_token("GAMETITLEID", sprintfaA("%X", pContentItem->getTitleId()));
			obj.out_replace_token("GAMETITLE",wstrtostr(pContentItem->getTitle()));
			obj.out_replace_token("GAMEDESCRIPTION",wstrtostr(pContentItem->getDescription()));
			obj.out_replace_token("GAMEDEVELOPER",wstrtostr(pContentItem->getDeveloper()));
			obj.out_replace_token("GAMEEXECUTABLE",pContentItem->getExecutable());
			obj.out_replace_token("GAMEGENRE",wstrtostr(pContentItem->getGenre()));
			obj.out_replace_token("GAMECONTENTID", sprintfaA("%X", dwContentId)); 
	
			BYTE * dwIconData;
			DWORD dwIconSize = pContentItem->getIconData(&dwIconData);
			string szIconPath = sprintfaA("/memory/gameicon.png?titleid=%X&address=%X&size=%X", pContentItem->getTitleId(), dwIconData, dwIconSize);
			obj.out_replace_token("GAMEICON", szIconPath);
			string boxartPart = pContentItem->getBoxArtPath();
			boxartPart = str_replaceallA(boxartPart,szDataPath,"");
			boxartPart = str_replaceallA(boxartPart,"\\","/");
			boxartPart = str_replaceallA(boxartPart,".dds",".dds");
			obj.out_replace_token("GAMEBOXART","fsdata/"+boxartPart);
			string bannerPath = pContentItem->getBannerPath();
			bannerPath = str_replaceallA(bannerPath,szDataPath,"");
			bannerPath = str_replaceallA(bannerPath,"\\","/");
			bannerPath = str_replaceallA(bannerPath,".dds",".dds");
			obj.out_replace_token("GAMEBANNER","fsdata/"+bannerPath);
			string backgroundPath = pContentItem->getBackgroundPath();
			backgroundPath = str_replaceallA(backgroundPath,szDataPath,"");
			backgroundPath = str_replaceallA(backgroundPath,"\\","/");
			backgroundPath = str_replaceallA(backgroundPath,".dds",".dds");
			obj.out_replace_token("GAMEBACKGROUND","fsdata/"+backgroundPath);

			break;
		}
	}
}

void ApplyProfileTokens(ehttp &obj)
{
	// Create some constant strings
	char szGamerTag[HTTPMaxGamerTagLength];
	char szGamerScore[HTTPMaxGamerTagLength];
	char szGamerPicURL[HTTPMaxGamerTagLength];

	const char szGamerTagMissing[] = "Not Signed In.";
	const char szGamerScoreMissing[] = "0";
	const char szGamerPicMissing[] = "/files/images/missinggamerpic.png";

	strcpy(szGamerTag, szGamerTagMissing);
	strcpy(szGamerScore, szGamerScoreMissing);
	strcpy(szGamerPicURL, szGamerPicMissing);
	
	// Loop through each of the possible players and replace data
	for(int nCount = 0; nCount < 4; nCount++) {
		
		// Define Tokens for each profile entity
		string szGamerTagToken = sprintfaA("PLAYER%dGAMERTAG", nCount + 1);
		string szGamerScoreToken = sprintfaA("PLAYER%dGAMERSCORE", nCount + 1);
		string szGamerPicToken = sprintfaA("PLAYER%dGAMERPICURL", nCount + 1);
		
		// Determine if the user is online- 
		if(IsUserSignedIn(nCount)) {
			// Grab GamerTag for Current Player
			XUserGetName(nCount, szGamerTag, HTTPMaxGamerTagLength);

			// Grab GamerScore for Current Player
			long dwScore = GetGamerPoints(nCount);
			sprintf(szGamerScore, "%d", dwScore);

			// Create a GamerPic Memory Link
			sprintf(szGamerPicURL, "/memory/gamerpic.png?playerid=%d", nCount);
		}

		// Replace tokens
		obj.out_replace_token(szGamerScoreToken, szGamerScore);
		obj.out_replace_token(szGamerTagToken, szGamerTag);
		obj.out_replace_token(szGamerPicToken, szGamerPicURL);

	}
}

void ApplyStatsTokens(ehttp &obj)
{
	

	obj.out_replace_token("IP", StatusMonitor::getInstance().GetIP());
	obj.out_replace_token("STATUSTEXT", StatusMonitor::getInstance().GetStatus());
	string tempUnit = "";
	if(SETTINGS::getInstance().getCels() == 1)
		tempUnit ="C";
	else
		tempUnit ="F";
	obj.out_replace_token("TEMPUNIT", tempUnit);
	obj.out_replace_token("CASETEMP", sprintfa("%.2f",StatusMonitor::getInstance().GetCASETemp()));
	obj.out_replace_token("CPUTEMP", sprintfa("%.2f",StatusMonitor::getInstance().GetCPUTemp()));
	obj.out_replace_token("GPUTEMP", sprintfa("%.2f",StatusMonitor::getInstance().GetGPUTemp()));
	obj.out_replace_token("MEMTEMP", sprintfa("%.2f",StatusMonitor::getInstance().GetMEMTemp()));
	obj.out_replace_token("CASETEMPPCT", sprintfa("%i",StatusMonitor::getInstance().GetCASETPercent()/10));
	obj.out_replace_token("CPUTEMPPCT", sprintfa("%i",StatusMonitor::getInstance().GetCPUTPercent()/10));
	obj.out_replace_token("GPUTEMPPCT", sprintfa("%i",StatusMonitor::getInstance().GetGPUTPercent()/10));
	obj.out_replace_token("MEMTEMPPCT", sprintfa("%i",StatusMonitor::getInstance().GetMEMTPercent()/10));
}
void ApplyVersionTokens(ehttp &obj)
{
	obj.out_replace_token("DASHVERSION", VersionManager::getInstance().getFSDDashVersionAsString());
	obj.out_replace_token("SKINVERSION", VersionManager::getInstance().getFSDSkinVersionAsString());
	obj.out_replace_token("PLUGINVERSION", VersionManager::getInstance().getFSDPluginVersionAsString());
}
void ApplyPathEntriesToken(ehttp &obj)
{
	string szPathEntryRelativePath = "wwwroot\\pathentry.inc";
	string szPathEntryTemplate = "";

	if( HTTPServer::getInstance().bXZPMode == true ) {
		// Entry Template
		BYTE * entryTemplateData = NULL; DWORD entryTemplateSize = 0;
		XZPManager::getInstance().XZPOpenMemory(SkinManager::getInstance().getCurrentFullSkinPath(), szPathEntryRelativePath, &entryTemplateData, (UINT*)&entryTemplateSize);

		// Copy the data from the file buffer to our Entry Template string
		char * szTempStringA = (char*)malloc(entryTemplateSize + 1);
		memset( szTempStringA, 0, entryTemplateSize + 1);
		memcpy( szTempStringA, (char*)entryTemplateData, entryTemplateSize);
		szPathEntryTemplate = szTempStringA;
		free(szTempStringA);
		free(entryTemplateData);
	} else {
		// Non XZP is easier to load
		szPathEntryTemplate = ReadFileToString(SkinManager::getInstance().getCurrentFullSkinPath() + szPathEntryRelativePath);
	}
	
	string szPathEntryList = "";
	vector<ScanPath> ScanPaths = FSDSql::getInstance().getScanPaths();
	vector<ScanPath>::iterator sp_itr;
	for (sp_itr = ScanPaths.begin(); sp_itr != ScanPaths.end(); ++sp_itr)
	{
		ScanPath ScanPaths = (*sp_itr);
		string szCurrentPathEntry = szPathEntryTemplate;
		string szDevkit = ScanPaths.DevkitTab;
		string szRetail = ScanPaths.RetailTab;
		if(strcmp(szDevkit.c_str(), "NONE") == 0)
			szDevkit = "Automatic";

		if(strcmp(szRetail.c_str(), "NONE") == 0)
			szRetail = "Automatic";

		szCurrentPathEntry = str_replaceallA(szCurrentPathEntry,"##PATHDEVKITTYPE##",szDevkit);
		szCurrentPathEntry = str_replaceallA(szCurrentPathEntry,"##PATHRETAILTYPE##",szRetail);
		szCurrentPathEntry = str_replaceallA(szCurrentPathEntry,"##PATHPATH##", ScanPaths.Path);
		szPathEntryList = szPathEntryList + szCurrentPathEntry;
	}

	obj.out_replace_token("PATHENTRIES", szPathEntryList);
}
void ApplyMenuBarTokens( ehttp &obj)
{
	SkinXMLReader xmlReader;
	xmlReader.LoadSettings("ScnGameView", "Captions");

	obj.out_replace_token("XBOX360", xmlReader.GetSetting("XBOX360", "Xbox 360 Games"));
	obj.out_replace_token("XBLA", xmlReader.GetSetting("XBLA", "Xbox Arcade Games"));
	obj.out_replace_token("XBOXCLASSIC", xmlReader.GetSetting("XBOXCLASSIC", "Xbox Classic Games"));
	obj.out_replace_token("HOMEBREW", xmlReader.GetSetting("HOMEBREW", "Homebrew"));
	obj.out_replace_token("EMULATORS", xmlReader.GetSetting("EMULATORS", "Emulators"));

}

void ApplyGenericTokens( ehttp &obj)
{
	ApplyMenuBarTokens(obj);
	ApplyVersionTokens(obj);
	ApplyProfileTokens(obj);
	ApplyStatsTokens(obj);
}

int handleGetRequest( ehttp &obj, void *cookie )
{
	string httpReferer = obj.getRequestHeader("Referer");
	string httpRequestedFile = obj.getFilename();
	size_t extensionPos = httpRequestedFile.find_first_of(".") + 1;
	string httpFileExt = "";
	if(extensionPos != string::npos)
		httpFileExt = httpRequestedFile.substr(extensionPos);

	if(HTTPServer::getInstance().IsExtensionBlocked(httpFileExt) && httpReferer.find(StatusMonitor::getInstance().GetIP()) == string::npos)
	{
		DebugMsg("HTTPServer", "Direct link attempted");
		obj.out_write_str("Access Denied.");
		obj.out_set_file("");
		return obj.out_commit();
	}

	if(httpRequestedFile.find("/files/") == 0)
	{
		handleFiles(obj, cookie);
		return EHTTP_ERR_OK;
	}
	if(httpRequestedFile.find("/memory/") == 0)
	{
		handleMemoryImage( obj, cookie );
		return EHTTP_ERR_OK;
	}
	if(httpRequestedFile.find("/fsdata/") == 0)
	{
		handleGameAssets( obj, cookie );
		return EHTTP_ERR_OK;
	}
	if(httpRequestedFile == "/screenshot.png")
	{
		string path = ScreenShot(ATG::g_pd3dDevice);
		DebugMsg("HTTPServer","Returning image at %s",path.c_str());
		obj.out_set_file(path,EHTTP_BINARY_FILE);
		obj.out_replace();
		return EHTTP_ERR_OK;
	}
	else if(httpRequestedFile == "/debug.txt")
	{
		//string szFileName = sprintfaA("%s%s", LOG_PATH, LOG_FILE));
		obj.out_set_file(sprintfa("%s%s", LOG_PATH, LOG_FILE));
		obj.out_replace();
		return EHTTP_ERR_OK;
	}

	bool bAuthenticated = false;

	string userName, password, sessionid;
	HTTP_SESSION_DATA * data = NULL;
	if(cookie != NULL) {
		data = (HTTP_SESSION_DATA*)cookie;

		userName = data->Username;
		password = data->Password;
		sessionid = data->SessionId;

		if(strcmp(sessionid.c_str(), HTTPServer::getInstance().g_mGlobalSessionId.c_str()) == 0) {
			bAuthenticated = true;
		}
	}

	bool bBypassSecurity = SETTINGS::getInstance().getHTTPDisableSecurity() == 1 ? true : false;
	if(bBypassSecurity == true)
	{
		data = new HTTP_SESSION_DATA;
		data->Username = "Disabled";
		data->Password = "Disabled";
		data->SessionId = GenerateSessionId();
		HTTPServer::getInstance().g_mGlobalSessionId = data->SessionId;

		cookie = (void*)data;

		bAuthenticated = true;
	}

	if(bAuthenticated == false)
	{
		handleLogin( obj, cookie );
	}
	else if(httpRequestedFile == "/")
	{
		handleIndex( obj, cookie );
	}
	else if(httpRequestedFile == "/360" || httpRequestedFile == "/XBLA" || httpRequestedFile == "/Homebrew" ||
		httpRequestedFile == "/Emulators" || httpRequestedFile == "/Xbox1")
	{
		handleGames( obj, cookie );
	}
	else if(httpRequestedFile.find("/game/") == 0)
	{
		if(httpRequestedFile.find("gamedetails.html") != string::npos)
			handleGameDetails( obj, cookie );
		else if(httpRequestedFile.find("screenshots.html") != string::npos)
			handleGameScreenshots( obj, cookie );
	}
	else if(httpRequestedFile.find("/paths") == 0)
	{
		handlePaths( obj, cookie );
	}
	else if(httpRequestedFile == "/dump")
	{
		string out = "<html><head>";
		// Output our Version Information
		out.append("<b>Version Information:  </b><br>");
		out.append(sprintfaA("<t>Dash Version:  <t>%s<br>", VersionManager::getInstance().getFSDDashVersionAsString().c_str()));
		out.append(sprintfaA("<t>Skin Version:  <t>%s<br>", VersionManager::getInstance().getFSDSkinVersionAsString().c_str()));
		out.append(sprintfaA("<t>Plugin Version:  <t>%s<br>", VersionManager::getInstance().getFSDPluginVersionAsString().c_str()));
		out.append(sprintfaA("<t>Kernel Version:  <t>%s", VersionManager::getInstance().getKernelAsString().c_str()));
		out.append("<br><br>");

		// Output our Memory Information
		MEMORYSTATUS memStat;
		GlobalMemoryStatus(&memStat);
		out.append("<b>Memory Information:  </b><br>");
		out.append(sprintfaA("Memory Remaining:  &nbsp;%d bytes<br>", memStat.dwAvailPhys )); 

		// Output Registered Scan Paths
		vector<ScanPath> m_Paths = FSDSql::getInstance().getScanPaths();
		out.append(sprintfaA("<br><b>Scan Path Information:  [%d] Scan Paths.</b>", m_Paths.size()));
		for(unsigned int x = 0; x < m_Paths.size(); x++)
		{
			ScanPath temp = m_Paths.at(x);
			out.append(sprintfaA("<br>Scan Path Id:  %d<br>", x ));
			out.append(sprintfaA("Path Id:  %d<br>", temp.PathId ));
			out.append(sprintfaA("Path:  %s<br>", temp.Path.c_str()));
			out.append(sprintfaA("Scan Depth:  %d<br>", temp.ScanDepth ));
			out.append(sprintfaA("Retail Tab ID:  %d<br>", temp.RetailTabId));
			out.append(sprintfaA("Retail Tab Name:  %s<br>", temp.RetailTab.c_str()));
			out.append(sprintfaA("Devkit Tab ID:  %d<br>", temp.DevkitTabId));
			out.append(sprintfaA("Devkit Tab Name:  %s<br>", temp.DevkitTab.c_str()));
		}

		// Push our string to the browser
		obj.out_write_str(sprintfaA("%s", out.c_str()));
	}
	else if(httpRequestedFile == "/getdb")
	{
		string out = "<html><head>";
		out.append("<script type=\"text/javascript\">\n");
		out.append("function postwith (to,p) {\n");
		out.append("var myForm = document.createElement(\"form\");\n");
		out.append("myForm.method=\"post\" ;\n");
		out.append("myForm.action = to;\n");
		out.append("for (var k in p) {\n");
		out.append("  var myInput = document.createElement(\"input\");\n");
		out.append("  myInput.setAttribute(\"name\", k);\n");
		out.append("  myInput.setAttribute(\"value\", p[k]);\n");
		out.append("  myForm.appendChild(myInput);\n");
		out.append("}\n");
		out.append("document.body.appendChild(myForm);\n");
		out.append("myForm.submit();\n");
		out.append("document.body.removeChild(myForm);\n");
		out.append("}\n");
		out.append("</script>\n<br><body>");
		vector<string> tns;
		int cnt = FSDSql::getInstance().GetSQLTableNames(&tns);
		if (cnt > 0)
		{
			for each (string tn in tns)
			{
				string link = sprintfaA("<a href=\"javascript:postwith('/gettable?name=%s', {sessionid:'%s'})\" >", tn.c_str(), ((HTTP_SESSION_DATA*)cookie)->SessionId.c_str());
				out.append(link);
				out.append(tn);
				out.append("</a><br />");
			}
		}
		out.append("</body></html>");
		obj.out_write_str(sprintfaA("%s", out.c_str()));
	}
	else if(httpRequestedFile.substr(0, 15) == "/gettable")
	{
		obj.out_write_str("<html><body><table border=1>");
		string tableName = obj.getUrlParam("name");
		SQLTable t;
		t.Clear();
		if (FSDSql::getInstance().GetSQLTable(tableName, &t))
		{
			DebugMsg("HTTPServer", "'%s'.'%s' columns: %d, rows: %d", t.DataBaseName.c_str(), t.TableName.c_str(), t.GetColumnCount(), t.GetRowCount());
			obj.out_write_str("<tr>");
			for each (string s in t.ColumnHeaders)
			{
				obj.out_write_str("<td class='font-weight: bold;'>");
				obj.out_write_str(s);
				obj.out_write_str("</td>");
			}
			obj.out_write_str("</tr>");
			
			for each (map<string, string> m in t.Rows)
			{
				obj.out_write_str("<tr>");
				
				for each (string s in t.ColumnHeaders)
				{
					obj.out_write_str("<td>");
					obj.out_write_str(m.find(s)->second);
					obj.out_write_str("</td>");
				}
				obj.out_write_str("</tr>");
			}
		}
		else
		{
			DebugMsg("HTTPServer", "Failed to get table %s", tableName.c_str());
		}
		obj.out_write_str("</table></body></html>");
	}
		
	if(data)
	{
		delete data;
	}
		
	return 0;
}

int handlePostRequest( ehttp &obj, void *cookie )
{

	string szAction = obj.getPostParam("Action");
	string szFileName = obj.getURL();
	if(strcmp(szAction.c_str(), "Login") == 0)
	{
		HTTP_SESSION_DATA * pData = new HTTP_SESSION_DATA;
		pData->Username = obj.getPostParam("j_username");
		pData->Password = obj.getPostParam("j_password");

		if(strcmp(pData->Username.c_str(), SETTINGS::getInstance().getHttpUsername().c_str()) == 0) 
		{
			if(strcmp(pData->Password.c_str(), SETTINGS::getInstance().getHttpPassword().c_str()) == 0)
			{

				pData->SessionId = GenerateSessionId();
				HTTPServer::getInstance().g_mGlobalSessionId = pData->SessionId;

				handleGetRequest( obj, (void*)pData );
			}
			// Incorrect Password
		}
		// Incorrect Username
	}
	else if(szFileName.find("screenshots.html") != string::npos && szFileName.find("saving=true") != string::npos)
	{
		handleScreenshotUpload(obj, cookie);
	}
	else if(szFileName.find("gamedetails.html") != string::npos && szFileName.find("saving=true") != string::npos)
	{
		handleAssetUpload(obj, cookie);
	}
	else if(szAction == "Restart+FSD")
	{
		string szSessionId = obj.getPostParam("sessionid");
		if(strcmp(szSessionId.c_str(), HTTPServer::getInstance().g_mGlobalSessionId.c_str()) == 0)
		{
			obj.out_write_str("<html>Restarting...<br>");
			obj.out_commit();
			Restart();
		}
	}
	else if(szAction == "Shutdown+FSD")
	{
		string szSessionId = obj.getPostParam("sessionid");
		if(strcmp(szSessionId.c_str(), HTTPServer::getInstance().g_mGlobalSessionId.c_str()) == 0)
		{
			obj.out_write_str("<html>Console has been shut down.<br></html>");
			obj.out_commit();
			Sleep(2000);
			ShutdownXbox();
		}
	}
/*	else if(szAction == "Add+Path")
	{
		// Grab data from the form post
		
		// Determine if the path entered is valid

		// Determine if the path entered already exists

		// If path is real and is not in db, create a scan path struct

		// Add the scan path to the database

		// Add the scan path to the content manager

		string devkitType = obj.getPostParam("devkit");
		string retailType = obj.getPostParam("retail");
		string path = URLdecode(obj.getPostParam("path"));
		path.append("\\");
		path = str_replaceallA(path, "\\\\", "\\");
		DebugMsg("HTTPServer","Adding path : %s",path.c_str());
		if(FileExists(path))
		{
			//vector<ScanPath> paths = SETTINGS::getInstance().getScanPaths();
			//ScanPath newSP;
			//newSP.DevkitTab = devkitType;
			//newSP.RetailTab = retailType;
			//newSP.Path = path;
			//paths.push_back(newSP);
			//SETTINGS::getInstance().setScanPaths(paths);
			//ContentManager::getInstance().AddScanPath(newSP);
			obj.out_write_str("<html>Adding scan paths through HTTP Server is Currently Disabled.<br>");
			obj.out_write_str("<a href=\"/\">Back to main</a><br></html>");
		}
		else
		{
			obj.out_write_str("<html>Entered path doesnt exist...<br>");
			obj.out_write_str("<a href=\"/\">Back to main</a><br></html>");

		}
	}
*/	else 
	{
		// Standard link was clicked- let's submit our session data and load the next page
		HTTP_SESSION_DATA * pData = new HTTP_SESSION_DATA;

		pData->Username = obj.getPostParam("j_username");
		pData->Password = obj.getPostParam("j_password");
		pData->SessionId = obj.getPostParam("sessionid");

		handleGetRequest( obj, (void*)pData );
	}

	return 0;
}

int handleDefault( ehttp &obj, void *cookie )
{
	int procRet = -1;

	if(obj.isGetRequest() == true)
	{
		// Handle All GET Requests here
		procRet = handleGetRequest( obj, cookie );
	}
	else if(obj.isPostRequest() == true)
	{
		// Handle All POST Requests here
		procRet = handlePostRequest( obj, cookie );
	}
	else
	{
		// Unrecongized Request
	}
	return obj.out_commit();
}