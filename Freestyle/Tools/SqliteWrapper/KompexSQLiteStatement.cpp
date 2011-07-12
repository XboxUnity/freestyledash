/*
    This file is part of Kompex SQLite Wrapper.
	Copyright (c) 2008-2010 Sven Broeske

    Kompex SQLite Wrapper is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Kompex SQLite Wrapper is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Kompex SQLite Wrapper. If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <iomanip>
#include <exception>
#include <sstream>
#include <string.h>

#include "stdafx.h"
#include "..\SQLite\sqlite3.h"
#include "..\Debug\Debug.h"

#include "KompexSQLiteStatement.h"
#include "KompexSQLiteDatabase.h"
#include "KompexSQLiteException.h"


namespace Kompex
{

SQLiteStatement::SQLiteStatement(SQLiteDatabase *db):
mDatabase(db),
mStatement(0),
mTransactionID(0)
{
}

SQLiteStatement::~SQLiteStatement()
{
	FreeAllStatements();
	CleanUpTransaction();
}

void SQLiteStatement::FreeAllStatements()
{
	CheckDatabase();

	// sqlite3_next_stmt() returns a pointer to the next prepared statement after pStmt associated with the database connection pDb.
	// If pStmt is NULL then this interface returns a pointer to the first prepared statement associated with the database connection pDb. 
	// If no prepared statement satisfies the conditions of this routine, it returns NULL.

	mStatement = sqlite3_next_stmt(mDatabase->GetDatabaseHandle(), 0);
	while(mStatement)
	{
		FreeQuery();
		mStatement = sqlite3_next_stmt(mDatabase->GetDatabaseHandle(), 0);
	}
}

void SQLiteStatement::Prepare(const char *sqlStatement)
{
//	DebugMsg("KompexSQLiteStatement", "about to Prepare [%s]", sqlStatement);
//	DebugMsg("KompexSQLiteStatement", "database is [%X]", mDatabase);
//	DebugMsg("KompexSQLiteStatement", "in Prepare about to check database");
	CheckDatabase();
//	DebugMsg("KompexSQLiteStatement", "back");
//	DebugMsg("KompexSQLiteStatement", "database handle is [%X]", mDatabase->GetDatabaseHandle());

	// If the nByte argument is less than zero, 
	// then zSql is read up to the first zero terminator. 

//	DebugMsg("KompexSQLiteStatement", "about to Prepare2 [%s]", sqlStatement);

	if(sqlite3_prepare_v2(mDatabase->GetDatabaseHandle(), sqlStatement, -1, &mStatement, 0) != SQLITE_OK) {
		DebugMsg("KompexSQLiteStatement", "FAILED TO PREPARE: %s", sqlStatement);
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
	}
	
//	DebugMsg("KompexSQLiteStatement", "Prepare [%s] returned [%X]", sqlStatement, mStatement);
	if(!mStatement)
		KOMPEX_EXCEPT("Prepare() SQL statement failed");
}

void SQLiteStatement::Prepare(const wchar_t *sqlStatement)
{
	CheckDatabase();

	// If the nByte argument is less than zero, 
	// then zSql is read up to the first zero terminator. 

	if(sqlite3_prepare16_v2(mDatabase->GetDatabaseHandle(), sqlStatement, -1, &mStatement, 0) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));

	if(!mStatement)
		KOMPEX_EXCEPT("Prepare() SQL statement failed");
}

bool SQLiteStatement::Step() const
{
	int retval = sqlite3_step(mStatement);
	switch(retval)
	{
		// sqlite3_step() has finished executing
		case SQLITE_DONE:
			return false;
		// sqlite3_step() has another row ready
		case SQLITE_ROW:
			return true;
		default: 
		{ 
			DebugMsg("KompexSQLiteStatement", "Step returned %d", retval);
			KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
		}
	}
}

void SQLiteStatement::SqlStatement(const char *sqlStatement)
{
//	DebugMsg("KompexSQLiteStatement", "SqlStatement1 before prepare");
//	DebugMsg("KompexSQLiteStatement", "SqlStatement1 before prepare [%X]", sqlStatement);

//	for (int i =0; i < strlen(sqlStatement) ; i++) {
//		DebugMsg("KompexSQLiteStatement", "[%c]", sqlStatement[i]);
//	}
//	DebugMsg("KompexSQLiteStatement", "SqlStatement1 before prepare [%s]", sqlStatement);
//	DebugMsg("KompexSQLiteStatement", "SqlStatement1 before prepare2");

	Prepare(sqlStatement);
	Step();
	FreeQuery();
}

void SQLiteStatement::SqlStatement(const wchar_t *sqlStatement)
{
	Prepare(sqlStatement);
	Step();
	FreeQuery();
}

void SQLiteStatement::SqlStatement(const std::string &sqlStatement)
{
//	DebugMsg("KompexSQLiteStatement", "SqlStatement before prepare [%s]", sqlStatement.c_str());
	Prepare(sqlStatement.c_str());
//	DebugMsg("KompexSQLiteStatement", "SqlStatement after prepare");
	Step();
//	DebugMsg("KompexSQLiteStatement", "SqlStatement after step");
	FreeQuery();
}

bool SQLiteStatement::FetchRow() const
{
	int rc = sqlite3_step(mStatement);

	switch(rc)
	{
		case SQLITE_BUSY:
			KOMPEX_EXCEPT("FetchRow() SQLITE_BUSY");
//			return false;
		case SQLITE_DONE:
			return false;
		case SQLITE_ROW:
			return true;
		case SQLITE_ERROR:
			KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
//			return false;
		case SQLITE_MISUSE:
			KOMPEX_EXCEPT("FetchRow() SQLITE_MISUSE");
//			return false;
	}

	return false;
}

void SQLiteStatement::FreeQuery()
{
	// destroy prepared statement
	sqlite3_finalize(mStatement);
	mStatement = 0;
}

void SQLiteStatement::CheckStatement() const
{
	if(!mStatement)
		KOMPEX_EXCEPT("empty statement pointer");
}

void SQLiteStatement::CheckDatabase() const
{
	if(!mDatabase) {
		DebugMsg("KompexSQLiteStatement", "database pointer invalid");
		KOMPEX_EXCEPT("database pointer invalid");
	}
}

float SQLiteStatement::SqlAggregateFuncResult(const std::string &countSql)
{
	float result = 0.0;

	Sql(countSql);
	while(FetchRow())
		result = static_cast<float>(GetColumnDouble(0));
	
	FreeQuery();
	return result;
}

float SQLiteStatement::SqlAggregateFuncResult(wchar_t *countSql)
{
	float result;

	Sql(countSql);
	while(FetchRow())
		result = static_cast<float>(GetColumnDouble(0));
	
	FreeQuery();
	return result;
}

float SQLiteStatement::SqlAggregateFuncResult(const char *countSql)
{
	float result;

	Sql(countSql);
	while(FetchRow())
		result = static_cast<float>(GetColumnDouble(0));
	
	FreeQuery();
	return result;
}

//------------------------------------------------------------------------------------
// GetColumn...() Methods

const char *SQLiteStatement::GetColumnName(int column) const
{
	CheckStatement();

	if(column < 0 || column >= GetColumnCount())
		KOMPEX_EXCEPT("GetColumnName() column value invalid");

	return sqlite3_column_name(mStatement, column);
}

wchar_t *SQLiteStatement::GetColumnName16(int column) const
{
	CheckStatement();

	if(column < 0 || column >= GetColumnCount())
		KOMPEX_EXCEPT("GetColumnName16() column value invalid");

	return (wchar_t*)sqlite3_column_name16(mStatement, column);
}

const unsigned char *SQLiteStatement::GetColumnCString(int column) const
{
	CheckStatement();
	return sqlite3_column_text(mStatement, column);
}

std::string SQLiteStatement::GetColumnString(int column, bool& isNull) const
{
	CheckStatement();
	std::stringstream ss;
	const unsigned char *chars = sqlite3_column_text(mStatement, column);
	if (chars == NULL) {  
		isNull = true;
		return "";
	}
	isNull = false;
	ss << chars;
	return ss.str();
}

std::string SQLiteStatement::GetColumnString(int column) const
{
	CheckStatement();
	std::stringstream ss;
	const unsigned char *chars = sqlite3_column_text(mStatement, column);
	if (chars == NULL) {  
		return "";
	}
	ss << chars;
	return ss.str();
}


double SQLiteStatement::GetColumnDouble(int column) const
{
	CheckStatement();
	return sqlite3_column_double(mStatement, column);
}

int SQLiteStatement::GetColumnInt(int column) const
{
	CheckStatement();
	return sqlite3_column_int(mStatement, column);
}

int64 SQLiteStatement::GetColumnInt64(int column) const
{
	CheckStatement();
	return sqlite3_column_int64(mStatement, column);
}

int SQLiteStatement::GetColumnType(int column) const
{
	CheckStatement();
	return sqlite3_column_type(mStatement, column);
}

wchar_t *SQLiteStatement::GetColumnString16(int column) const
{
	CheckStatement();
	return (wchar_t*)sqlite3_column_text16(mStatement, column);
}

const void *SQLiteStatement::GetColumnBlob(int column) const
{
	CheckStatement();
	return sqlite3_column_blob(mStatement, column);
}

int SQLiteStatement::GetColumnCount() const
{
	CheckStatement();
	return sqlite3_column_count(mStatement);
}

int SQLiteStatement::GetColumnBytes(int column) const
{
	CheckStatement();
	return sqlite3_column_bytes(mStatement, column);
}

int SQLiteStatement::GetColumnBytes16(int column) const
{
	CheckStatement();
	return sqlite3_column_bytes16(mStatement, column);
}

const char *SQLiteStatement::GetColumnDatabaseName(int column) const
{
	CheckStatement();

	if(column < 0 || column >= GetColumnCount())
		KOMPEX_EXCEPT("GetColumnDatabaseName() column value invalid");

	return sqlite3_column_database_name(mStatement, column);
}

wchar_t *SQLiteStatement::GetColumnDatabaseName16(int column) const
{
	CheckStatement();

	if(column < 0 || column >= GetColumnCount())
		KOMPEX_EXCEPT("GetColumnDatabaseName16() column value invalid");

	return (wchar_t*)sqlite3_column_database_name16(mStatement, column);
}

const char *SQLiteStatement::GetColumnTableName(int column) const
{
	CheckStatement();

	if(column < 0 || column >= GetColumnCount())
		KOMPEX_EXCEPT("GetColumnTableName() column value invalid");

	return sqlite3_column_table_name(mStatement, column);
}

wchar_t *SQLiteStatement::GetColumnTableName16(int column) const
{
	CheckStatement();

	if(column < 0 || column >= GetColumnCount())
		KOMPEX_EXCEPT("GetColumnTableName16() column value invalid");

	return (wchar_t*)sqlite3_column_table_name16(mStatement, column);
}

const char *SQLiteStatement::GetColumnOriginName(int column) const
{
	CheckStatement();

	if(column < 0 || column >= GetColumnCount())
		KOMPEX_EXCEPT("GetColumnOriginName() column value invalid");

	return sqlite3_column_origin_name(mStatement, column);
}

wchar_t *SQLiteStatement::GetColumnOriginName16(int column) const
{
	CheckStatement();

	if(column < 0 || column >= GetColumnCount())
		KOMPEX_EXCEPT("GetColumnOriginName16() column value invalid");

	return (wchar_t*)sqlite3_column_origin_name16(mStatement, column);
}

const char *SQLiteStatement::GetColumnDeclaredDatatype(int column) const
{
	CheckStatement();

	if(column < 0 || column >= GetColumnCount())
		KOMPEX_EXCEPT("GetColumnDeclaredDatatype() column value invalid");

	return sqlite3_column_decltype(mStatement, column);
}

wchar_t *SQLiteStatement::GetColumnDeclaredDatatype16(int column) const
{
	CheckStatement();

	if(column < 0 || column >= GetColumnCount())
		KOMPEX_EXCEPT("GetColumnDeclaredDatatype16() column value invalid");

	return (wchar_t*)sqlite3_column_decltype16(mStatement, column);
}

//------------------------------------------------------------------------------------
// Bind...() Methods

void SQLiteStatement::BindInt(int column, int value) const
{
	if(sqlite3_bind_int(mStatement, column, value) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
}

void SQLiteStatement::BindString(int column, const std::string &string) const
{
//	DebugMsg("KompexSQLiteStatment", "About to bind to a statement with this db [%X] stmt [%X]", mDatabase->GetDatabaseHandle(), mStatement);
	if(sqlite3_bind_text(mStatement, column, string.c_str(), string.length(), SQLITE_TRANSIENT) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
}

void SQLiteStatement::BindString16(int column, const wchar_t *string) const
{
	if(sqlite3_bind_text16(mStatement, column, string, -1, SQLITE_TRANSIENT) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
}

void SQLiteStatement::BindDouble(int column, double value) const
{
	if(sqlite3_bind_double(mStatement, column, value) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
}

void SQLiteStatement::BindInt64(int column, int64 value) const
{
	if(sqlite3_bind_int64(mStatement, column, value) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
}

void SQLiteStatement::BindNull(int column) const
{
	if(sqlite3_bind_null(mStatement, column) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
}

void SQLiteStatement::BindBlob(int column, const void* data, int numberOfBytes) const
{
	if(sqlite3_bind_blob(mStatement, column, data, numberOfBytes, SQLITE_TRANSIENT) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
}

void SQLiteStatement::BindZeroBlob(int column, int length) const
{
	if(sqlite3_bind_zeroblob(mStatement, column, length) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
}

//------------------------------------------------------------------------------------
void SQLiteStatement::ExecuteAndFree()
{
	Step();
	FreeQuery();
}

void SQLiteStatement::GetTable(const std::string &sql, unsigned short consoleOutputColumnWidth) const
{
	CheckDatabase();

	char *errMsg;
	char **result;
	int rows, columns;
	
	if(sqlite3_get_table(mDatabase->GetDatabaseHandle(), sql.c_str(), &result, &rows, &columns, &errMsg) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));

	int counter = 0;
	for(int r = 0; r <= rows; ++r)
	{
		for(int c = 0; c < columns; ++c)
		{
			std::cout << std::left << std::setw(consoleOutputColumnWidth - 3);
			if(result[counter])
				std::cout << result[counter];
			else
				std::cout << "NULL";

			if(c < columns - 1)
				std::cout << " | ";

			counter++;
		}
		std::cout << std::endl;

		if(r == 0)
		{
			for(int dum = consoleOutputColumnWidth * columns; dum != 0; --dum)
				std::cout << "-";

			std::cout << std::endl;
		}
	}

	sqlite3_free_table(result);
}

void SQLiteStatement::GetTableColumnMetadata(const std::string &tableName, const std::string &columnName) const
{
	CheckDatabase();

	int notnull, primaryKey, autoInc;
	const char *dataType, *collSeq;

	if(sqlite3_table_column_metadata(mDatabase->GetDatabaseHandle(), 0, tableName.c_str(), columnName.c_str(), &dataType, &collSeq, &notnull, &primaryKey, &autoInc))
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
			
	std::cout << "TableColumnMetadata:" << std::endl;
	std::cout << "data type: " << dataType << std::endl;
	std::cout << "collation sequence: " << primaryKey << std::endl;
	std::cout << "not null: " << notnull << std::endl;
	std::cout << "primary key: " << primaryKey << std::endl;
	std::cout << "auto increment: " << autoInc << std::endl;
}

void SQLiteStatement::ClearBindings() const
{
	CheckStatement();

	if(sqlite3_clear_bindings(mStatement) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
}

void SQLiteStatement::Reset() const
{
	CheckStatement();

	if(sqlite3_reset(mStatement) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabase->GetDatabaseHandle()));
}

void SQLiteStatement::CommitTransaction() 
{
	if(!mTransactionSQL.empty() || !mTransactionSQL16.empty())
	{
		try
		{
			int i = 0;
			// check wheter we have sql statements with different data types
			if(!mTransactionSQL.empty() && !mTransactionSQL16.empty())
			{
				TTransactionSQL::iterator transIter;
				TTransactionSQL16::iterator trans16Iter;

				unsigned short transactions = (unsigned short)(mTransactionSQL.size() + mTransactionSQL16.size());

				while(i < transactions)
				{
					transIter = mTransactionSQL.find(i);
					if(transIter != mTransactionSQL.end())
					{
						SqlStatement(transIter->second.first);
					}
					else
					{
						trans16Iter = mTransactionSQL16.find(i);
						if(trans16Iter != mTransactionSQL16.end())
						{
							SqlStatement(trans16Iter->second.first);
						}					
						else
						{
							KOMPEX_EXCEPT("CommitTransaction() transaction id not found");
						}
					}

					++i;
				}
			}
			else
			{
				// because we have only one data type, we can execute all sql statements from the filled container
				if(!mTransactionSQL.empty())
				{
					for(TTransactionSQL::iterator transIter = mTransactionSQL.begin(); transIter != mTransactionSQL.end(); ++transIter)
						SqlStatement(transIter->second.first);
				}
				else
				{
					for(TTransactionSQL16::iterator trans16Iter = mTransactionSQL16.begin(); trans16Iter != mTransactionSQL16.end(); ++trans16Iter)
						SqlStatement(trans16Iter->second.first);
				}
			}

			SqlStatement("COMMIT;");
			CleanUpTransaction();
		}
		catch(SQLiteException &exception)
		{
			std::cerr << "Exception Occured!" << std::endl;
			exception.Show();
			RollbackTransaction();
			std::cerr << "Rollback was executed!" << std::endl;
			CleanUpTransaction();
		}
	}
	else
	{
		SqlStatement("COMMIT;");
	}
	mTransactionID = 0;
}

void SQLiteStatement::CleanUpTransaction()
{
	// clean up memory and container
	for(TTransactionSQL::iterator transIter = mTransactionSQL.begin(); transIter != mTransactionSQL.end(); ++transIter)
		DeleteTransactionSqlStr(transIter->second.second, transIter->second.first);
	mTransactionSQL.clear();

	for(TTransactionSQL16::iterator trans16Iter = mTransactionSQL16.begin(); trans16Iter != mTransactionSQL16.end(); ++trans16Iter)
		DeleteTransactionSqlStr(trans16Iter->second.second, trans16Iter->second.first);
	mTransactionSQL16.clear();
}

void SQLiteStatement::BeginTransaction() 
{
	SqlStatement("BEGIN;");
	CleanUpTransaction();
}

void SQLiteStatement::SecureTransaction(const char *sql)
{
	char *buffer = new char[strlen(sql) + 1];
	strcpy_s(buffer, strlen(sql) +1, sql);
	mTransactionSQL[mTransactionID++] = std::make_pair(buffer, true);
}

void SQLiteStatement::SecureTransaction(const std::string sql)
{
	char *buffer = new char[sql.length() + 1];
	strcpy(buffer, sql.c_str());
	mTransactionSQL[mTransactionID++] = std::make_pair(buffer, true);
}

void SQLiteStatement::SecureTransaction(const wchar_t *sql) 
{
	wchar_t *buffer = new wchar_t[wcslen(sql) + 1];
	wcscpy(buffer, sql);
	mTransactionSQL16[mTransactionID++] = std::make_pair(buffer, true);
}

}	// namespace Kompex
