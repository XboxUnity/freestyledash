#pragma once

#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"

// 128x128 DXT1 Header
const BYTE Dxt1Header[128] = { 
	0x44, 0x44, 0x53, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x07, 0x10, 0x08, 
	0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x20, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 
	0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x44, 0x58, 0x54, 0x31,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

class XboxExecutable
{
private :

	struct XbeHeader {
		DWORD Magic;
		BYTE  HeaderSignature[256];
		DWORD BaseAddress;
		DWORD HeaderSize;
		DWORD ImageSize;
		DWORD XbeHeaderSize;
		DWORD Timestamp;
		DWORD CertPtr;
		DWORD NumSections;
		DWORD SectionsPtr;
		DWORD InitFlags;
		DWORD EntryPoint;
		DWORD TlsDirPtr;
		DWORD StackCommit;
		DWORD HeapReserve;
		DWORD HeapCommit;
		DWORD PeBaseAddress;
		DWORD PeImageSize;
		DWORD PeChecksum;
		DWORD PeTimestamp;
		DWORD PcExePathPtr;
		DWORD PcExeFilenamePtr;
		DWORD PcExeFilenameUnicodePtr;
		DWORD KernelThunkTable;
		DWORD DebugImportTable;
		DWORD NumLibraries;
		DWORD LibrariesPtr;
		DWORD KernelLibPtr;
		DWORD XapiLibPtr;
		DWORD LogoBitmap;
		DWORD LogoBitmapSize;
	};

	struct XbeCertificate {
		DWORD Size;
		DWORD Timestamp;
		DWORD TitleId;
		WCHAR TitleName[40];
		DWORD AlternateTitleIds[16];
		DWORD MediaTypes;
		DWORD GameRegion;
		DWORD GameRating;
		DWORD DiskNumber;
		DWORD Version;
		BYTE  LanKey[16];
		BYTE  SignatureKey[16];
		BYTE  AlternateSignatureKeys[16][16];
	};

	struct XbeSection {
		DWORD Flags;
		DWORD VirtualAddress;
		DWORD VirtualSize;
		DWORD FileAddress;
		DWORD FileSize;
		DWORD SectionNamePtr;
		DWORD SectionReferenceCount;
		DWORD HeadReferenceCount;
		DWORD TailReferenceCount;
		BYTE  ShaHash[20];
	};

	// Xbox Packed Resource header
	struct XproHeader {
		DWORD Magic;
		DWORD FileSize;
		DWORD DataStart;
	};

public:

	FILE*			fHandle;

	XbeHeader		Header;
	XbeCertificate  Certificate;
	XbeSection*		Sections;

	XboxExecutable(void) {}
	~XboxExecutable(void) {}
	HRESULT OpenExecutable(string ExecutablePath);
	HRESULT GetGameThumbnail(void** Data, int* Size);
	wstring GetTitleName();
	HRESULT GetTitleName(string * titleName, int * Length);
	DWORD GetTitleId();
	HRESULT GetTitleId(DWORD * titleId);
	DWORD GetMediaId();
	HRESULT GetMediaId(DWORD * mediaTypes);
	DWORD GetCurrentDisc();
	HRESULT GetCurrentDisc(DWORD * currentDisc);
	HRESULT CloseExecutable();
};
