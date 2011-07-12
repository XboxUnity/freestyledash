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

#ifndef KompexSQLiteStatement_H
#define KompexSQLiteStatement_H

#include <map>
#include <string>

#include "KompexSQLitePrerequisites.h"

namespace Kompex
{	
	#if defined(_MSC_VER) || defined(__BORLANDC__)
	  typedef __int64 int64;
	  typedef unsigned __int64 uint64;
	#else
	  typedef long long int int64;
	  typedef unsigned long long int uint64;
	#endif

	class SQLiteDatabase;

	class _SQLiteWrapperExport SQLiteStatement
	{
	public:
		//! Constructor.\n
		//! @param db		Database in which the SQL should be performed
		SQLiteStatement(SQLiteDatabase *db);
		//! Destructor.
		virtual ~SQLiteStatement();

		//! Only for SQL statements, which have no result.
		//! Can be used for transactions; if you want, you can use an own error handling.
		//! @param sqlStatement		SQL statement (UTF-8) 
		void SqlStatement(const std::string &sqlStatement);
		//! Only for SQL statements, which have no result.
		//! Can be used for transactions; if you want, you can use an own error handling.
		//! @param sqlStatement		SQL statement (UTF-8) 
		void SqlStatement(const char *sqlStatement);
		//! Only for SQL statements, which have no result.
		//! Can be used for transactions; if you want, you can use an own error handling.
		//! @param sqlStatement		SQL statement (UTF-16) 
		void SqlStatement(const wchar_t *sqlStatement);

		//! Only for SQL queries/statements which have a result.\n
		//! e.g. SELECT's or INSERT's which use Bind..() methods!
		//! Do not forget to call FreeQuery() when you have finished.
		inline void Sql(const std::string &sql) {Prepare(sql.c_str());}
		//! Only for SQL queries/statements which have a result.\n
		//! e.g. SELECT's or INSERT's which use Bind..() methods!
		//! Do not forget to call FreeQuery() when you have finished.
		inline void Sql(wchar_t *sql) {Prepare(sql);}
		//! Only for SQL queries/statements which have a result.\n
		//! e.g. SELECT's or INSERT's which use Bind..() methods!
		//! Do not forget to call FreeQuery() when you have finished.
		inline void Sql(const char *sql) {Prepare(sql);}

		//! If you have called Sql(), you can step throw all results.
		bool FetchRow() const;
		//! Call FreeQuery() after Sql() and FetchRow() to clean-up.
		void FreeQuery();

		//! Can be used for all SQLite aggregate functions.
		//! Here you can see all available aggregate functions: 
		//! http://sqlite.org/lang_aggfunc.html
		//! @param countSql		Complete SQL query string (UTF-16).
		float SqlAggregateFuncResult(const std::string &countSql);
		//! Can be used for all SQLite aggregate functions.
		//! Here you can see all available aggregate functions: 
		//! http://sqlite.org/lang_aggfunc.html
		//! @param countSql		Complete SQL query string (UTF-16).
		float SqlAggregateFuncResult(wchar_t *countSql);
		//! Can be used for all SQLite aggregate functions.
		//! Here you can see all available aggregate functions: 
		//! http://sqlite.org/lang_aggfunc.html
		//! @param countSql		Complete SQL query string (UTF-16).
		float SqlAggregateFuncResult(const char *countSql);

		//! Returns the name (UTF-8) assigned to a particular column in the result set of a SELECT statement.\n
		//! You must first call Sql()!
		const char *GetColumnName(int column) const;
		//! Returns the name (UTF-16) assigned to a particular column in the result set of a SELECT statement.\n
		//! You must first call Sql()!
		wchar_t *GetColumnName16(int column) const;

		//! Returns the datatype code for the initial data type of the result column.\n
		//! SQLITE_INTEGER		1\n
		//! SQLITE_FLOAT		2\n
		//! SQLITE_TEXT			3\n
		//! SQLITE3_TEXT		3\n
		//! SQLITE_BLOB			4\n
		//! SQLITE_NULL			5\n
		//! You must first call Sql()!
		int GetColumnType(int column) const;

		//! Returns a character-string from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		const unsigned char *GetColumnCString(int column) const;
		//! Returns a std::string from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		std::string SQLiteStatement::GetColumnString(int column) const;
		std::string GetColumnString(int column, bool& isNull) const;
		//! Returns a UTF-16 string from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		wchar_t *GetColumnString16(int column) const;
		//! Returns a double from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		double GetColumnDouble(int column) const;
		//! Returns a int from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		int GetColumnInt(int column) const;
		//! Returns a int64 from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		int64 GetColumnInt64(int column) const;
		//! Returns a void* from a single column of the current result row of a query.\n
		//! You must first call Sql()!
		const void *GetColumnBlob(int column) const;

		//! Return the number of columns in the result set.\n
		//! You must first call Sql()!
		int GetColumnCount() const;
		//! Returns the number of bytes in a column that has type BLOB or the number of bytes in a TEXT string with UTF-8 encoding.\n
		//! You must first call Sql()!
		int GetColumnBytes(int column) const;
		//! Returns the same value for BLOBs but for TEXT strings returns the number of bytes in a UTF-16 encoding.\n
		//! You must first call Sql()!
		int GetColumnBytes16(int column) const;
		//! Returns a UTF-8 zero-terminated name of the database.\n
		//! You must first call Sql()!
		const char *GetColumnDatabaseName(int column) const;
		//! Returns a UTF-16 zero-terminated name of the database.\n
		//! You must first call Sql()!
		wchar_t *GetColumnDatabaseName16(int column) const;
		//! Returns a UTF-8 zero-terminated name of the table.\n
		//! You must first call Sql()!
		const char *GetColumnTableName(int column) const;
		//! Returns a UTF-16 zero-terminated name of the table.\n
		//! You must first call Sql()!
		wchar_t *GetColumnTableName16(int column) const;
		//! Returns a UTF-8 zero-terminated name of the table column.\n
		//! You must first call Sql()!
		const char *GetColumnOriginName(int column) const;
		//! Returns a UTF-16 zero-terminated name of the table column.\n
		//! You must first call Sql()!
		wchar_t *GetColumnOriginName16(int column) const;
		//! Returns a zero-terminated UTF-8 string containing the declared datatype of the table column.\n
		//! You must first call Sql()!
		const char *GetColumnDeclaredDatatype(int column) const;
		//! Returns a zero-terminated UTF-16 string containing the declared datatype of the table column.\n
		//! You must first call Sql()!
		wchar_t *GetColumnDeclaredDatatype16(int column) const;
		//! Returns the number of values in the current row of the result set.\n
		//! You must first call Sql()!
		inline int GetDataCount() const {return sqlite3_data_count(mStatement);}

		//! Overrides prior binding on the same parameter with an int value.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param value		int value which should inserted in the indicated column
		void BindInt(int column, int value) const;
		//! Overrides prior binding on the same parameter with an UTF-8 string.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param string		UTF-8 string which should inserted in the indicated column
		void BindString(int column, const std::string &string) const;
		//! Overrides prior binding on the same parameter with an UTF-16 string.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param string		UTF-16 string which should inserted in the indicated column
		void BindString16(int column, const wchar_t *string) const;
		//! Overrides prior binding on the same parameter with a double value.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param value		double value which should inserted in the indicated column
		void BindDouble(int column, double value) const;
		//! Overrides prior binding on the same parameter with an int64 value.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param value		int64 value which should inserted in the indicated column
		void BindInt64(int column, int64 value) const;
		//! Overrides prior binding on the same parameter with NULL.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		void BindNull(int column) const;
		//! Overrides prior binding on the same parameter with a BLOB.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column			Column, in which the data should be inserted
		//! @param data				BLOB data which should inserted in the indicated column
		//! @param numberOfBytes	The size of the second parameter (const void *data) in bytes.\n
		//!							Please pay attention, that numberOfBytes is not the number of characters!
		//!							Default: -1.\n
		//!							Negative numberOfBytes means, that the length of the string is the number of\n
		//!							bytes up to the first zero terminator.
		void BindBlob(int column, const void* data, int numberOfBytes = -1) const;
		//! Overrides prior binding on the same parameter with a blob that is filled with zeroes.\n
		//! You must call Sql() one time, before you can use Bind..() methods!
		//! @param column		Column, in which the data should be inserted
		//! @param length		length of BLOB, which is filled with zeroes
		void BindZeroBlob(int column, int length) const;

		//! Executes a INSERT statement and clean-up.\n
		//! You must first call Sql() and Bind..() methods! 
		void ExecuteAndFree();
		//! Returns the result as a complete table.\n
		//! Note: only for console (textoutput)\n
		//! Output: std::cout
		//! @param sql							SQL query string
		//! @param consoleOutputColumnWidth		Width of the output column within the console
		void GetTable(const std::string &sql, unsigned short consoleOutputColumnWidth = 20) const;

		//! Returns metadata about a specific column of a specific database table.
		//! Note: only console output\n
		//! Output: std::cout
		//! @param tableName		Table in which the column is found
		//! @param columnName		Column for which we want the metadata
		void GetTableColumnMetadata(const std::string &tableName, const std::string &columnName) const;

		//! Resets all SQL parameter bindings back to NULL.\n
		//! ClearBindings() does not reset the bindings on a prepared statement!
		void ClearBindings() const;

		//! Reset() is called to reset a prepared statement object back to its initial state,\n
		//! ready to be re-executed. Any SQL statement variables that had values bound to them\n
		//! using the Bind*() functions retain their values. Use ClearBindings() to reset the bindings.
		void Reset() const;

		//! Begins a transaction.
		void BeginTransaction();
		//! Commits a transaction.\n
		//! Exception output: std::cerr
		void CommitTransaction();
		//! Rollback a transaction.
		inline void RollbackTransaction()
		{
			SqlStatement("ROLLBACK;");
			FreeAllStatements();
		}

		//! Can be used only for transaction SQL statements.\n
		//! Can be used for transactions, if you want use the default error handling.
		//! Please note that there is only used a reference of your sql statement.\n
		//! If your sql statement variable is invalid before you called CommitTransaction()
		//! you need to use SecureTransaction(), which creates a internal copy of your sql statement.
		//! @param sql		SQL statement
		inline void Transaction(const char *sql) {mTransactionSQL[mTransactionID++] = std::make_pair(sql, false);}
		//! Can be used only for transaction SQL statements.\n
		//! Can be used for transactions, if you want use the default error handling.
		//! Please note that there is only used a reference of your sql statement.\n
		//! If your sql statement variable is invalid before you called CommitTransaction()
		//! you need to use SecureTransaction(), which creates a internal copy of your sql statement.
		//! @param sql		SQL statement
		inline void Transaction(const std::string &sql)	{mTransactionSQL[mTransactionID++] = std::make_pair(sql.c_str(), false);}
		//! Can be used only for transaction SQL statements.\n
		//! Can be used for transactions, if you want use the default error handling.
		//! Please note that there is only used a reference of your sql statement.\n
		//! If your sql statement variable is invalid before you called CommitTransaction()
		//! you need to use SecureTransaction(), which creates a internal copy of your sql statement.
		//! @param sql		SQL statement
		inline void Transaction(const wchar_t *sql) {mTransactionSQL16[mTransactionID++] = std::make_pair(sql, false);}

		//! Can be used only for transaction SQL statements.\n
		//! Can be used for transactions, if you want use the default error handling.\n
		//! The SecureTransaction() method creates a internal copy of the given sql statement string,\n
		//! so that you do not run into danger if the string will be invalid due to deletion or local scope.
		//! @param sql		SQL statement
		void SecureTransaction(const char *sql);
		//! Can be used only for transaction SQL statements.\n
		//! Can be used for transactions, if you want use the default error handling.\n
		//! The SecureTransaction() method creates a internal copy of the given sql statement string,\n
		//! so that you do not run into danger if the string will be invalid due to deletion or local scope.
		//! @param sql		SQL statement
		void SecureTransaction(const std::string sql);
		//! Can be used only for transaction SQL statements.\n
		//! Can be used for transactions, if you want use the default error handling.\n
		//! The SecureTransaction() method creates a internal copy of the given sql statement string,\n
		//! so that you do not run into danger if the string will be invalid due to deletion or local scope.
		//! @param sql		SQL statement
		void SecureTransaction(const wchar_t *sql);

		struct sqlite3_stmt *getStatement () { return mStatement; }

	protected:
		//! Compile sql query into a byte-code program.
		//! @param sqlStatement			SQL statement (UTF-8) 
		void Prepare(const char *sqlStatement);
		//! Compile sql query into a byte-code program.
		//! @param sqlStatement			SQL statement (UTF-16) 
		void Prepare(const wchar_t *sqlStatement);
		//! Must be called one or more times to evaluate the statement.
		bool Step() const;
		//! Checks if the statement pointer is valid
		void CheckStatement() const;
		//! Checks if the database pointer is valid
		void CheckDatabase() const;
		//! Clean-up all statements.
		void FreeAllStatements();

		//! Free the allocated memory of sql statements 
		template<class T>
		inline void DeleteTransactionSqlStr(bool isMemAllocated, T *str)
		{
			if(isMemAllocated)
				delete[] str;
		}

		//! Free the allocated memory and clean the containers
		void CleanUpTransaction();

	private:
		//! SQL statement
		struct sqlite3_stmt *mStatement;
		//! Database pointer
		SQLiteDatabase *mDatabase;

		//! typedef for UTF-8 transaction statements
		typedef std::map<unsigned short /* transaction id */, std::pair<const char* /* sql */, bool /* memory allocated */> > TTransactionSQL;
		//! typedef for UTF-16 transaction statements
		typedef std::map<unsigned short /* transaction id */, std::pair<const wchar_t* /* sql */, bool /*  is memory allocated */> > TTransactionSQL16;
	
		//! Stores UTF-8 transaction statements
		TTransactionSQL mTransactionSQL;
		//! Stores UTF-16 transaction statements
		TTransactionSQL16 mTransactionSQL16;

		//! id for transactions
		unsigned short mTransactionID;

	};
};

#endif // KompexSQLiteStatement_H
