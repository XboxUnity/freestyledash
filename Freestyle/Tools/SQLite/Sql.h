#pragma once
#include "sqlite3.h"
//#include "log.h"
//#include <string>
//#include <vector>

#include "../Debug/Debug.h"

using namespace std;

string mysql_real_escape_string(string str);


class ISqliteColumn
{
public:
	ISqliteColumn(){
	}

	ISqliteColumn(std::string column,std::string val){
		ColumnName=column;
		Value=val;
	}

	void Set(std::string column,std::string val){
		ColumnName=column;
		Value=val;
	}

	std::string ColumnName;
	std::string Value;
};

class ISqliteRow
{
	int rowsize;
	std::vector<ISqliteColumn> m_list_column;
public:

	ISqliteRow():rowsize(0)
	{
	}

	void New()
	{
		rowsize=0;
		m_list_column.clear();
	}

	void AddColumn(ISqliteColumn col)
	{
		m_list_column.push_back(col);
		rowsize++;
	}

	std::string GetValue(std::string keyname)
	{
		std::string ret="";
		std::vector<ISqliteColumn>::iterator it;
		for(it=m_list_column.begin();it<m_list_column.end();it++)
		{
//			DebugMsg("Sql.h", "Comparing to %s", it->ColumnName.c_str());
			if (keyname.compare(it->ColumnName) == 0)
			{
				return it->Value;
			}
		}
		return ret;
	}

	void printColumnNames()
	{
		std::vector<ISqliteColumn>::iterator it;
		ostringstream s;
		for(it=m_list_column.begin();it<m_list_column.end();it++)
			s << "[" << it->ColumnName << "]  ";
		DebugMsg("Sqlite","%s", s.str().c_str());
	}

	void print()
	{
		std::vector<ISqliteColumn>::iterator it;
		ostringstream s;

		for(it=m_list_column.begin(); it < m_list_column.end(); it++)
		{
			s << setw(10) << "[" << it->Value << "]  ";
		}
		DebugMsg("Sqlite","%s", s.str().c_str());
	}
};

class ISqliteResult
{
public:
	std::vector<ISqliteRow> m_list_row;
	void AddRow(ISqliteRow row)
	{
		m_list_row.push_back(row);
	}
	void print()
	{
		std::vector<ISqliteRow>::iterator it;

		bool bFirst = true;
		for(it=m_list_row.begin();it<m_list_row.end();it++)
		{
			if (bFirst) {
				it->printColumnNames();
				bFirst = false;
			}
			it->print();
//			DebugMsg("Sqlite","\r\n");
		}
	}
	void Clear() {
		m_list_row.clear();
	}
};

static int ISqliteResultCallback(void *obj, int argc, char **argv, char **azColName)
{
	DebugMsg("Sqlite","ISqliteResultCallback %d %d", obj, argc);

	if(obj==NULL)
		return 0;
	
	ISqliteResult *Result=(ISqliteResult*)obj;
	ISqliteRow row;
	
	std::string FirstCol;
	for(int i=0; i<argc; i++)
	{
		DebugMsg("Sqlite","in loop %d", i);
		DebugMsg("Sqlite","in loop col [%d]", azColName[i]);
		DebugMsg("Sqlite","in loop val [%d]", argv[i]);
		DebugMsg("Sqlite","in loop col [%s]", azColName[i]);
		DebugMsg("Sqlite","in loop val [%s]", argv[i]);
		ISqliteColumn col;
		col.Set(azColName[i],argv[i]);
		row.AddColumn(col);
	}
	DebugMsg("Sqlite","ISqliteResultCallback before AddRow");
	Result->AddRow(row);
	DebugMsg("Sqlite","ISqliteResultCallback returning");
	return 0;
}

class CSql
{
public:
	int OpenFile(std::string file)
	{
		int ret = sqlite3_open(file.c_str(), &m_db);	
//		DebugMsg("Sqlite", "OpenFile returned %d", ret);
		return ret;
	};
	void CloseFile()
	{
		sqlite3_close(m_db);
	};

    static CSql & GetInstance()
	{
		static CSql instance;
		return instance;
	};


	int ExecQuery(std::string query,ISqliteResult *TableRow)
	{
		m_ErrorStr.clear();
		char *zErrMsg = 0;
		DebugMsg("Sqlite", "Before Q");
		int result = sqlite3_exec(m_db, query.c_str(), ISqliteResultCallback, (void*)TableRow, &zErrMsg);
		DebugMsg("Sqlite", "After Q");
		if( result!=SQLITE_OK ){
			DebugMsg("Sqlite","SQL error: %s in %s", zErrMsg,query.c_str());
			m_ErrorStr=zErrMsg;
			sqlite3_free(zErrMsg);
		}
		return result;
	};
	//int ExecQuery(std::string query,int _callback)
	
	int ExecQuery(std::string query)
	{
		m_ErrorStr.clear();
		char *zErrMsg = 0;
		int result = sqlite3_exec(m_db, query.c_str(), 0, 0, &zErrMsg);
		if( result!=SQLITE_OK ){
			DebugMsg("Sqlite","SQL error: %s in %s", zErrMsg,query.c_str());
			m_ErrorStr=zErrMsg;
			sqlite3_free(zErrMsg);
		}
		return result;
	};

	std::string GetErrorStr()
	{
		return m_ErrorStr;
	}

private :
	sqlite3 *m_db;
	std::string m_ErrorStr;

    CSql() {
	
	} 

    CSql( const CSql & ); 
    CSql & operator =( const CSql & );
};
