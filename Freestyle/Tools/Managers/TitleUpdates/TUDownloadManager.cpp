#include "stdafx.h"

#include "../../HTTP/HTTPItem.h"
#include "../../Threads/CThread.h"
#include "../../Debug/Debug.h"
#include "../../../Scenes/ManageTUs/XML/TUXml.h"
#include "TUDownloadManager.h"
#include "../../XEX/Xbox360Container.h"
#include "../../GameContent/GameContentManager.h"
#include "../../../Scenes/ManageTUs/ScnManageTUs.h"
#include "../../XEX/Xbox360Container.h"

TUDownloadManager::TUDownloadManager() {
	TitleID = -1;
	downloadCount = 0;
}
TUDownloadManager::~TUDownloadManager() {
}

void TUDownloadManager::addTUDownload(bool download, ULONGLONG ID) {

}

void TUDownloadManager::DownloadCompleted(HttpItem* itm)
{
	if(strcmp(itm->getTag().c_str(), "TUList")==0)
	{
		if(itm->getResponseCode() == 200) {
			DebugMsg("TUDownloadManager", "Reading List for TitleID [%08X]", TitleID);
			parseXML(itm);
		} else {
			DebugMsg("TUDownloadManager", "Clearing TtileID");
			TitleID = -1;
			DebugMsg("TUDownloadManager", "Clearing Mediaid");
			MediaID = "";
			DebugMsg("TUDownloadManager", "Clearing Gametype");
			gametype = NULL;
			XNotifyQueueUICustom(L"Transferring Title Updates Failed");
		}
	} else if(strcmp(itm->getTag().c_str(), "TUItem")==0)
	{
		downloadCount--;
		if(FileExists(itm->getSaveAsPath()))
		{
			addToDb(itm->getSaveAsPath()); 
		} else {
			XNotifyQueueUICustom(L"Transferring of a Title Update Failed");
		}
		CheckComplete();
	}
}

void TUDownloadManager::parseXML(HttpItem* Item)
{
	TUXml xml;
	xml.szGameID = sprintfaW(L"%08X", TitleID);
	ATG::XMLParser parser;
	parser.RegisterSAXCallbackInterface(&xml);
	MemoryBuffer& buffer = Item->GetMemoryBuffer();
	parser.ParseXMLBuffer((CHAR*)buffer.GetData(),	buffer.GetDataLength());
	if (dl)
	{
		DebugMsg("TUDownloadManager", "Downloading Title Updates for TitleID [%08X]", TitleID);
		DownloadMissing(xml);
	} else {
		UploadMissing(xml);
	}
}

void TUDownloadManager::DownloadMissing(TUXml xml)
{				

	string URL;
	string savePath;
	vector<TUInfo> TU;

	TU = xml.TU;
	
	bool process = false;
			
	for (unsigned int x =0; x < TU.size(); x++)
	{
		
		
		URL = TU.at(x).URL;
		bool doit = false;
		if (gametype == XCONTENTTYPE_ARCADE) {
			DebugMsg("TUDownloadManager", "Process as Arcade");
			doit = true;
		} else if (strcmp(make_lowercaseA(TU.at(x).MediaID).c_str(), make_lowercaseA(MediaID).c_str())==0) {
			DebugMsg("TUDownloadManager", "Process as Media Match %s : %s", TU.at(x).MediaID.c_str(), make_uppercaseA(MediaID).c_str());
			doit = true;
		} 
		if (doit)
		{
			TitleUpdatePath* path = FSDSql::getInstance().getTitleUpdatePathByPathID(pathID);
			DebugMsg("TUDownloadManager", "DataPath : [%s]", path->dataPath.c_str());
			string savePath = path->dataPath;
			string filename = TU.at(x).FileName;
			if (filename.find("tu") != string::npos) {
				savePath = savePath + "\\" + TU.at(x).UpdateName;
			}
			savePath = str_replaceallA(savePath,"\\\\","\\");
			RecursiveMkdir(savePath);
			savePath = sprintfaA("%s\\%s", savePath.c_str(), TU.at(x).FileName.c_str());
			DebugMsg("TUDownloadManager", "Saving Title Update to [%s]", savePath.c_str());
			if (!FileExists(savePath))
			{
				bool found = false;
				for (unsigned int x = 0; x < dQueue.size(); x++)
				{
					if (strcmp(dQueue.at(x).c_str(), savePath.c_str()) == 0)
					{
						found = true;
						DebugMsg("TUDownloadManager", "Found File in Queue passing");
						x = dQueue.size();
					}
				}
				if (!found)
				{
					process = true;
					dQueue.push_back(savePath);
					DebugMsg("TUDownloadManager", "Starting Download");
					HttpItem* itm = HTTPDownloader::getInstance().CreateHTTPItem((CHAR*)URL.c_str(), (CHAR*)savePath.c_str());
					itm->setTag("TUItem");
					itm->setRequester(this);
					HTTPDownloader::getInstance().AddToQueue(itm);
					downloadCount++;
				}
			}
		}
	}
	if (!process) {
		TitleID = -1;
		MediaID = "";
		gametype = NULL;
	}
}

void TUDownloadManager::UploadMissing(TUXml xml)
{	
	xml.szGameID = sprintfaW(L"%08X", TitleID);

	string URL;
	string savePath;
	vector<TUInfo> TU;

	TU = xml.TU;
	bool process = false;
	vector<SQLTitleUpdate*> vec = FSDSql::getInstance().getTitleUpdatesForTitleId(0, TitleID);
	for (unsigned int x = 0; x < vec.size(); x++)
	{
		bool found = false;
		for (unsigned int y = 0; y < TU.size(); y++)
		{
			if (gametype == XCONTENTTYPE_ARCADE ||
				strcmp(make_lowercaseA(TU.at(x).MediaID).c_str(), make_lowercaseA(MediaID).c_str())==0)
			{
				found = true;
				y = TU.size();
			}
		}
		if (!found)
		{
			process = true;
		}
	}
	if (!process) {
		TitleID = -1;
		MediaID = "";
		gametype = NULL;
	}
}


void TUDownloadManager::CheckComplete()
{
	if (downloadCount == 0)
	{
		DebugMsg("TUDownloadManager", "Clearing TtileID");
		TitleID = -1;
		DebugMsg("TUDownloadManager", "Clearing Mediaid");
		MediaID = "";
		DebugMsg("TUDownloadManager", "Clearing Gametype");
		gametype = NULL;
		DebugMsg("TUDownloadManager", "Clearing downloadqueu");
		dQueue.clear();
		DebugMsg("TUDownloadManager", "Notify Requester");
		if (requester != NULL)
			requester->done();
		else
			XNotifyQueueUICustom(L"Transferring Title Updates Complete");
	}
}

void TUDownloadManager::addToDb(string fullpath)
{
	vector<string> pathParts;
	StringSplit(fullpath, "\\", &pathParts);
	string filename = pathParts.at(pathParts.size()-1);
	string addPath;
	TitleUpdatePath* path = FSDSql::getInstance().getTitleUpdatePathByPathID(pathID);
	if (strcmp(pathParts.at(pathParts.size()-2).c_str(), path->deviceID.c_str()) != 0)
	{
		addPath = pathParts.at(pathParts.size()-2);
	}
	ULONGLONG TUID;
	string conFullPath = fullpath;
	Xbox360Container conFile;
	HRESULT hr = conFile.OpenContainer(conFullPath);
	if(hr != S_OK) {
		conFile.CloseContainer();
	} else {

		// Determine what type of content this is
		DWORD contentType = 0x0;
		if((hr = conFile.GetContentType(&contentType)) == S_OK){
			if(contentType == XCONTENTTYPE_INSTALLER) {
				DWORD TmpId;
				hr = conFile.GetTitleId(&TmpId);
				if (hr == S_OK)
				{
					TUID = FSDSql::getInstance().addTitleUpdate(TmpId, conFile.GetDisplayName(), filename, addPath, 0, -1, pathID, 0 );
					FSDSql::getInstance().updateTitleDisabled(0, TUID, true);
				}
			}
		}
		conFile.CloseContainer();	
	}
}