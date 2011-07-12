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

#ifndef KompexSQLiteException_H
#define KompexSQLiteException_H

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include "KompexSQLitePrerequisites.h"

namespace Kompex
{	
	class _SQLiteWrapperExport SQLiteException
	{
	public:
		//! Overloaded constructor
		SQLiteException(const std::string &filename, unsigned int line, const std::string &errDescription = ""):
		mErrorDescription(errDescription.c_str()),
		mFilename(filename),
		mLine(line)
		{
		}
		
		//! Overloaded constructor
		SQLiteException(const std::string &filename, unsigned int line, const char *errDescription = ""):
		mErrorDescription(errDescription),
		mFilename(filename),
		mLine(line)
		{
		}

		//! Shows all error information.\n
		//! Output: std::cerr
		inline void Show() const {std::cerr << "file: " << mFilename << "\nline: " << mLine << "\nerror: " << mErrorDescription << std::endl;}
		//! Get all error information as std::string
		std::string GetString() const 
		{
			std::stringstream strStream;
			strStream << "file: " << mFilename << "\nline: " << mLine << "\nerror: " << std::string(mErrorDescription) << "\n";
			return strStream.str();
		}

	private:
		//! Error description
		const char *mErrorDescription;
		//! Filename in which the error occured
		std::string mFilename;
		//! Line number in which the error occured
		unsigned int mLine;
	};
};

#endif // KompexSQLiteException_H

#ifndef KOMPEX_EXCEPT
#define KOMPEX_EXCEPT(errorString) throw SQLiteException(__FILE__, __LINE__, errorString)
#endif
