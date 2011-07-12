#include "stdafx.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../Settings/Settings.h"
#include "../../FileBrowser/FileBrowserBase.h"

class FileBrowserCommand:public DebugCommand
{
public :

	FileBrowserCommand()
	{
		m_CommandName = "FileBrowser";
	}

	void Perform(string parameters)
	{
		if (parameters == "1")
		{
			vector<string> tns;
			int cnt = FSDSql::getInstance().GetSQLTableNames(&tns);
			DebugMsg("MATTIE", "get table names: %d", cnt);
			if (cnt > 0)
			{
				SQLTable t;
				string s;
				for each (string tn in tns)
				{
					t.Clear();
					if (FSDSql::getInstance().GetSQLTable(tn, &t))
					{
						DebugMsg("MATTIE", "'%s' columns: %d, rows: %d", t.TableName.c_str(), t.GetColumnCount(), t.GetRowCount());
					}
					else
					{
						DebugMsg("MATTIE", "Failed to get table '%s'", tn.c_str());
					}
				}
			}
			return;
		}
		
		
		SystemFileBrowser f;
		vector<string> files;
		vector<string> dirs;
		
		vector<string> splitParams;
		StringSplit(parameters, "&", &splitParams);
		
		vector<string> splitCommands;
		for each (string command in splitParams)
		{
			splitCommands.clear();
			StringSplit(command, "?", &splitCommands);
			
			DebugMsg("FileBrowserDebug", "==================  Command: '%s'  ===================== [%d] [%d]", command.c_str(), splitParams.size(), splitCommands.size());

			if (splitCommands.size() == 2)
			{
				if (splitCommands.at(0) == "CD")
				{
					DebugMsg("FileBrowserDebug", "CD '%s': %s", splitCommands.at(1).c_str(), f.ChangeDirectory(splitCommands.at(1)) ? "OK" : "FAILED");
					DebugMsg("FileBrowserDebug", "New dir: '%s'", f.GetCurrentPath().c_str());
				}

				if (splitCommands.at(0) == "TABLE")
				{
					SQLTable t;
					t.Clear();
					if (FSDSql::getInstance().GetSQLTable(splitCommands.at(1), &t))
					{
						DebugMsg("MATTIE", "'%s' columns: %d, rows: %d", t.TableName.c_str(), t.GetColumnCount(), t.GetRowCount());
						string header = "";
						for each (string s in t.ColumnHeaders)
						{
							header = sprintfaA("%s%50s", header.c_str(), s.c_str());
						}
						DebugMsg("MATTIE", "%s", header.c_str());

						string row;
						for each (map<string, string> m in t.Rows)
						{
							row = "";
							for each (string s in t.ColumnHeaders)
							{
								row = sprintfaA("%s%50s", row.c_str(), m.find(s)->second.c_str());
							}
							DebugMsg("MATTIE", "%s", row.c_str());
						}
					}
					else
					{
						DebugMsg("MATTIE", "Failed to get table %s", splitCommands.at(1).c_str());
					}
				}
			}
			
			if (splitCommands.size() == 1)
			{
				if (splitCommands.at(0) == "DIR")
				{
					DebugMsg("FileBrowserDebug", "DIR '%s': %s", f.GetCurrentPath().c_str(), f.GetDirectoryAndFileLists(&files, &dirs) ? "OK" : "FAILED");
				}

				if (splitCommands.at(0) == "UP")
				{
					DebugMsg("FileBrowserDebug", "UP from '%s'", f.GetCurrentPath().c_str());
					f.UpDirectory();				
					DebugMsg("FileBrowserDebug", "New dir: '%s'", f.GetCurrentPath().c_str());
				}

				if (splitCommands.at(0) == "ROOT")
				{
					DebugMsg("FileBrowserDebug", "ROOT from '%s'", f.GetCurrentPath().c_str());
					f.UpToRoot();			
					DebugMsg("FileBrowserDebug", "New dir: '%s'", f.GetCurrentPath().c_str());
				}
			}
		}
	}
};