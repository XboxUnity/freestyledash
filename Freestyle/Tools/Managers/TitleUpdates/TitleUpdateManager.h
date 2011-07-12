#pragma once

#include "../../Generic/tools.h"
#include "../../Threads/cthread.h"
#include "../../../Scenes/ManageTUs/ActiveList/ActiveList.h"
#include "../../HTTP/HttpItem.h"
#include "../../HTTP/HTTPDownloader.h"
#include "../../../Scenes/ManageTUs/XML/TUXml.h"
#include "../../SQLite/FSDSql.h"

class TitleUpdateManager : public CThread
{
public:

	TitleUpdateManager() {
	CreateTitleUpdateManager();
	}
	~TitleUpdateManager( void ) {}
	
	void TitleIDScan(string path, string destdrive);
	void FileScan(string path, string destdrive);
	void newTuFolderScan(TitleUpdatePath* path, string addPath);
	void ScanCache(TitleUpdatePath* tPath);
	void ScanContent(TitleUpdatePath* tPath);
	string szStatus;
	string getStatus();

	static TitleUpdateManager& getInstance()
	{
		static TitleUpdateManager singleton;
		return singleton;
	}
	
	void cancel();
	void scan();
	void DeleteTitleUpdates(DWORD TitleID, SQLTitleUpdate* TitleUpdate);
	void ConsolidateTitleUpdates(DWORD TitleID);
	bool isWorking() { return working; }

private:
	bool CreateTitleUpdateManager( void );
	int Function;
	bool pause;
	DWORD TID;
	bool working;
	SQLTitleUpdate* TU;
	

protected:

	virtual unsigned long Process (void* parameter);	
};

