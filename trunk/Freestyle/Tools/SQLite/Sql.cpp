#include <string>

#include "stdafx.h"

#include "sqlite3.h"
#include "Sql.h"

string mysql_real_escape_string(string str)
{
	string::iterator it;
	string ret="";
	for ( it=str.begin() ; it < str.end(); it++ ){
		switch(*it)
		{
			case 0:
				ret.append("\0");
				break;
			case '\n':
				ret.append("\\n");
				break;
			case '\'':
				ret.append("''");
				break;
			case '\0x00':
				ret.append("\\0x00");
				break;
			default:
				ret+=*it;
		}
	}
	return ret;
};