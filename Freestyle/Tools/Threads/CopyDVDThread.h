#pragma once

#include "../Generic/tools.h"
#include "../Threads/cthread.h"


class CopyDVDThread : public CThread
{
public:

	CopyDVDThread() {
	CreateCopyDVDThread();
	}
	~CopyDVDThread( void ) {}
	
	static CopyDVDThread& getInstance()
	{
		static CopyDVDThread singleton;
		return singleton;
	}
	void getTitle(int type);
	bool ExtComp();
	string ReturnTitle();
	void startScan(bool Update, string path);
	string ReturnCurrentItem();
	void cancel();

private:
	bool CreateCopyDVDThread( void );
	int gameType;
	int DvdCopyProcess;
	bool Extraction;
	string Title;
	bool IncludeUpdate;
	string fullcopy;
	string currentItem;
	bool scandone;
	bool loop;

protected:
	string getDVDTitleXBE();
	string getDVDTitleXeX();
	virtual unsigned long Process (void* parameter);	
};

