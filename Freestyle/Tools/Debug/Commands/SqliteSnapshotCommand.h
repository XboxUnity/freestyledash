#include "stdafx.h"
//#include "../../Generic/tools.h"
//#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"
//#include "../../HTTP/HTTPDownloader.h"
#include "../../SQLite/FSDSql.h"
#include "../../Settings/Settings.h"

class SqliteSnapshotCommand : public DebugCommand
{
public :
	SqliteSnapshotCommand()
	{
		m_CommandName = "SqliteSnapshot";
	}
	void Perform(string parameters)
	{
		FSDSql::getInstance().closeDatabase();

		string dbPath = "Game:\\Data\\Databases\\fsd2data.db";
		string dest = "Game:\\Data\\Databases\\fsd2data_snap.db";

		CopyFile(dbPath.c_str(), dest.c_str(), false);

	}
	
};