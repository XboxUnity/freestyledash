#pragma once
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../ContentList/ContentManager.h"
#include "../FileBrowser.h"

class VirtualFileBrowser:public FileBrowser, public ContentManagerObserver
{
private :
	wstring GameNameToFolderNameW(wstring gameName);
	string GameNameToFolderName(string gameName);
	string GetRealCurrentPath();

public :
	 VirtualFileBrowser()
	 {
		 Content.clear();
		 ContentManager::getInstance().AddContentObserver(*this);
	 };
	 ~VirtualFileBrowser()
	 {
		 ContentManager::getInstance().RemoveContentObserver(*this);
	 }
	void ContentItemAdded(ContentItemNew * pContent);
	void ContentItemMassAdd(vector<ContentItemNew*> items);
	void ContentItemUpdated(ContentItemNew * pContent, DWORD pContentFlags) {}
	void ContentItemCacheLoaded( DWORD ItemCount ) {}
	void ContentItemDatabaseLoaded( DWORD ItemCount ) {}
	void ContentItemScanComplete( DWORD ItemCount ) {}

	vector<ContentItemNew *> Content;

	 string GetWriteFilePath(string file);
	 FILE* OpenFile(string file);
	 string GetFolderFTPLongDescription(string folder);
	 string GetFileFTPLongDescription(string file);
	 vector<string> GetFolderList();
	 vector<string> GetFileList();
	 void DebugOut();
	
	 BOOL DeleteFolder(string folder);
	
		
};