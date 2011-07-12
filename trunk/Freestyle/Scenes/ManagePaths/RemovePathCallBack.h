#pragma once
#include "../../Tools/Threads/cthread.h"
#include "../../Tools/SQLite/FSDSql.h"
#include "../../Tools/ContentList/ContentManager.h"

using namespace std;

class RemovePathCallBack : public CThread {
private:

	ScanPath		m_CurrentItem;
	bool			done;


public:

	void removePath(ScanPath tempPath) { m_CurrentItem = tempPath; done = false; }
	void Start();
	bool getDone() { return done; }
	static RemovePathCallBack& getInstance()
	{
		static RemovePathCallBack singleton;
		return singleton;
	}

	RemovePathCallBack();
	~RemovePathCallBack();
	RemovePathCallBack(const RemovePathCallBack&);
	RemovePathCallBack& operator=(const RemovePathCallBack&);
};