#include <algorithm>
#include "stdafx.h"
#include "tools.h"
#include "xboxtools.h"
#include "../Debug/Debug.h"
unsigned int starttime = 0;

using namespace std;
using std::string;

void TokenizeA(const string& str,
                      vector<string>& tokens,
                      const string& delimiters)
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while ( (wstring::npos != pos && pos != 0xffffffff) || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
DWORD FSDGetFileAttribute(const char* file)
{
	DWORD retVal = 0;
	WIN32_FIND_DATA findFileData;
		memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	
	
		HANDLE hFind = FindFirstFile(file, &findFileData);
		if (hFind == INVALID_HANDLE_VALUE)
			retVal = -1;
		else
			retVal =findFileData.dwFileAttributes;
			
		
		FindClose(hFind);
		return retVal;
}
void StringSplit(string str, string delim, vector<string>* results, bool ShowEmptyEntries)
{
	unsigned int cutAt;
	
	while( (cutAt = str.find_first_of(delim)) != str.npos )
	{
		if(!ShowEmptyEntries){
			if(cutAt > 0)
				results->push_back(str.substr(0,cutAt));
		}
		else
		{
			results->push_back(str.substr(0, cutAt));
		}

		str = str.substr(cutAt+1);
	}
	if(str.length() > 0)
	{
		results->push_back(str);
	}
}
void wStringSplit(wstring str, wstring delim, vector<wstring>* results)
{
	unsigned int cutAt;
	
	while( (cutAt = str.find_first_of(delim)) != str.npos )
	{
		
		if(cutAt > 0)
		{
	
			results->push_back(str.substr(0,cutAt));
		}
		str = str.substr(cutAt+1);
	}
	if(str.length() > 0)
	{
		results->push_back(str);
	}
}
void TokenizeW(const wstring& str,
                      vector<wstring>& tokens,
                      const wstring& delimiters)
{
    // Skip delimiters at beginning.
    wstring::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    wstring::size_type pos     = str.find_first_of(delimiters, lastPos);

    while ( (wstring::npos != pos && pos != 0xffffffff) || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

struct lowercase_func {
	void operator()(std::string::value_type& v) { v = (char)tolower(v); }
};

string make_lowercaseA(string s) {
	for_each(s.begin(), s.end(), lowercase_func());
	return s;
}

struct uppercase_func {
	void operator()(std::string::value_type& v) { v = (char)toupper(v); }
};

string make_uppercaseA(string s) {
	for_each(s.begin(), s.end(), uppercase_func());
	return s;
}

struct lowercasew_func {
	void operator()(std::wstring::value_type& v) { v = (char)tolower(v); }
};

wstring make_lowercaseW(wstring s) {
	for_each(s.begin(), s.end(), lowercasew_func());
	return s;
}
int strpos(const char *haystack,const char *needle)
{
   const char *p = strstr(haystack, needle);
   if (p)
      return p - haystack;
   return -1;   // Not found = -1.
}

struct uppercasew_func {
	void operator()(std::wstring::value_type& v) { v = (char)toupper(v); }
};

wstring make_uppercaseW(wstring s) {
	for_each(s.begin(), s.end(), uppercasew_func());
	return s;
}

wstring make_titlecase(wstring s)
{
	wstring res;
	res.reserve(s.size());

	wstring::iterator itr;
	char last = ' ';
	for (itr = s.begin() ; itr != s.end() ; itr++)
	{
		if (last == ' ' || last == '(' || last == '{' || last == '[')
		{
			res.push_back((wchar_t)toupper(*itr));
		} else {
			res.push_back((wchar_t)tolower(*itr));
		}

		last = (char)*itr;
	}

	return res;
}

wstring str_replaceW(wstring source, wstring find, wstring replace)
{
	int srcpos = source.find(find);
	if (srcpos < 0) return source;

	wstring res = source.replace(srcpos,find.size(),replace);

	return res;
}

/*wstring str_replaceallW(wstring source, wstring find, wstring replace)
{
	int srcpos = source.find(find);
	while (srcpos > -1)
	{
		source = source.replace(srcpos,find.size(),replace);
		srcpos = source.find(find);
	}

	return source;
}*/

wstring 
str_replaceallW(wstring s, wstring sub, wstring other)
{
	size_t b = 0;
	for (;;)
	{
		b = s.find(sub, b);
		if (b == s.npos) break;
		s.replace(b, sub.size(), other);
		b += other.size();
	}
	return s;
}



string str_replaceA(string source, string find, string replace)
{
	int srcpos = source.find(find);
	if (srcpos < 0) return source;

	string res = source.replace(srcpos,find.size(),replace);

	return res;
}

/*
string str_replaceallA(string source, string find, string replace)
{
	int srcpos = source.find(find);
	while (srcpos > -1)
	{
		source = source.replace(srcpos,find.size(),replace);
		srcpos = source.find(find);
	}
	return source;
}
*/
string ReadFileToString(string filePath)
{
	string retVal = "";

	char* title;
	FILE * fp = NULL;
	fopen_s(&fp,filePath.c_str(),"r");
	if(fp == NULL) return "";
	fseek(fp,0,SEEK_END);
	long fSize = ftell(fp);
	rewind(fp);

	title = (char*)malloc(fSize+1);
	memset(title,0,fSize+1);
	fread(title,1,fSize,fp);
	fclose(fp);
	retVal = title;
	//	}
	return (retVal);
}

string str_removeInvalidChar(string s)
{
	string retVal = "";
	for(int x=0;x<(int)s.length();x++)
	{
		if(s.at(x) >=0)
		{
			retVal = retVal + s.at(x);
		}
	}
	return retVal;
}

wstring str_removeSpecialCharW(wstring s)
{
	
	wstring retVal = L"";
	for(int x=0;x<(int)s.length();x++)
	{
		if(isalnum(s.at(x)) !=0 || isspace(s.at(x)) != 0 || s.at(x) == 45 || s.at(x) == 46)
		{
			retVal = retVal + s.at(x);
		}
	}
	return retVal;
}

string str_removeSpecialChar(string s)
{
	
	string retVal = "";
	for(int x=0;x<(int)s.length();x++)
	{
		if(isalnum(s.at(x)) !=0 || isspace(s.at(x)) != 0 || s.at(x) == 45 || s.at(x) == 46)
		{
			retVal = retVal + s.at(x);
		}
	}
	return retVal;
}

wstring str_removeInvalidCharW(wstring s)
{
	wstring retVal = L"";
	for(int x=0;x<(int)s.length();x++)
	{
		if(s.at(x) >=0)
		{
			retVal = retVal + s.at(x);
		}
	}
	return retVal;
}


string str_replaceallA(string s, string sub, string other)
{
	size_t b = 0;
	for (;;)
	{
		b = s.find(sub, b);
		if (b == s.npos) break;
		s.replace(b, sub.size(), other);
		b += other.size();
	}
	return s;
}

string ParseExecutablePath( string szFilePath )
{
	size_t found;
	found = szFilePath.find_last_of("\\");
	return szFilePath.substr(0, found);
}

string wstrtostr(wstring wstr)
{
	string s(wstr.begin(), wstr.end());
	s.assign(wstr.begin(), wstr.end());
	return s;
}
void RecursiveMkdir(string path)
{
	vector<string> makePath;
	StringSplit(path, "\\", &makePath);
	if (makePath.size() > 1)
	{
		string tempPath = makePath.at(0);
		for (unsigned int x = 1; x < makePath.size(); x++)
		{
			tempPath = tempPath + "\\" + makePath.at(x);
			if (!FileExists(tempPath))
			{
				_mkdir(tempPath.c_str());
			}
		}
	}

}
WCHAR* strtowchar(string str)
{
	WCHAR* retVal;

	int Length = MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,NULL,0);	
	retVal = new WCHAR[Length+1];
	MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,retVal,Length);

	return retVal;
}
char* wchartochar(const WCHAR* str,int mLength)
{
	int Length = WideCharToMultiByte(CP_UTF8,0,str,mLength,NULL,0,NULL,NULL);	
	char* retVal = new char[Length+1];
	WideCharToMultiByte(CP_UTF8,0,str,mLength,retVal,Length,NULL,NULL);
	return retVal;
}

wstring FromUtf8(const std::string& utf8string)
{
	int widesize = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, NULL, 0);
	if (widesize == ERROR_NO_UNICODE_TRANSLATION)
	{
		DebugMsg("Tools", "FromUtf8 Invalid UTF-8 sequence");
		return L"FromUtf8 Invalid UTF8";
//		throw std::exception("Invalid UTF-8 sequence.");
	}
	if (widesize == 0)
	{
		DebugMsg("Tools", "FromUtf8 Error in conversion");
		return L"FromUtf8 Error in conversion";
//		throw std::exception("Error in conversion.");
	}

	std::vector<wchar_t> resultstring(widesize);

	int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, &resultstring[0], widesize);

	if (convresult != widesize)
	{
		DebugMsg("Tools", "FromUtf8 Failed");
		return L"FromUtf8 Failed";
//		throw std::exception("Failed");
	}

	return std::wstring(&resultstring[0]);
}


string ToUtf8(const std::wstring& widestring)
{
	int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, NULL, 0, NULL, NULL);
	if (utf8size == 0)
	{
		DebugMsg("Tools", "ToUtf8 Error in conversion");
		return "ToUtf8 Error in conversion";
//		throw std::exception("Error in conversion.");
	}

	std::vector<char> resultstring(utf8size);

	int convresult = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, &resultstring[0], utf8size, NULL, NULL);

	if (convresult != utf8size)
	{
		DebugMsg("Tools", "ToUtf8 FAILED");
		return "ToUtf8 Failed";
//		throw std::exception("Failed!");
	}

	return std::string(&resultstring[0]);
}


/*
wstring strtowstr(string str)
{
	wstring s(str.begin(), str.end());
	s.assign(str.begin(),str.end());
	return s;
}*/

wstring strtowstr(string str)
{
	WCHAR* retVal;
	int Length = MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,NULL,0);
	retVal = new WCHAR[Length+1];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, retVal, Length);

	std::wstring ret(retVal);
	delete[] retVal;
	
	return ret;
}

/*
bool DirectoryInfo(string directory, vector<DI_Item>& files, vector<DI_Item>& folders, bool clear)
{
	printf("Scanning Dir %s\n",directory.c_str());
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd = directory+"\\*";
	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	if (clear)
	{
		files.clear();
		folders.clear();
	}
	do {
		string s = findFileData.cFileName;
		if (s == ".") continue;
		if (s == "..") continue;

		DI_Item item;
		item.Path = directory;
		item.FileName = s;
		item.attribs = findFileData.dwFileAttributes;
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			folders.push_back(item);
		} else{
			files.push_back(item);
		}
	} while (FindNextFile(hFind, &findFileData));
	FindClose(hFind);

	return true;
}*/

wstring LastFolderW(wstring folder)
{
	if (folder.at(folder.size()-1) == L'\\')
		folder = folder.substr(0,folder.size()-1);
	folder = folder.substr(folder.rfind(L"\\")+1);

	return folder;
}

wstring FileExtW(wstring filename)
{
	int dotpos = (int)filename.rfind(L".");
	if (dotpos == 0) return L"";
	wstring lc = make_lowercaseW(filename.substr(dotpos+1));
	return lc;
}

wstring FileNoExtW(wstring filename)
{
	int dotpos = (int)filename.rfind(L".");
	if (dotpos == 0) return filename;
	return filename.substr(0,dotpos);
}

string LastFolderA(string folder)
{
	if (folder.at(folder.size()-1) == '\\')
		folder = folder.substr(0,folder.size()-1);
	folder = folder.substr(folder.rfind("\\")+1);

	return folder;
}

string FileExtA(string filename)
{
	int dotpos = (int)filename.rfind(".");
	if (dotpos == 0) return "";
	string lc = make_lowercaseA(filename.substr(dotpos+1));
	return lc;
}

string FileNoExtA(string filename)
{
	int dotpos = (int)filename.rfind(".");
	if (dotpos == 0) return filename;
	return filename.substr(0,dotpos);
}


bool isbadcharw(WCHAR in)
{
	if (in == L' ' || in == L'\t' || in == L'\r' || in == L'\n') return true;
	return false;
}

wstring TrimLeftW(wstring str) 
{
	if (str.size() == 0) return L"";
	wstring::iterator i;
	for (i = str.begin(); i != str.end(); i++) {
		if (!isbadcharw(*i)) {
			break;
		}
	}
	if (i == str.end()) {
		str.clear();
	} else {
		str.erase(str.begin(), i);
	}
	return str;
}

wstring TrimRightW(wstring str)
{
	if (str.size() == 0) return L"";
	wstring::iterator i;
	for (i = str.end() - 1; ;i--) {
		if (!isbadcharw(*i)) {
			str.erase(i + 1, str.end());
			break;
		}
		if (i == str.begin()) {
			str.clear();
			break;
		}
	}
	return str;
}

wstring TrimW(wstring s) 
{
	return TrimLeftW(TrimRightW(s));
}

bool isbadchara(WCHAR in)
{
	if (in == ' ' || in == '\t' || in == '\r' || in == '\n') return true;
	return false;
}

string TrimLeftA(string str) 
{
	if (str.size() == 0) return "";
	string::iterator i;
	for (i = str.begin(); i != str.end(); i++) {
		if (!isbadchara(*i)) {
			break;
		}
	}
	if (i == str.end()) {
		str.clear();
	} else {
		str.erase(str.begin(), i);
	}
	return str;
}

string TrimRightA(string str)
{
	if (str.size() == 0) return "";
	string::iterator i;
	for (i = str.end() - 1; ;i--) {
		if (!isbadchara(*i)) {
			str.erase(i + 1, str.end());
			break;
		}
		if (i == str.begin()) {
			str.clear();
			break;
		}
	}
	return str;
}

string TrimA(string s) 
{
	return TrimLeftA(TrimRightA(s));
}

string TrimRightStr(string str,string crop)
{
	if (str.size() < crop.size()) return str;
	string temp = str.substr(str.size()-crop.size(),crop.size());
	if (strcmp(make_lowercaseA(temp).c_str(),make_lowercaseA(crop).c_str()) == 0)
	{
		return  str.substr(0,str.size()-crop.size());
	}	
	return str;
}

wstring sprintfaW(const WCHAR *format, ...)
{
	WCHAR temp[16384];

	va_list ap;
	va_start (ap, format);
	vswprintf_s (temp,16384, format, ap);
	va_end (ap);

	return temp;
}

string sprintfaA(const char *format, ...)
{
	char temp[16384];

	va_list ap;
	va_start (ap, format);
	vsprintf_s (temp, 16384, format, ap);
	va_end (ap);

	return temp;
}

string mstohms(int time)
{
	int hours = 0;
	int mins = 0;
	int secs = 0;

	secs = time / 1000;
	mins = secs / 60;
	secs = secs % 60;

	hours = mins / 60;
	mins = mins % 60;

	if (hours > 0)
	{
		return sprintfa("%d:%02d:%02d",hours,mins,secs);
	} else {
		return sprintfa("%d:%02d",mins,secs);
	}
}

string stohms(int time)
{
	return mstohms(time * 1000);
}

CHAR * ShortMonth(int iMonth)
{
CHAR * Month = NULL;

	switch (iMonth)
	{
	case 1:
		Month = "Jan";
		break;
	case 2:
		Month = "Feb";
		break;
	case 3:
		Month = "Mar";
		break;
	case 4:
		Month = "Apr";
		break;
	case 5:
		Month = "May";
		break;
	case 6:
		Month = "Jun";
		break;
	case 7:
		Month = "Jul";
		break;
	case 8:
		Month = "Aug";
		break;
	case 9:
		Month = "Sep";
		break;
	case 10:
		Month = "Oct";
		break;
	case 11:
		Month = "Nov";
		break;
	case 12:
		Month = "Dec";
		break;
	}

	return Month;
}

CHAR * AMPM(SYSTEMTIME & LocalSysTime)
{
	if (LocalSysTime.wHour > 11)
		return "pm";
	return "am";
}

CHAR * ShortDay(int Day)
{
	CHAR * DOW = NULL;

	switch (Day)
	{
	case 0:
		DOW = "Sun";
		break;
	case 1:
		DOW = "Mon";
		break;
	case 2:
		DOW = "Tue";
		break;
	case 3:
		DOW = "Wed";
		break;
	case 4:
		DOW = "Thu";
		break;
	case 5:
		DOW = "Fri";
		break;
	case 6:
		DOW = "Sat";
		break;
	}

	return DOW;
}
string GetDate(int Format, SYSTEMTIME & LocalSysTime)
{
	string date;


	switch (Format)
	{
	case 0: // Mon, 15 May, 2001
		date = sprintfa("%s, %d %s, %d",ShortDay(LocalSysTime.wDayOfWeek),LocalSysTime.wDay,ShortMonth(LocalSysTime.wMonth),LocalSysTime.wYear);
		break;
	case 1: // 2001-05-15
		date = sprintfa("%d-%d-%d",LocalSysTime.wYear,LocalSysTime.wMonth,LocalSysTime.wDay);
		break;
	case 2: // 15 May 2001
		date = sprintfa("%d %s %d",LocalSysTime.wDay,ShortMonth(LocalSysTime.wMonth),LocalSysTime.wYear);
		break;
	case 3: // 05/15/2001
		date = sprintfa("%d/%d/%d",LocalSysTime.wMonth,LocalSysTime.wDay,LocalSysTime.wYear);
		break;
	case 4: // 15/05/2001
		date = sprintfa("%d/%d/%d",LocalSysTime.wDay,LocalSysTime.wMonth,LocalSysTime.wYear);
		break;
	}

	return date;
}

string GetTime(int Format, SYSTEMTIME & LocalSysTime)
{
	int Hour = LocalSysTime.wHour % 12;
	string time;
	switch (Format)
	{
	case 0: // 5:16 pm
		if(Hour == 0) Hour = 12;
		time = sprintfa("%d:%02d %s",Hour,LocalSysTime.wMinute,AMPM(LocalSysTime));
		break;
	case 1: // 5:16:05 pm
		if(Hour == 0) Hour = 12;
		time = sprintfa("%d:%02d:%02d %s",Hour,LocalSysTime.wMinute,LocalSysTime.wSecond,AMPM(LocalSysTime));
		break;
	case 2: // 17:16 pm
		time = sprintfa("%d:%02d",LocalSysTime.wHour,LocalSysTime.wMinute);
		break;
	case 3: // 17:16:05 pm
		time = sprintfa("%d:%02d:%02d",LocalSysTime.wHour,LocalSysTime.wMinute,LocalSysTime.wSecond);
		break;
	}

	return time;
}
string URLdecode(const std::string& l)
{
	std::ostringstream L;
	for(std::string::size_type x=0;x<l.size();++x)
		switch(l[x])
		{
			case('+'):
			{
				L<<' ';
				break;
			}
			case('%'): // Convert all %xy hex codes into ASCII characters.
			{
				const std::string hexstr(l.substr(x+1,2)); // xy part of %xy.
				x+=2; // Skip over hex.
				if(hexstr=="26" || hexstr=="3D")
				// Do not alter URL delimeters.
					L<<'%'<<hexstr;
				else
				{
					std::istringstream hexstream(hexstr);
					int hexint;
					hexstream>>std::hex>>hexint;
					L<<static_cast<char>(hexint);
				}
				break;
			}
			default: // Copy anything else.
			{
				L<<l[x];
				break;
			}
		}
	return L.str();
}


int FileExistsA(string filename)
{
	return GetFileAttributes(filename.c_str()) != 0xFFFFFFFF;
}

int FileExistsW(wstring filename)
{
	return FileExistsA(wstrtostr(filename));
}

wstring escape(wstring in)
{
	wstring out = L"";

	for (unsigned int i = 0 ; i < in.size() ; i++)
	{
		if (in.at(i) == L'&')
			out += L"&amp;";
		else if (in.at(i) == L'>')
			out += L"&gt;";
		else if (in.at(i) == L'<')
			out += L"&lt;";
		else if (in.at(i) == L'\'')
			out += L"&apos;";
		else if (in.at(i) == L'\"')
			out += L"&quot;";
		else 
			out += in.at(i);
	}
	return out;
}

wstring unescape(wstring in)
{
	//if (in.find("&") == in.npos) 
		return in;

	/*string out = "";

	for (unsigned int i = 0 ; i < in.size() ; i++)
	{
		if (in.at(i) == '&')
		{
			string code;
			i++;

			while (in.at(i) != ';')
			{
				code += in.at(i);
				i++;
			}

			if (code.at(0) == '#') // numerical jobby
			{
				int charcode = atoi(code.substr(1).c_str());
				char temp[2];
				temp[0] = charcode;
				temp[1] = 0;
				out.append(temp);
			} else {
				_ASSERT(FALSE);
				_ASSERT(TRUE);
			}
		}
		else 
			out += in.at(i);
	}
	return out;*/
}


const float fast_atof_table[] =	{
										0.f,
										0.1f,
										0.01f,
										0.001f,
										0.0001f,
										0.00001f,
										0.000001f,
										0.0000001f,
										0.00000001f,
										0.000000001f,
										0.0000000001f,
										0.00000000001f,
										0.000000000001f,
										0.0000000000001f,
										0.00000000000001f,
										0.000000000000001f
									};

//! Provides a fast function for converting a string into a float,
//! about 6 times faster than atof in win32.
// If you find any bugs, please send them to me, niko (at) irrlicht3d.org.
WCHAR* fast_atof_move(WCHAR* c, float& out)
{
	bool inv = false;
	WCHAR *t;
	float f;

	if (*c==L'-')
	{
		c++;
		inv = true;
	}

	f = (float)wcstol(c, &t, 10);

	c = t;

	if (*c == L'.')
	{
		c++;

		float pl = (float)wcstol(c, &t, 10);
		pl *= fast_atof_table[t-c];

		f += pl;

		c = t;

		if (*c == L'e')
		{
			++c;
			float exp = (float)wcstol(c, &t, 10);
			f *= (float)pow(10.0f, exp);
			c = t;
		}
	}

	if (inv)
		f *= -1.0f;
	
	out = f;
	return c;
}

//! Provides a fast function for converting a string into a float,
//! about 6 times faster than atof in win32.
// If you find any bugs, please send them to me, niko (at) irrlicht3d.org.
const CHAR* fast_atof_move_const(const CHAR* c, float& out)
{
	bool inv = false;
	CHAR *t;
	float f;

	if (*c=='-')
	{
		c++;
		inv = true;
	}

	f = (float)strtol(c, &t, 10);

	c = t;

	if (*c == '.')
	{
		c++;

		float pl = (float)strtol(c, &t, 10);
		pl *= fast_atof_table[t-c];

		f += pl;

		c = t;

		if (*c == 'e') 
		{ 
			++c; 
			float exp = (float)strtol(c, &t, 10); 
			f *= (float)powf(10.0f, exp); 
			c = t; 
		}
	}

	if (inv)
		f *= -1.0f;
	
	out = f;
	return c;
}


float fast_atof(const CHAR* c)
{
	float ret;
	fast_atof_move_const(c, ret);
	return ret;
}

void aGetFileSize(string filename, DWORD & modified, DWORD & high)
{
	HANDLE file = CreateFile(filename.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
	modified = GetFileSize(file,&high);
	CloseHandle(file);
}

int GetFileCreated(wstring filename)
{
	struct __stat64 buf;
	int result;
	result = _wstat64( filename.c_str(), &buf );
	result = 0;
	if (result == 0)
	{
		return (int)buf.st_ctime;
	} else {
		// TODO: file not found, but generall this is something messed up, so try to get the filesize
		return 0;
	}
}

void FindDataToStats(WIN32_FIND_DATA* findFileData, int & size, int & modified)
{
	INT64 bsize = findFileData->nFileSizeHigh;
	bsize = bsize << 32;
	bsize += findFileData->nFileSizeLow;
	bsize = bsize / 1024;

	size = (int)bsize;

	
	INT64 bmodified = findFileData->ftLastWriteTime.dwHighDateTime;
	bmodified = bmodified << 32;
	bmodified += findFileData->ftLastWriteTime.dwLowDateTime;
	bmodified = bmodified / 10000000;
	bmodified -= 11644473600;

	modified = (int)bmodified;
}

string formatsize(float val, string type)
{
	if (val > 100)
	{
		return sprintfa("%0.0f %s",val,type.c_str());
	}

	return sprintfa("%0.1f %s",val,type.c_str());
}

/*string format_size(string in)
{
	int num = atoi(in.c_str());
	return format_size(num);
}

string format_size(int in)
{
	if (in > 1073741824) // Tb
	{
		float val = (float)in / 1073741824;
		return formatsize(val,"Tb");
	}
	if (in > 1048576) // Gb
	{
		float val = (float)in / 1048576;
		return formatsize(val,"Gb");
	}
	if (in > 1024) // Mb
	{
		float val = (float)in / 1024;
		return formatsize(val,"Mb");
	}

	return sprintfa("%d Kb",in);
}*/

bool IsFolder(string filename)
{
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	HANDLE hFind = FindFirstFile(filename.c_str(), &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return false;
	}

	if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		FindClose(hFind);
		return true;
	}

	FindClose(hFind);
	return false;
}

bool SortAlphabetical(const wstring left, const wstring right) 
{ 
	int res = _wcsicmp(left.c_str(),right.c_str());
	if (res < 0) return true;
	return false;
}


void outputcmd(vector<wstring> commandparts, vector<wstring>& result)
{
	result.push_back(L"<div class='h4'>Commands</div>");
	for (unsigned int i = 0 ; i < commandparts.size() ; i++)
	{
		result.push_back(commandparts.at(i) + L"<br>");
	}
}
/*
string ImageFromFolder(string path)
{
	path = make_lowercase(path);
	string * res = (string*)g_CoverCache->GetData(path);
	if (res != NULL)
		return *res;

	string temp = ImageFromFolderQuick(path);
	if (temp.size() > 0) 
		return temp;

	if (FileExists(path + "\\cover.jpg")) return "cover.jpg";
	if (FileExists(path + "\\front.jpg")) return "front.jpg";

	if (FileExists(path + "\\cd.jpg")) return "cd.jpg";
	if (FileExists(path + "\\cover.png")) return "cover.png";
	if (FileExists(path + "\\front.png")) return "front.png";
	if (FileExists(path + "\\cd.png")) return "cd.png";

	if (FileExists(path + "\\cover.gif")) return "cover.gif";
	if (FileExists(path + "\\front.gif")) return "front.gif";
	if (FileExists(path + "\\cd.gif")) return "cd.gif";

	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd = path+"\\*";
	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);

	string result = "";
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			string s = findFileData.cFileName;
			if (s == ".") continue;
			if (s == "..") continue;
			if (FileExt(s) == "jpg")
			{
				result = s;
				break;
			}
			if (FileExt(s) == "png")
			{
				result = s;
				break;
			}
			if (FileExt(s) == "gif")
			{
				result = s;
				break;
			}
		} while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
	}

	string * str = new string(result);
	g_CoverCache->SetData(path, str);


	return result;
}

string ImageFromFolderQuick(string path)
{
	path = make_lowercase(path);
	string * res = (string*)g_CoverCache->GetData(path);
	if (res != NULL)
		return *res;

	if (FileExists(path + "\\folder.jpg")) 
	{
		string * str = new string("folder.jpg");
		g_CoverCache->SetData(path, str);
		return "folder.jpg";
	}

	if (FileExists(path + "\\folder.png"))
	{
		string * str = new string("folder.png");
		g_CoverCache->SetData(path, str);
		return "folder.jpg";
	}

	if (FileExists(path + "\\folder.gif"))
	{
		string * str = new string("folder.png");
		g_CoverCache->SetData(path, str);
		return "folder.jpg";
	}

	return "";
}*/

HWND FindClient()
{
/*#ifdef _DEBUG
	return NULL;
#endif*/
	//HWND hWnd = FindWindow(WINDOWCLASS,WINDOWCLASS);
	return 0;
}

//#define PLAYERUIOVERRIDE
/*
void MinimizeClient()
{
	HWND hWnd = FindClient();
	if (!hWnd) return;

#ifdef PLAYERUIOVERRIDE
	// override players UI
	SendMessage(hWnd, WM_USER+1,1,0);
#else
	// allow player to use its own ui
	SendMessage (hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
	//SendMessage(hWnd, WM_USER+1,1,0);
	//ShowWindow(hWnd, SW_HIDE);
#endif
}*/
/*
void RestoreClient()
{
	HWND hWnd = FindClient();
	if (!hWnd) return;

#ifdef PLAYERUIOVERRIDE
	// override players UI
	SendMessage(hWnd, WM_USER+1,2,0);
#else
	SendMessage (hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
	//SendMessage(hWnd, WM_USER+1,2,0);
	//ShowWindow(hWnd, SW_RESTORE);
	SetForegroundWindow(hWnd);
#endif
}*/

int FilesInDir(string path)
{
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd = path+"\\*";
	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	int count = 0;

	do {
		string s = findFileData.cFileName;
		if (s == ".") continue;
		if (s == "..") continue;

		count++;
		
	} while (FindNextFile(hFind, &findFileData));
	FindClose(hFind);

	return count;
}

void FileToStringA(string & result, string filename)
{
	FILE * fp;
	if (fopen_s(&fp,filename.c_str(),"r") == 0)
	{
		fseek(fp,0,SEEK_END);
		int size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		char * buffer = new char[size+1];
		memset(buffer,0,size+1);

		result = "";
		result.reserve(size+1);
		
		int read = fread(buffer,1,size,fp);
		buffer[read] = 0;
		result = buffer;
		fclose(fp);
		SAFE_DELETE_A( buffer );
	} else {
		result = "";
	}
}

void FileToStringW(wstring & result, wstring filename)
{
	FILE * fp;
	if (_wfopen_s(&fp,filename.c_str(),L"r") == 0)
	{
		fseek(fp,0,SEEK_END);
		int size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		WCHAR * buffer = new WCHAR[size+1];
		memset(buffer,0,size+1);

		result = L"";
		result.reserve(size+1);
		
		int read = fread(buffer,1,size,fp);
		buffer[read] = 0;
		result = buffer;
		fclose(fp);
		SAFE_DELETE_A( buffer );
	} else {
		result = L"";
	}
}


void StringToFileA(string &data, string filename)
{
	FILE * fp;
	if (fopen_s(&fp,filename.c_str(),"w") == 0)
	{
		fwrite(data.c_str(),1,data.size(),fp);
		fclose(fp);
	}
}

void StringToFileW(wstring &data, wstring filename)
{
	FILE * fp;
	if (_wfopen_s(&fp,filename.c_str(),L"w") == 0)
	{
		fwrite(data.c_str(),1,data.size()*2,fp);
		fclose(fp);
	}
}


bool ExtInList(wstring ext, vector<wstring>* filetypes)
{
	if (!filetypes)
		return true;

	if (filetypes->size() == 0) return true;
	for (unsigned int i = 0 ; i < filetypes->size() ; i++)
	{
		if (filetypes->at(i) == ext) return true;
	}
	return false;
}

const char NumericMap[256] = 
{
    /*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
    /* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 3 */ 0,1,2,3, 4,5,6,7, 8,9,0,0, 0,0,0,0,
    
    /* 4 */ 0,2,2,2, 3,3,3,4, 4,4,5,5, 5,6,6,6,
    /* 5 */ 7,7,7,7, 8,8,8,9, 9,9,9,0, 0,0,0,0,
    /* 4 */ 0,2,2,2, 3,3,3,4, 4,4,5,5, 5,6,6,6,
    /* 5 */ 7,7,7,7, 8,8,8,9, 9,9,9,0, 0,0,0,0,
    
    /* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    
    /* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};

const char NumericMap2[10] = {'0','1','2','3','4','5','6','7','8','9'};

string make_numeric(string in)
{
	string result;
	result.reserve(in.size());
	char res = 0;
	for (unsigned int i = 0 ; i < in.size() ; i++)
	{
		res = NumericMap[in.at(i)];
		if (res > 0)
			result.append(1,NumericMap2[res]);
	}

	return result;
}

const char noncharnummap[256] = 
{
    /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
    /* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 3 */ 1,1,2,3, 4,5,6,7, 8,9,0,0, 0,0,0,0,
    
    /* 4 */ 0,2,2,2, 3,3,3,4, 4,4,5,5, 5,6,6,6,
    /* 5 */ 7,7,7,7, 8,8,8,9, 9,9,9,0, 0,0,0,0,
    /* 4 */ 0,2,2,2, 3,3,3,4, 4,4,5,5, 5,6,6,6,
    /* 5 */ 7,7,7,7, 8,8,8,9, 9,9,9,0, 0,0,0,0,
    
    /* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    
    /* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};


wstring replace_noncharnum(wstring in)
{
	wstring result;
	result.reserve(in.size());
	char res = 0;
	for (unsigned int i = 0 ; i < in.size() ; i++)
	{
		res = noncharnummap[in.at(i)];
		if (res > 0)
			result.append(1,in.at(i));
	}

	return result;	
}

wstring replace_numtochar(wstring in)
{
	wstring result;
	result.reserve(in.size());

	for (unsigned int i = 0 ; i < in.size() ; i++)
	{
		char curchar = (char)in.at(i);
		switch (curchar)
		{
		case L'2':
			result.append(1,L'a');
			break;
		case L'3':
			result.append(1,L'd');
			break;
		case L'4':
			result.append(1,L'g');
			break;
		case L'5':
			result.append(1,L'j');
			break;
		case L'6':
			result.append(1,L'm');
			break;
		case L'7':
			result.append(1,L'p');
			break;
		case L'8':
			result.append(1,L't');
			break;
		case L'9':
			result.append(1,L'w');
			break;
		}
	}

	return result;	
}

wstring make_filter_label(wstring filter, wstring text)
{
	wstring result = L"";
	if (text.size() > 0)
	{
		text = replace_noncharnum(text);
		text = text.substr(0,filter.size());
		result = text;
	} else {
		result = replace_numtochar(filter);
	}

	result = make_uppercaseW(result);

	return result;
}

wstring make_filter_label_2(wstring filter)
{
	/*string result = "";
	if (text.size() > 0)
	{
		text = replace_noncharnum(text);
		text = text.substr(0,filter.size());
		result = text;
	} else {
		result = replace_numtochar(filter);
	}

	result = make_uppercase(result);*/

	return filter;
}

unsigned int MakeUID()
{
	unsigned int id = 0;
	//rand_s(&id);
	return id;
}

string MakeBigUID()
{
	unsigned int id1 = 0;
	//rand_s(&id1);
	unsigned int id2 = 0;
	//rand_s(&id2);
	unsigned int id3 = 0;
	//rand_s(&id3);

	return sprintfa("%u-%u-%u",id1,id2,id3);
}

string DecodeHtmlA(string in)
{
	while (in.find("&") != in.npos)
	{
		string code = in.substr(in.find("&"));
		code = code.substr(0,code.find(";")+1);
		if (!code.empty())
		{
			if (code == "&amp;")
			{
				in = str_replaceA(in,code,"&");
			} else {
				in = str_replaceA(in,code,"");
			}
		} else {
			break;
		}
	}
	return in;
}

wstring DecodeHtmlW(wstring in)
{
	while (in.find(L"&") != in.npos)
	{
		wstring code = in.substr(in.find(L"&"));
		code = code.substr(0,code.find(L";")+1);
		if (!code.empty())
		{
			if (code == L"&amp;")
			{
				in = str_replaceW(in,code,L"&");
			} else {
				in = str_replaceW(in,code,L"");
			}
		} else {
			break;
		}
	}
	return in;
}

wstring DecodeHtmlW(string in)
{
	int neededbuffer = MultiByteToWideChar(CP_UTF8,0,in.c_str(),-1,NULL,0);
	if (neededbuffer == 0)
		return NULL;
	WCHAR* buf2 = new WCHAR[neededbuffer];
	memset(buf2,0,neededbuffer);
	int res = MultiByteToWideChar(CP_UTF8,0,in.c_str(),-1,buf2,neededbuffer);
	if (res == 0)
		return NULL;

	wstring result = DecodeHtmlW(buf2);
	delete buf2;
	return result;
}

string bytesToHex(char* data, const int datasize)
{
	ostringstream result;
	for(int i = 0; i < datasize; ++i)
	{
		result << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << (int)(unsigned char)data[i];
	}
	return result.str();
} 

//Function to convert unsigned char to string of length 2
void Char2Hex(unsigned char ch, char* szHex)
{
	unsigned char byte[2];
	byte[0] = ch/16;
	byte[1] = ch%16;
	for(int i=0; i<2; i++)
	{
		if(byte[i] >= 0 && byte[i] <= 9)
			szHex[i] = '0' + byte[i];
		else
			szHex[i] = 'A' + byte[i] - 10;
	}
	szHex[2] = 0;
}

//Function to convert string of length 2 to unsigned char
void Hex2Char(char const* szHex, unsigned char& rch)
{
	rch = 0;
	for(int i=0; i<2; i++)
	{
		if(*(szHex + i) >='0' && *(szHex + i) <= '9')
			rch = (rch << 4) + (*(szHex + i) - '0');
		else if(*(szHex + i) >='A' && *(szHex + i) <= 'F')
			rch = (rch << 4) + (*(szHex + i) - 'A' + 10);
		else
			break;
	}
}    

//Function to convert string of unsigned chars to string of chars
void CharStr2HexStr(unsigned char const* pucCharStr, char* pszHexStr, int iSize)
{
	int i;
	char szHex[3];
	pszHexStr[0] = 0;
	for(i=0; i<iSize; i++)
	{
		Char2Hex(pucCharStr[i], szHex);
		strcat(pszHexStr, szHex);
	}
}

//Function to convert string of chars to string of unsigned chars
void HexStr2CharStr(char const* pszHexStr, unsigned char* pucCharStr, int iSize)
{
	int i;
	unsigned char ch;
	for(i=0; i<iSize; i++)
	{
		Hex2Char(pszHexStr+2*i, ch);
		pucCharStr[i] = ch;
	}
}

HRESULT GetBytesString(BYTE* Data, UINT DataLen, CHAR* OutBuffer, UINT* OutLen) {

	// Check our lenghts
	if(*OutLen < (DataLen * 2))
		return S_FALSE;

	*OutLen = DataLen * 2;

	// Output into our buffer as hex
	CHAR hexChars[] = "0123456789ABCDEF";
	for(UINT x = 0, y = 0; x < DataLen; x++, y+=2) {
		OutBuffer[y] = hexChars[(Data[x] >> 4)];
		OutBuffer[y + 1] = hexChars[(Data[x] & 0x0F)];
	}

	// All done =)
	return S_OK;
}