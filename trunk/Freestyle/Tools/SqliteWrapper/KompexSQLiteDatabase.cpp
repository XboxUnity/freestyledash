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

#include <fstream>
#include <iostream>

#include "stdafx.h"

#include <exception>
#include "KompexSQLiteDatabase.h"
#include "KompexSQLiteException.h"

namespace Kompex
{

SQLiteDatabase::SQLiteDatabase():
mDatabaseHandle(0),
mIsMemoryDatabaseActive(false)
{
}

SQLiteDatabase::SQLiteDatabase(const char *filename, int flags, const char *zVfs):
mDatabaseHandle(0),
mIsMemoryDatabaseActive(false)
{
	Open(filename, flags, zVfs);
}

SQLiteDatabase::SQLiteDatabase(const wchar_t *filename):
mDatabaseHandle(0),
mIsMemoryDatabaseActive(false)
{
	Open(filename);
}

SQLiteDatabase::SQLiteDatabase(const std::string &filename, int flags, const char *zVfs):
mDatabaseHandle(0),
mIsMemoryDatabaseActive(false)
{
	Open(filename, flags, zVfs);
}

SQLiteDatabase::~SQLiteDatabase()
{
	Close();
}

void SQLiteDatabase::Open(const char *filename, int flags, const char *zVfs)
{
	// close old db, if one exist
	if(mDatabaseHandle)
		Close();

	if(sqlite3_open_v2(filename, &mDatabaseHandle, flags, zVfs) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabaseHandle));

	mDatabaseFilename = std::string(filename);
}

void SQLiteDatabase::Open(const std::string &filename, int flags, const char *zVfs)
{
	// close old db, if one exist
	if(mDatabaseHandle)
		Close();

	if(sqlite3_open_v2(filename.c_str(), &mDatabaseHandle, flags, zVfs) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabaseHandle));

	mDatabaseFilename = std::string(filename);
}

void SQLiteDatabase::Open(const wchar_t *filename)
{
	// close old db, if one exist
	if(mDatabaseHandle)
		Close();

	if(sqlite3_open16(filename, &mDatabaseHandle) != SQLITE_OK)
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabaseHandle));
}

void SQLiteDatabase::Close()
{	
	// detach database if the database was moved into memory
	if(mIsMemoryDatabaseActive)
	{
		if(sqlite3_exec(mDatabaseHandle, "DETACH DATABASE origin", 0, 0, 0) != SQLITE_OK)
			KOMPEX_EXCEPT(sqlite3_errmsg(mDatabaseHandle));
	}

	if(mDatabaseHandle && sqlite3_close(mDatabaseHandle) != SQLITE_OK)
	{
		KOMPEX_EXCEPT(sqlite3_errmsg(mDatabaseHandle));
	}
	else
	{
		mDatabaseHandle = 0;
		mDatabaseFilename = "";
		mIsMemoryDatabaseActive = false;
	}
}

void SQLiteDatabase::TraceOutput(void *ptr, const char *sql)
{
	std::cout << "trace: " << sql << std::endl;
}

void SQLiteDatabase::ProfileOutput(void* ptr, const char* sql, sqlite3_uint64 time)
{
	std::cout << "profile: " << sql << std::endl;
	std::cout << "profile time: " << time << std::endl;
}

void SQLiteDatabase::MoveDatabaseToMemory()
{
	if(!mIsMemoryDatabaseActive)
	{
		if(mDatabaseFilename == "")
		{
			KOMPEX_EXCEPT("Can't move a database with a UTF16 filename to memory!");
			return;
		}

		sqlite3 *memoryDatabase;
		sqlite3_open(":memory:", &memoryDatabase);   
		
		// create the in-memory schema from the origin database
		sqlite3_exec(mDatabaseHandle, "BEGIN", 0, 0, 0);
		sqlite3_exec(mDatabaseHandle, "SELECT sql FROM sqlite_master WHERE sql NOT NULL", &Kompex::SQLiteDatabase::ProcessDDLRow, memoryDatabase, 0);
		sqlite3_exec(mDatabaseHandle, "COMMIT", 0, 0, 0);
		sqlite3_close(mDatabaseHandle);

		// attach the origin databse to the in-memory
		std::string sql = "ATTACH DATABASE '" + mDatabaseFilename + "' as origin";
		if(sqlite3_exec(memoryDatabase, sql.c_str(), 0, 0, 0) != SQLITE_OK)
			KOMPEX_EXCEPT(sqlite3_errmsg(memoryDatabase));

		// copy the data from the origin databse to the in-memory
		sqlite3_exec(memoryDatabase, "BEGIN", 0, 0, 0);
		sqlite3_exec(memoryDatabase, "SELECT name FROM origin.sqlite_master WHERE type='table'", &Kompex::SQLiteDatabase::ProcessDMLRow, memoryDatabase, 0);
		sqlite3_exec(memoryDatabase, "COMMIT", 0, 0, 0);

		mDatabaseHandle = memoryDatabase;
		mIsMemoryDatabaseActive = true;
	}
}

int SQLiteDatabase::ProcessDDLRow(void *db, int columnsCount, char **values, char **columns)
{
	if(columnsCount != 1)
	{
		KOMPEX_EXCEPT("error occured during DDL: columnsCount != 1");
//		return -1;
	}

	// execute a sql statement in values[0] in the database db.
	if(sqlite3_exec(static_cast<sqlite3*>(db), values[0], 0, 0, 0) != SQLITE_OK)
		KOMPEX_EXCEPT("error occured during DDL: sqlite3_exec");

	return 0;
}

int SQLiteDatabase::ProcessDMLRow(void *db, int columnsCount, char **values, char **columns)
{
	if(columnsCount != 1)
	{
		KOMPEX_EXCEPT("error occured during DML: columnsCount != 1");
//		return -1;
	}
	
	char *stmt = sqlite3_mprintf("INSERT INTO main.%q SELECT * FROM origin.%q", values[0], values[0]);

	if(sqlite3_exec(static_cast<sqlite3*>(db), stmt, 0, 0, 0) != SQLITE_OK)
		KOMPEX_EXCEPT("error occured during DML: sqlite3_exec");

	sqlite3_free(stmt);    

	return 0;
}

void SQLiteDatabase::SaveDatabaseFromMemoryToFile(const std::string &filename)
{
	if(mIsMemoryDatabaseActive)
	{
		std::string file = filename;
		if(file == "")
			file = mDatabaseFilename;

		sqlite3 *fileDatabase;
		if(sqlite3_open_v2(file.c_str(), &fileDatabase, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0) != SQLITE_OK)
			KOMPEX_EXCEPT(sqlite3_errmsg(fileDatabase));
		
		sqlite3_backup *backup;
		backup = sqlite3_backup_init(fileDatabase, "main", mDatabaseHandle, "main");
		if(backup)
		{
			if(sqlite3_backup_step(backup, -1) != SQLITE_DONE)
				KOMPEX_EXCEPT(sqlite3_errmsg(fileDatabase));
			if(sqlite3_backup_finish(backup) != SQLITE_OK)
				KOMPEX_EXCEPT(sqlite3_errmsg(fileDatabase));
		}
		if(sqlite3_close(fileDatabase) != SQLITE_OK)
			KOMPEX_EXCEPT(sqlite3_errmsg(fileDatabase));
	}
}

}	// namespace Kompex
