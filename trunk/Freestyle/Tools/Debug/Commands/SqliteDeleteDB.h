#include "stdafx.h"
//#include "../../Generic/tools.h"
//#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"
//#include "../../HTTP/HTTPDownloader.h"
#include "../../SQLite/FSDSql.h"
#include "../../Settings/Settings.h"

class SqliteDeleteDB : public DebugCommand
{
public :
	SqliteDeleteDB()
	{
		m_CommandName = "SqliteDeleteDB";
	}
	void Perform(string parameters)
	{
		FSDSql::getInstance().closeDatabase();

		string dbPath = "Game:\\Data\\Databases\\fsd2data.db";
		unlink(dbPath.c_str());
	}
	
};