#pragma once

#include "../Generic/tools.h"
#include "../Threads/cthread.h"
#include "../../Scenes/ManageTUs/ActiveList/ActiveList.h"
#include "../../Tools/HTTP/HttpItem.h"
#include "../../Tools/HTTP/HTTPDownloader.h"
#include "../../Scenes/ManageTUs/XML/TUXml.h"
#include "../../Tools/SQLite/FSDSql.h"

class TitleUpdateThread : public CThread
{
public:

	TitleUpdateThread() {
	CreateTitleUpdateThread();
	}
	~TitleUpdateThread( void ) {}
	
	void TitleIDScan(string path, string destdrive);
	void FileScan(string path, string destdrive);

	static TitleUpdateThread& getInstance()
	{
		static TitleUpdateThread singleton;
		return singleton;
	}
	
	void cancel();
	void scan();

private:
	bool CreateTitleUpdateThread( void );

protected:

	virtual unsigned long Process (void* parameter);	
};

