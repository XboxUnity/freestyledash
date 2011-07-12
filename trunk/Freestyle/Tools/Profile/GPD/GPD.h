#pragma once
#include "../../Generic/xboxtools.h"

class GPD
{

protected:

	#define GPD_SECTION_ACHIEVEMENT  0x0001
	#define GPD_SECTION_IMAGES       0x0002
	#define GPD_SECTION_SETTINGS	 0x0003
	#define GPD_SECTION_TITLE		 0x0004
	#define GPD_SECTION_STRING		 0x0005
	#define GPD_SECTION_SECURITY	 0x0006

	#define GPD_STRING_LABEL		 0x0000
	#define GPD_STRING_DESC			 0x0001
	#define GPD_STRING_UNACH		 0x0002

	#define GPD_MAGIC_XSTC           0x0000000058535443

	#pragma warning(push)
	#pragma warning(disable:4200)
	#pragma pack(1)
	
	struct XBDFHeader {
		int Magic;
		int Version;
		int EntryMax;
		int EntryCurrent;
		int FreeMax;
		int FreeCurrent;
	};

	struct XBDFEntry {
        short	NameSpace;
		__int64 Id;
        int		Offset;
        int		Size;
	};

	struct XBDFFileLoc {
		int Offset;
        int Size;
	};	  

	struct GPDXSTC {
		int Magic;
		int Version;
		int Size;
		int DefaultLanguage;
	};

	struct GPDXSTRHeader {
		int   Magic;
		int   Version;
		int	  Size;
		short StringCount;
	};
	
	struct GPDACHVHeader {
		DWORD dwStructSize;		// 0x00 - 0x04
		DWORD dwAchievementID;  // 0x04 - 0x08
		DWORD dwImageID;		// 0x08 - 0x0C
		DWORD dwGamerCred;		// 0x0C - 0x10
		DWORD dwFlags;			// 0x10 - 0x14
		FILETIME ftUnlockTime;  // 0x18 - 0x1C
	};

	struct GPDStringEntry {
		short Id;
		short StringLength;
		char String[];
	};
	#pragma pack() 
	#pragma warning(pop)

	short currentTable;
	map<short, string> m_Entries;

public:

	GPD(){
		EntryTable = NULL;
		FreeTable = NULL;
	}
	~GPD(){
			if(EntryTable!= NULL)
				delete [] EntryTable;
			if(FreeTable != NULL)
				delete [] FreeTable;
	}

	FILE*		 fHandle;
	XBDFHeader	 Header;
	XBDFEntry*   EntryTable;
	XBDFFileLoc* FreeTable;

	HRESULT OpenGPD(string filePath);
	HRESULT CloseGPD();
	HRESULT GetSection(short NameSpace, __int64 Id, byte** Data, int* Size);
};