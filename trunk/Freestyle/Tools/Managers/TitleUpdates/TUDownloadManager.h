#pragma once
#include "../../Threads/cthread.h"
#include "../../../Scenes/ManageTUs/XML/TUXml.h"
#include "../../HTTP/HTTPItem.h"

using namespace std;

class TUDownloadManager;

class TUDlMReq{
public :
	virtual void done(){};
};

class TUDownloadManager : public CThread, public iHttpItemRequester {
private:

	bool			notify;
	bool			dl;
	int				downloadCount;
	string			MediaID;
	DWORD			TitleID;
	DWORD			gametype;
	ULONGLONG		pathID;

	TUDlMReq*		requester;


	vector<string>	dQueue;


	

public:

	void addTUDownload(bool download, ULONGLONG ID);

	void setRequester(TUDlMReq* req)	{requester = req;}
	void removeRequester()	{requester = NULL;}
	static TUDownloadManager& getInstance()
	{
		static TUDownloadManager singleton;
		return singleton;
	}

	void parseXML(HttpItem* Item);
	void DownloadCompleted(HttpItem* itm);
	void addToDb(string fullpath);
	void CheckComplete();
	void UploadMissing(TUXml xml);
	void DownloadMissing(TUXml xml);

	TUDownloadManager();
	~TUDownloadManager();
	TUDownloadManager(const TUDownloadManager&);
	TUDownloadManager& operator=(const TUDownloadManager&);
};