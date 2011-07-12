#pragma once

using namespace std;
using namespace stdext;

#define TOOLS_API

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { if (x) { delete x; x = NULL; } }
#endif

#ifndef SAFE_DELETE_A
#define SAFE_DELETE_A(x) { if (x) { delete[] x; x = NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) { if (x) { x->Release(); x = NULL; } }
#endif

struct TOOLS_API DI_Item {
	wstring FileName;
	long FileSize;
	wstring Path;
	int attribs;
};
DWORD FSDGetFileAttribute(const char* file);
int strpos(const char *haystack,const char *needle);
// get time in ms since app started
TOOLS_API int GetTime();
string GetDate(int Format, SYSTEMTIME & LocalSysTime);
string GetTime(int Format, SYSTEMTIME & LocalSysTime);
string ParseExecutablePath( string szFilePath );
CHAR * ShortDay(int Day);
CHAR * AMPM(SYSTEMTIME & LocalSysTime);
void wStringSplit(wstring str, wstring delim, vector<wstring>* results);
CHAR * ShortMonth(int iMonth);
WCHAR* strtowchar(string str);
//wstring strtowstr(string str);
char* wchartochar(const WCHAR* str,int mLength);
string bytesToHex(char* data, const int datasize);

void CharStr2HexStr(unsigned char const* pucCharStr, char* pszHexStr, int iSize);
void HexStr2CharStr(char const* pszHexStr, unsigned char* pucCharStr, int iSize);

// scale an interger by sf. rounds to nearest int
inline int Scale(int num, float sf)
{
	return (int)((float)num * sf);
}
inline int ScaleFloor(int num, float sf)
{
	return (int)(floor((float)num * sf));
}
inline int ScaleCeil(int num, float sf)
{
	return (int)(ceil((float)num * sf));
}


////////////////////
// string tools
////////////////////

#ifdef _UNICODE
#define Tokenize TokenizeW
#define make_lowercase make_lowercaseW
#define make_uppercase make_uppercaseW
#else
#define Tokenize TokenizeA
#define make_lowercase make_lowercaseA
#define make_uppercase make_uppercaseA
#endif

// break a string down by a delimeter
TOOLS_API void TokenizeA(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " ");

TOOLS_API void TokenizeW(const wstring& str,
                      vector<wstring>& tokens,
                      const wstring& delimiters = L" ");

// lowercase string
TOOLS_API string make_lowercaseA(string s);
TOOLS_API wstring make_lowercaseW(wstring s);

// uppercase string
TOOLS_API string make_uppercaseA(string s);
TOOLS_API wstring make_uppercaseW(wstring s);

// Title Case String
TOOLS_API wstring make_titlecase(wstring s);

// Turn string into text numebr input
TOOLS_API wstring make_numeric(wstring s);

#ifdef _UNICODE
#define TrimLeft TrimLeftW
#define TrimRight TrimRightW
#define Trim TrimW
#else
#define TrimLeft TrimLeftA
#define TrimRight TrimRightA
#define Trim TrimA
#endif

// trim various chars from a string
TOOLS_API string TrimLeftA(string str);
TOOLS_API string TrimRightA(string str);
TOOLS_API string TrimA(string s);

// trim various chars from a string
TOOLS_API wstring TrimLeftW(wstring str);
TOOLS_API wstring TrimRightW(wstring str);
TOOLS_API wstring TrimW(wstring s);

// routines for making keypad filter display
TOOLS_API wstring make_filter_label(wstring filter, wstring text);
TOOLS_API wstring make_filter_label_2(wstring filter);
TOOLS_API wstring replace_numtochar(wstring in);
TOOLS_API wstring replace_noncharnum(wstring in);

// NO IDEA
TOOLS_API string TrimRightStr(string str,string crop);

// format time, from either ms or s (milliseconds or seconds)
TOOLS_API string mstohms(int time);
TOOLS_API string stohms(int time);

// format a filesize from size in Kb
TOOLS_API wstring format_size(wstring in);
TOOLS_API wstring format_size(int in);

// URL Encoding and Decoding
//TOOLS_API wstring UriDecode(const wstring & sSrc);
//TOOLS_API wstring UriEncode(const wstring & sSrc);
TOOLS_API wstring escape(wstring in); // break out html chars
TOOLS_API wstring unescape(wstring in);

// generic printf to make a string
#ifdef _UNICODE
#define sprintfa sprintfaW
#else
#define sprintfa sprintfaA
#endif
TOOLS_API string sprintfaA(const char *format, ...);
TOOLS_API wstring sprintfaW(const WCHAR *format, ...);

// Sort functions
TOOLS_API bool SortAlphabetical(const wstring left, const wstring right) ;

// convert to and from wide strings
TOOLS_API string wstrtostr(wstring wstr);
TOOLS_API wstring strtowstr(string str);

TOOLS_API wstring FromUtf8(const std::string& utf8string); 
TOOLS_API string ToUtf8(const std::wstring& widestring);


TOOLS_API void RecursiveMkdir(string path);
// Quick text to float routines
TOOLS_API CHAR* fast_atof_move(CHAR* c, float& out);
TOOLS_API const CHAR* fast_atof_move_const(const CHAR* c, float& out);
TOOLS_API float fast_atof(const CHAR* c);

/////////////////////////
// Filesystem stuff
//////////////////////////

TOOLS_API void FindDataToStats(WIN32_FIND_DATA* findFileData, int & size, int & modified);
TOOLS_API int GetFileCreated(wstring filename);
TOOLS_API bool IsFolder(string filename);
TOOLS_API wstring ImageFromFolder(wstring path);
TOOLS_API wstring ImageFromFolderQuick(wstring path);
TOOLS_API int FilesInDir(wstring path);

TOOLS_API bool DirectoryInfo(wstring directory, vector<DI_Item>& files, vector<DI_Item>& folders, bool clear = true);

#ifdef UNICODE
#define LastFolder LastFolderW
#define FileExt FileExtW
#define FileNoExt FileNoExtW
#define FileExists FileExistsW
#define str_replace str_replaceW
#define str_replaceall str_replaceallW
#else
#define LastFolder LastFolderA
#define FileExt FileExtA
#define FileNoExt FileNoExtA
#define FileExists FileExistsA
#define str_replace str_replaceA
#define str_replaceall str_replaceallA
#endif
string URLdecode(const std::string& l);
TOOLS_API string LastFolderA(string folder);
TOOLS_API string FileExtA(string filename);
TOOLS_API string FileNoExtA(string filename);
TOOLS_API int FileExistsA(string filename);
TOOLS_API string str_replaceA(string source, string find, string replace);
//TOOLS_API string str_replaceallA(string source, string find, string replace);
TOOLS_API string str_replaceallA(string s, string sub, string other);
TOOLS_API string str_removeInvalidChar(string s);
TOOLS_API wstring str_removeInvalidCharW(wstring s);
TOOLS_API wstring str_removeSpecialCharW(wstring s);
TOOLS_API string str_removeSpecialChar(string s);


string ReadFileToString(string filePath);
TOOLS_API wstring LastFolderW(wstring folder);
TOOLS_API wstring FileExtW(wstring filename);
TOOLS_API wstring FileNoExtW(wstring filename);
TOOLS_API int FileExistsW(wstring filename);
TOOLS_API wstring str_replaceW(wstring source, wstring find, wstring replace);
TOOLS_API wstring str_replaceallW(wstring s, wstring sub, wstring other);

//////////////////////////////////////////////


// output html list of commandparts into result
TOOLS_API void outputcmd(vector<wstring> commandparts, vector<wstring>& result);


// client window control stuff
TOOLS_API void RestoreClient();
TOOLS_API void MinimizeClient();
TOOLS_API HWND FindClient();

#ifdef _UNICODE
#define StringToFile StringToFileW
#define FileToString FileToStringW
#else
#define StringToFile StringToFileA
#define FileToString FileToStringA
#endif

// save and load a string to/from a file
TOOLS_API void StringToFileA(string &data, string filename);
TOOLS_API void FileToStringA(string & result, string filename);

TOOLS_API inline void StringToFileA(string &data, wstring filename)
{
	return StringToFileA(data,wstrtostr(filename));
}

TOOLS_API inline void FileToStringA(string & result, wstring filename)
{
	return FileToStringA(result,wstrtostr(filename));
}

TOOLS_API void StringToFileW(wstring &data, wstring filename);
TOOLS_API void FileToStringW(wstring & result, wstring filename);

// check if a string in is a list of extensions
TOOLS_API bool ExtInList(wstring ext, vector<string>* filetypes);

struct DriveInfo
{
	wstring letter;
	wstring desc;
	int type;
	int ready;

	int SizeMb;
	int FreeMb;
};

TOOLS_API void GetDriveList(vector <DriveInfo> & results);
TOOLS_API void GetDriveInfo(const WCHAR * unit, DriveInfo & di);

// uid stuff
TOOLS_API unsigned int MakeUID();
TOOLS_API string MakeBigUID();

// The following class defines a hash function for strings 
class stringhasher : public stdext::hash_compare <std::wstring>
{
public:
	size_t operator() (const wstring& s) const
	{
		size_t hash = 5381;

		for(size_t i = 0; i < s.length(); i++)
		{
			hash = ((hash << 5) + hash) + (s[i] | 0x20);
		}

		return hash;
	}

	bool operator() (const std::wstring& s1, const std::wstring& s2) const
	{
		return s1 < s2;
	}
};
void StringSplit(string str, string delim, vector<string>* results, bool ShowEmptyEntries = false);
//string MyAppPath;

TOOLS_API void CleanDir(wstring source);



enum CacheStyle {
	CS_IMAGE,
	CS_IMDBCOVER,
	CS_SMALL,
};

TOOLS_API DWORD ParseHTMLColor(wstring color);
TOOLS_API int PickValue(map<wstring, int>& values, wstring value, int defaultval);
TOOLS_API bool HTMLYesNo(wstring text);

TOOLS_API bool FirstInFolder(wstring path, wstring filename, vector <wstring> * filetypes = NULL);

TOOLS_API wstring GetSpecialFolderPath(int PathID);

TOOLS_API inline wstring DOWToDay(int DOW)
{
	switch (DOW)
	{
	case 0:
		return L"Sunday";
	case 1:
		return L"Monday";
	case 2:
		return L"Tuesday";
	case 3:
		return L"Wednesday";
	case 4:
		return L"Thursday";
	case 5:
		return L"Friday";
	case 6:
		return L"Saturday";
	}
	return L"";
}

TOOLS_API inline wstring DOWToDayS(int DOW)
{
	switch (DOW)
	{
	case 0:
		return L"Sun";
	case 1:
		return L"Mon";
	case 2:
		return L"Tue";
	case 3:
		return L"Wed";
	case 4:
		return L"Thu";
	case 5:
		return L"Fri";
	case 6:
		return L"Sat";
	}
	return L"";
}

TOOLS_API inline wstring MonthToStr(int Month)
{
	switch (Month)
	{
	case 1:
		return L"Janurary";
	case 2:
		return L"Feburary";
	case 3:
		return L"March";
	case 4:
		return L"April";
	case 5:
		return L"May";
	case 6:
		return L"June";
	case 7:
		return L"July";
	case 8:
		return L"August";
	case 9:
		return L"September";
	case 10:
		return L"October";
	case 11:
		return L"November";
	case 12:
		return L"December";
	}

	return L"";
}

TOOLS_API inline string DayToDayth(int Dayno)
{
	int last = Dayno % 10;
	string day = sprintfa("%d",Dayno);
	switch (last)
	{
	case 1:
		return day + "st";
	case 2:
		return day + "nd";
	case 3:
		return day + "rd";
	case 0:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		return day + "th";
	}

	return day;
}

TOOLS_API wstring SortVideoTSName(wstring path, wstring filename);

TOOLS_API inline void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
 // Note that LONGLONG is a 64-bit value
 LONGLONG ll;

 ll = Int32x32To64(t, 10000000) + 116444736000000000;
 pft->dwLowDateTime = (DWORD)ll;
 pft->dwHighDateTime = (DWORD)(ll >> 32);
}

TOOLS_API inline void UnixTimeToSystemTime(time_t t, LPSYSTEMTIME pst)
{
 FILETIME ft;

 UnixTimeToFileTime(t, &ft);
 FileTimeToSystemTime(&ft, pst);
}

TOOLS_API inline time_t FILETIMEtoUnix(FILETIME * ft)
{
	LONGLONG ll;
	ll = (LONGLONG)ft->dwLowDateTime + ((LONGLONG)ft->dwHighDateTime << 32);
	ll -= 116444736000000000;
	ll /= 10000000;
	return (time_t)ll;
}

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

TOOLS_API int gettimeofday(struct timeval *tv, struct timezone *tz);
TOOLS_API BOOL DeleteDirectory(const WCHAR* sPath);

TOOLS_API inline void GenSRand()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand((unsigned int)(((tv.tv_usec << 12) | (tv.tv_sec & 0xFFF)) ^ 5467203459UL));
}

TOOLS_API string UTF16toUTF8(const wstring & in);

#ifdef _UNICODE
#define DecodeHtml DecodeHtmlW
#else
#define DecodeHtml DecodeHtmlA
#endif

TOOLS_API string DecodeHtmlA(string in);
TOOLS_API wstring DecodeHtmlW(wstring in);
TOOLS_API wstring DecodeHtmlW(string in);

void aGetFileSize(string filename, DWORD & modified, DWORD & high);
HRESULT GetBytesString(BYTE* Data, UINT DataLen, CHAR* OutBuffer, UINT* OutLen);