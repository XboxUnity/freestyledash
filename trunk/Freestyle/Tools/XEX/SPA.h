#pragma once

typedef __int64 INT64;

class SPA
{
	#define SPA_SECTION_METADATA     0x0001
	#define SPA_SECTION_IMAGES       0x0002
	#define SPA_SECTION_STRINGTABLES 0x0003

	#define LOCALE_DEFAULT		0
	#define LOCALE_ENGLISH		1
	#define LOCALE_JAPANESE		2

	#define SPA_MAGIC_XSTC           0x0000000058535443

	#pragma pack(1)	

	typedef struct _XBDF_HEADER {
		DWORD Magic;
		DWORD Version;
		DWORD EntryMax;
		DWORD EntryCurrent;
		DWORD FreeMax;
		DWORD FreeCurrent;
	} XBDF_HEADER, *PXBDF_HEADER;

	typedef struct _XBDF_ENTRY {
        WORD	NameSpace;
		INT64	Id;
        DWORD	Offset;
        DWORD	Size;
	} XBDF_ENTRY, *PXBDF_ENTRY;

	typedef struct XBDF_FILE_LOC {
		DWORD Offset;
        DWORD Size;
	} XBDF_FILE_LOC, *PXBDF_FILE_LOC;	  

	typedef struct _SPA_XSTC {
		DWORD Magic;
		DWORD Version;
		DWORD Size;
		DWORD DefaultLanguage;
	} SPA_XSTC, *PSPA_XSTC;

	typedef struct _SPA_XSTR_HEADER {
		DWORD   Magic;
		DWORD   Version;
		DWORD	Size;
		WORD	StringCount;
	} SPA_XSTR_HEADER, *PSPA_XSTR_HEADER;

	typedef struct _SPA_STRING_ENTRY {
		WORD Id;
		WORD StringLength;
		CHAR String[1];
	} SPA_STRING_ENTRY, *PSPA_STRING_ENTRY;

	#pragma pack() 

	short currentTable;
	map<WORD, string> m_Entries;

	BOOL  isFile;
	DWORD SPADataSize;
	BYTE* SPAData;
	BYTE* CurrentPosition;

	void fRead(void* DstBuf, size_t ElementSize, size_t Count);
	void fSeek(long Offset, int Origin);

public:

	SPA() {
		EntryTable = NULL;
		FreeTable = NULL;
		isFile = FALSE;
		SPADataSize = 0;
		SPAData = NULL;
		CurrentPosition = NULL;
	}
	~SPA(){ CloseSPA(); }

	FILE*		   fHandle;
	XBDF_HEADER	   Header;
	XBDF_ENTRY*    EntryTable;
	XBDF_FILE_LOC* FreeTable;

	HRESULT OpenSPA(string FilePath);
	HRESULT OpenSPA(BYTE* SPABuffer, DWORD SPASize);
	HRESULT CloseSPA();
	HRESULT GetSection(WORD NameSpace, INT64 Id, BYTE** Data, DWORD* Size);
	HRESULT GetStringTable(INT64 Id);
	HRESULT GetTitleName(string * Title, DWORD locale);
	HRESULT GetTableValue(INT64 Id, string* value, DWORD locale);
	HRESULT GetImage(INT64 Id, BYTE** ImageData, DWORD* Size);
	HRESULT GetTitleImage(BYTE** ImageData, DWORD* Size);
};