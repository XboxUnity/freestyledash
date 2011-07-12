#pragma once
#include "../Generic/xboxtools.h"
#include "../Generic/tools.h"

#define XFSD_MAGIC			0x58465344
#define XFSD_MEMBERID_MASK	0xFFFF
#define XFSD_MEMBERID_IDENT 0x4944
#define XFSD_METADATA_MAX	0x3E8

//defines for team members (easy access)
#define XFSD_MEMBER_ANTHONY			0x00
#define XFSD_MEMBER_EQUINOX			0x01
#define XFSD_MEMBER_HELLDOC			0x02
#define XFSD_MEMBER_KHUONG			0x03
#define XFSD_MEMBER_JQE				0x04
#define XFSD_MEMBER_MAESTERROWEN	0x05
#define XFSD_MEMBER_MATTIE			0x06
#define XFSD_MEMBER_NODE			0x07
#define XFSD_MEMBER_WONDRO			0x08

class XFSD
{
private:
	struct XFSDHeader
	{
		int Magic;				// XFSD  - 0x58465344
		int Version;			// Ver1  - 0x00000001
		int EntryCount;			// Count - 0x00000009
	};

	struct XFSDEntry
	{
		int MemberID;			// ID - 0x4944XXXX (XXXX = id number)
		char MemberName[0x10];  // Name- 16bytes
		int Offset;				// Offset to Metadata
		int Size;				// Size of Metadata
	};

	FILE * fHandle;
	VOID * xfsdData;
	DWORD xfsdSize;
	XFSDHeader Header;
	XFSDEntry * EntryTable;
	bool m_bFromMemory;

public:
	~XFSD();
	XFSD();

	HRESULT OpenXFSD(string szFileLocation, bool fromMemory = false);
	string GetMemberName(int MemberID);
	HRESULT GetMemberMetadata(int MemberID, PXAVATAR_METADATA pMetadata);
	HRESULT GetMemberCount(int * MemberCount);
	HRESULT CloseXFSD( void );
};